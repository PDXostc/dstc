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


#define TO_POLL_EVENT_USER_DATA(_index, is_pub) (index | ((is_pub)?USER_DATA_PUB_FLAG:0))
#define FROM_POLL_EVENT_USER_DATA(_user_data) (_user_data & USER_DATA_INDEX_MASK)

static poll_elem_t* find_free_poll_elem_index(dstc_context_t* ctx)
{
    int ind = 0;
    while(ind < sizeof(ctx->poll_elem_array) / sizeof(ctx->poll_elem_array[0])) {
        // Check if file descriptor is unused
        if (ctx->poll_elem_array[ind].pfd.fd == -1)
            return &ctx->poll_elem_array[ind];
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
        RMC_LOG_INDEX_FATAL(event_user_data, "File descriptor %d already in poll set\n", descriptor);
        exit(255);
    }

    pelem = find_free_poll_elem_index(ctx);

    if (!pelem) {
        RMC_LOG_INDEX_FATAL(event_user_data, "Out of poll_elem_t elements. Rebuild with larger DSTC_MAX_CONNECTIONS");
        exit(255);
    }

    pelem->pfd.events = 0;
    pelem->pfd.revents = 0;
    if (action & RMC_POLLREAD)
        pelem->pfd.events |= POLLIN;

    if (action & RMC_POLLWRITE)
        pelem->pfd.events |= POLLOUT;

    pelem->pfd.fd = descriptor;
    HASH_ADD_INT(ctx->poll_hash, pfd.fd, pelem);
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

    _dstc_lock_context(ctx);

    // Does it even exist in our poll set.
    HASH_FIND_INT(ctx->poll_hash, &event->fd, pelem);
    if (!pelem) {
        RMC_LOG_FATAL("File descriptor %d not found in poll set\n", event->fd);
        exit(255);
    }

    rmc_index_t c_ind = (rmc_index_t) FROM_POLL_EVENT_USER_DATA(pelem->user_data);
    int is_pub = IS_PUB(pelem->user_data);

    RMC_LOG_INDEX_DEBUG(c_ind, "%s: %s%s%s",
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
            if (rmc_pub_write(ctx->pub_ctx, c_ind, &op_res) != 0)
                rmc_pub_close_connection(ctx->pub_ctx, c_ind);
        } else {
            if (rmc_sub_write(ctx->sub_ctx, c_ind, &op_res) != 0)
                rmc_sub_close_connection(ctx->sub_ctx, c_ind);
        }
    }
}

int _dstc_process_single_event(dstc_context_t* ctx, int timeout_msec)
{
    struct pollfd pfd[DSTC_MAX_CONNECTIONS];
    int nfds = 0;
    int ind = 0;

    do {
        errno = 0;
        nfds = poll(pfd, sizeof(pfd) / sizeof(pfd[0]), timeout_msec);
    } while(nfds == -1 && errno == EINTR);

    if (nfds == -1) {
        RMC_LOG_FATAL("poll(): %s", strerror(errno));
        exit(255);
    }

    // Timeout
    if (nfds == 0)
        return ETIME;

    // Process all pending event.s
    ind = sizeof(pfd) / sizeof(pfd[0]);
    while(ind-- && nfds) {
        if (pfd[ind].revents) {
            _dstc_process_poll_result(ctx, &pfd[ind]);
            nfds--;
        }
    }

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
