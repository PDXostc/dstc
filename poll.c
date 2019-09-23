// Copyright (C) 2018, Jaguar Land Rover
// This program is licensed under the terms and conditions of the
// Mozilla Public License, version 2.0.  The full text of the
// Mozilla Public License is at https://www.mozilla.org/MPL/2.0/
//
// Author: Magnus Feuer (mfeuer1@jaguarlandrover.com)


#include "dstc.h"
#include "dstc_internal.h"

#include <stdlib.h>
#include <rmc_log.h>  // From reliable multicast packet
#include <errno.h>
#include "uthash.h"



static poll_elem_t* find_free_poll_elem_index(dstc_context_t* ctx)
{
    int ind = 0;
    while(ind < sizeof(ctx->poll_elem_array) / sizeof(ctx->poll_elem_array[0])) {
        // Check if file descriptor is unused
        if (ctx->poll_elem_array[ind].pfd.fd == -1)
            return &ctx->poll_elem_array[ind];
        RMC_LOG_DEBUG("Poll element %d = %d", ind, ctx->poll_elem_array[ind].pfd.fd);
        ind++;
    }

    // Out of mem
    return 0;
}

static void poll_add(user_data_t user_data,
                     int descriptor,
                     uint32_t event_user_data,
                     rmc_poll_action_t action)
{
    dstc_context_t* ctx = (dstc_context_t*) user_data.ptr;

    poll_elem_t* pelem = 0;

    _dstc_lock_context(ctx);

    // Do we already have it in our poll set?
    HASH_FIND_INT(ctx->poll_hash, &descriptor, pelem);
    if (pelem) {
        RMC_LOG_INDEX_FATAL(FROM_POLL_EVENT_USER_DATA(event_user_data), "File descriptor %d already in poll set\n", descriptor);
        exit(255);
    }

    pelem = find_free_poll_elem_index(ctx);

    if (!pelem) {
        RMC_LOG_INDEX_FATAL(FROM_POLL_EVENT_USER_DATA(event_user_data), "Out of poll_elem_t elements. Rebuild with larger DSTC_MAX_CONNECTIONS");
        exit(255);
    }

    pelem->user_data = event_user_data;
    pelem->pfd.events = 0;
    pelem->pfd.revents = 0;
    pelem->pfd.fd = descriptor;

    if (action & RMC_POLLREAD)
        pelem->pfd.events |= POLLIN;

    if (action & RMC_POLLWRITE)
        pelem->pfd.events |= POLLOUT;

    HASH_ADD_INT(ctx->poll_hash, pfd.fd, pelem);
    RMC_LOG_COMMENT("poll_add(%d) %s read[%c] write[%c] user_data[%X]\n",
                    descriptor,
                    IS_PUB(event_user_data)?"pub":"sub",
                    ((action & RMC_POLLREAD)?'y':'n'),
                    ((action & RMC_POLLWRITE)?'y':'n'),
                    FROM_POLL_EVENT_USER_DATA(event_user_data));
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
    poll_elem_t* pelem = 0;

    if (old_action == new_action)
        return ;

    _dstc_lock_context(ctx);

    // Does it even exist in our poll set.
    HASH_FIND_INT(ctx->poll_hash, &descriptor, pelem);
    if (!pelem) {
        RMC_LOG_INDEX_FATAL(event_user_data, "File descriptor %d not found in poll set\n", descriptor);
        exit(255);
    }

    pelem->pfd.events = 0;
    pelem->pfd.revents = 0;

    if (new_action & RMC_POLLREAD)
        pelem->pfd.events |= POLLIN;

    if (new_action & RMC_POLLWRITE)
        pelem->pfd.events |= POLLOUT;

    RMC_LOG_COMMENT("poll_modify(%d) read[%c] write[%c]\n",
                    descriptor,
                    ((new_action & RMC_POLLREAD)?'y':'n'),
                    ((new_action & RMC_POLLWRITE)?'y':'n'));
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
    poll_elem_t* pelem = 0;
    _dstc_lock_context(ctx);

    // Does it even exist in our poll set.
    HASH_FIND_INT(ctx->poll_hash, &descriptor, pelem);
    if (!pelem) {
        RMC_LOG_FATAL("File descriptor %d not found in poll set\n", descriptor);
        exit(255);
    }

    RMC_LOG_COMMENT("poll_remove(%d)\n",
                    descriptor);

    HASH_DEL(ctx->poll_hash, pelem);

    pelem->pfd.events = 0;
    pelem->pfd.revents = 0;
    pelem->pfd.fd = -1;
    _dstc_unlock_context(ctx);
}



