// Copyright (C) 2018, Jaguar Land Rover
// This program is licensed under the terms and conditions of the
// Mozilla Public License, version 2.0.  The full text of the
// Mozilla Public License is at https://www.mozilla.org/MPL/2.0/
//
// Author: Magnus Feuer (mfeuer1@jaguarlandrover.com)

#ifdef USE_EPOLL
#include "dstc.h"
#include "dstc_internal.h"
#include <sys/epoll.h>
#include <stdlib.h>
#include <rmc_log.h>  // From reliable multicast packet
#include <errno.h>

static void poll_add(user_data_t user_data,
                     int descriptor,
                     uint32_t event_user_data,
                     rmc_poll_action_t action)
{
    dstc_context_t* ctx = (dstc_context_t*) user_data.ptr;
    struct epoll_event ev = {
        .data.u32 = event_user_data,
        .events = 0 // EPOLLONESHOT
    };

    if (action & RMC_POLLREAD)
        ev.events |= EPOLLIN;

    if (action & RMC_POLLWRITE)
        ev.events |= EPOLLOUT;

    _dstc_lock_context(ctx);
    if (epoll_ctl(ctx->epoll_fd, EPOLL_CTL_ADD, descriptor, &ev) == -1) {
        RMC_LOG_INDEX_FATAL(FROM_POLL_EVENT_USER_DATA(event_user_data), "epoll_ctl(add) event_udata[%lX]",
                            event_user_data);
        exit(255);
    }
    RMC_LOG_COMMENT("poll_add() read[%c] write[%c]\n",
                    ((action & RMC_POLLREAD)?'y':'n'),
                    ((action & RMC_POLLWRITE)?'y':'n'));
    _dstc_unlock_context(ctx);
}


void poll_add_sub(user_data_t user_data,
                         int descriptor,
                         rmc_index_t index,
                         rmc_poll_action_t action)
{
    poll_add(user_data, descriptor, TO_POLL_EVENT_USER_DATA(index, 0), action);
}

void poll_add_pub(user_data_t user_data,
                  int descriptor,
                  rmc_index_t index,
                  rmc_poll_action_t action)
{
    poll_add(user_data, descriptor, TO_POLL_EVENT_USER_DATA(index, 1), action);
}

static void poll_modify(user_data_t user_data,
                        int descriptor,
                        uint32_t event_user_data,
                        rmc_poll_action_t old_action,
                        rmc_poll_action_t new_action)
{
    dstc_context_t* ctx = (dstc_context_t*) user_data.ptr;

    struct epoll_event ev = {
        .data.u32 = event_user_data,
        .events = 0 // EPOLLONESHOT
    };

    if (old_action == new_action)
        return ;

    if (new_action & RMC_POLLREAD)
        ev.events |= EPOLLIN;

    if (new_action & RMC_POLLWRITE)
        ev.events |= EPOLLOUT;

    _dstc_lock_context(ctx);
    if (epoll_ctl(ctx->epoll_fd, EPOLL_CTL_MOD, descriptor, &ev) == -1) {
        RMC_LOG_INDEX_FATAL(FROM_POLL_EVENT_USER_DATA(event_user_data), "epoll_ctl(modify): %s", strerror(errno));
        exit(255);
    }
    _dstc_unlock_context(ctx);
}


void poll_modify_pub(user_data_t user_data,
                     int descriptor,
                     rmc_index_t index,
                     rmc_poll_action_t old_action,
                     rmc_poll_action_t new_action)
{
    poll_modify(user_data,
                descriptor,
                TO_POLL_EVENT_USER_DATA(index, 1),
                old_action,
                new_action);
}

void poll_modify_sub(user_data_t user_data,
                     int descriptor,
                     rmc_index_t index,
                     rmc_poll_action_t old_action,
                     rmc_poll_action_t new_action)
{
    poll_modify(user_data,
                descriptor,
                TO_POLL_EVENT_USER_DATA(index, 0),
                old_action,
                new_action);
}

void poll_remove(user_data_t user_data,
                 int descriptor,
                 rmc_index_t index)
{
    dstc_context_t* ctx = (dstc_context_t*) user_data.ptr;

    _dstc_lock_context(ctx);
    if (epoll_ctl(ctx->epoll_fd, EPOLL_CTL_DEL, descriptor, 0) == -1) {
        RMC_LOG_INDEX_WARNING(index, "epoll_ctl(delete): %s", strerror(errno));
        _dstc_unlock_context(ctx);
        return;
    }
    RMC_LOG_INDEX_COMMENT(index, "poll_remove() desc[%d] index[%d]", descriptor, index);
    _dstc_unlock_context(ctx);
}



static void _dstc_process_epoll_result(dstc_context_t* ctx,
                                       struct epoll_event* event)

{

    uint8_t op_res = 0;
    rmc_index_t c_ind = (rmc_index_t) FROM_POLL_EVENT_USER_DATA(event->data.u32);
    int is_pub = IS_PUB(event->data.u32);

    RMC_LOG_INDEX_DEBUG(c_ind, "%s: %s%s%s",
                        (is_pub?"pub":"sub"),
                        ((event->events & EPOLLIN)?" read":""),
                        ((event->events & EPOLLOUT)?" write":""),
                        ((event->events & EPOLLHUP)?" disconnect":""));


    if (event->events & EPOLLIN) {
        if (is_pub)
            rmc_pub_read(ctx->pub_ctx, c_ind, &op_res);
        else
            rmc_sub_read(ctx->sub_ctx, c_ind, &op_res);
    }

    if (event->events & EPOLLOUT) {
        if (is_pub) {
            op_res = rmc_pub_write(ctx->pub_ctx, c_ind, &op_res);
            if (op_res != 0 && op_res != ENODATA)
                rmc_pub_close_connection(ctx->pub_ctx, c_ind);
        } else {
            op_res = rmc_sub_write(ctx->sub_ctx, c_ind, &op_res);
            if (op_res != 0 && op_res != ENODATA)
                rmc_sub_close_connection(ctx->sub_ctx, c_ind);
        }
    }
}

int _dstc_process_single_event(dstc_context_t* ctx, int timeout_msec)
{
    struct epoll_event events[DSTC_MAX_CONNECTIONS];
    int nfds = 0;
    int fd = ctx->epoll_fd;

    _dstc_unlock_context(ctx);
    do {
        errno = 0;
        nfds = epoll_wait(fd,
                          events,
                          sizeof(events) / sizeof(events[0]),
                          timeout_msec);
    } while(nfds == -1 && errno == EINTR);
    _dstc_lock_context(ctx);

    if (nfds == -1) {
        RMC_LOG_FATAL("epoll_wait(%d): %s",  fd, strerror(errno));
        exit(255);
    }

    // Timeout
    if (nfds == 0)
        return ETIME;


    // Process all pending event.s
    while(nfds--)
        _dstc_process_epoll_result(ctx, &events[nfds]);

    return 0;
}

void dstc_process_epoll_result(struct epoll_event* event)

{
    extern dstc_context_t _dstc_default_context;

    // Prep for future, caller-provided contexct.
    dstc_context_t* ctx = &_dstc_default_context;

    _dstc_lock_and_init_context(ctx);
    _dstc_process_epoll_result(ctx, event);
    _dstc_unlock_context(ctx);
}
#endif // POLL=epoll