static void _dstc_process_poll_result(dstc_context_t* ctx,
                                       struct pollfd* event)

{

    uint8_t op_res = 0;
    poll_elem_t* pelem = 0;
    int res = 0;


    // Does it even exist in our poll set.
    HASH_FIND_INT(ctx->poll_hash, &event->fd, pelem);
    if (!pelem) {
        RMC_LOG_INFO("File descriptor %d not found in poll set. Probably deleted by other thread\n", event->fd);
        return;
    }

    rmc_index_t c_ind = (rmc_index_t) FROM_POLL_EVENT_USER_DATA(pelem->user_data);
    int is_pub = IS_PUB(pelem->user_data);

    RMC_LOG_INDEX_DEBUG(c_ind, "desc[%d/%d] ind[%d] user_data[%u] %s:%s%s%s",
                        event->fd,
                        pelem->pfd.fd,
                        c_ind,
                        FROM_POLL_EVENT_USER_DATA(pelem->user_data),
                        (is_pub?"pub":"sub"),
                        ((event->revents & POLLIN)?" read":""),
                        ((event->revents & POLLOUT)?" write":""),
                        ((event->revents & POLLHUP)?" disconnect":""));


    if (event->revents & POLLIN) {
        if (is_pub)
            rmc_pub_read(ctx->pub_ctx, c_ind, &op_res);
        else
            rmc_sub_read(ctx->sub_ctx, c_ind, &op_res);
    }

    if (event->revents & POLLOUT) {
        if (is_pub) {
            if ((res = rmc_pub_write(ctx->pub_ctx, c_ind, &op_res))  != 0) {
                RMC_LOG_INFO("rmc_pub_write(%d) failed: %s - %s", c_ind,
                             strerror(res), op_res);
                rmc_pub_close_connection(ctx->pub_ctx, c_ind);
            }
        } else {
            if ((res = rmc_sub_write(ctx->sub_ctx, c_ind, &op_res)) != 0) {
                RMC_LOG_INFO("rmc_sub_write(%d) failed: %s - %s", c_ind,
                             strerror(res), op_res);
                rmc_sub_close_connection(ctx->sub_ctx, c_ind);
            }
        }
    }
}

int _dstc_process_single_event(dstc_context_t* ctx, int timeout_msec)
{
    struct pollfd pfd[DSTC_MAX_CONNECTIONS];
    int n_hits = 0;
    int n_events = 0;
    int ind = 0;
    poll_elem_t* iter;

    _dstc_lock_context(ctx);
    iter = ctx->poll_hash;
    // Fill out pfd.
    // SLOW!
    while(iter) {
        pfd[n_events] = iter->pfd;
        pfd[n_events].revents = 0;
        iter = iter->hh.next;
        n_events++;
    }
    _dstc_unlock_context(ctx);

    do {
        errno = 0;
        RMC_LOG_DEBUG("Will poll for %d milleseconds.", timeout_msec);
        n_hits = poll(pfd, n_events, timeout_msec);
        RMC_LOG_DEBUG("Done polling.: %d / %s", n_hits, strerror(errno));
    } while(n_hits == -1 && errno == EINTR);

    if (n_hits == -1) {
        RMC_LOG_FATAL("poll(): %s", strerror(errno));
        exit(255);
    }

    // Timeout
    if (n_hits == 0)
        return ETIME;

    // Process all pending event.s

    _dstc_lock_context(ctx);

    while(ind < n_events && n_hits) {
        if (pfd[ind].revents) {
            _dstc_process_poll_result(ctx, &pfd[ind]);
            n_hits--;
        }
        ++ind;
    }

    _dstc_unlock_context(ctx);
    return 0;
}

void dstc_process_poll_result(struct pollfd* events, int nevents)
{
    extern dstc_context_t _dstc_default_context;

    // Prep for future, caller-provided contexct.
    dstc_context_t* ctx = &_dstc_default_context;

    _dstc_lock_and_init_context(ctx);
    while(nevents--)
        if (events[nevents].revents)
            _dstc_process_poll_result(ctx, &events[nevents]);

    _dstc_unlock_context(ctx);
}
