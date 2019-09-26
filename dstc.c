// Copyright (C) 2018, Jaguar Land Rover
// This program is licensed under the terms and conditions of the
// Mozilla Public License, version 2.0.  The full text of the
// Mozilla Public License is at https://www.mozilla.org/MPL/2.0/
//
// Author: Magnus Feuer (mfeuer1@jaguarlandrover.com)

// Server that can load and execute lambda functions.
// See README.md for details

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#if (defined(__linux__) || defined(__ANDROID__)) && !defined(USE_POLL)
#include <sys/epoll.h>
#else
#include <poll.h>
#endif

#include "dstc_internal.h"

#include <rmc_log.h>

#define SUSPEND_TRAFFIC_THRESHOLD 3000
#define RESTART_TRAFFIC_THRESHOLD 2800


// Default context to use if caller does not supply one.
//

dstc_context_t _dstc_default_context = {
#ifdef __APPLE__
    .lock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER,
#else
    .lock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP,
#endif
    .remote_node = { { 0, { 0 } } },
    .remote_node_ind = 0,
    .local_callback = { {0,0 } },

#if (defined(__linux__) || defined(__ANDROID__)) && !defined(USE_POLL)
    .epoll_fd = -1,
#else
    .poll_hash = 0,
#endif

    .client_func = { { { 0 }, 0 } },
    .client_func_ind = 0 ,
    .client_callback_count = 0,
    .server_func = { { { 0 }, 0 } },
    .server_func_ind = 0,
    .sub_ctx = 0,
    .pub_ctx = 0,
    .pub_buffer = { 0 },
    .pub_buffer_ind = 0,
    .pub_is_buffering= 0
};




typedef struct {
    rmc_node_id_t node_id;
    char name[256];
} dstc_control_message_t;


char* _op_res_string(uint8_t res)
{
    switch(res) {
    case RMC_ERROR:
        return "error";

    case RMC_READ_MULTICAST:
        return "read multicast";

    case RMC_READ_MULTICAST_LOOPBACK:
        return "multicast loopback";

    case RMC_READ_MULTICAST_NEW:
        return "new multicast";

    case RMC_READ_MULTICAST_NOT_READY:
        return "multicast not ready";

    case RMC_READ_TCP:
        return "read tcp";

    case RMC_READ_ACCEPT:
        return "accept";

    case RMC_READ_DISCONNECT:
        return "disconnect";

    case RMC_WRITE_MULTICAST:
        return "write multicast";

    case RMC_COMPLETE_CONNECTION:
        return "complete connection";

    case RMC_WRITE_TCP:
        return "tcp write";

    default:
        return "[unknown]";

    }
}

//
// ---------------------------------------------------------
// Internal support functions
// ---------------------------------------------------------
//


static int _dstc_context_initialized(dstc_context_t* ctx)
{
    if (!ctx->pub_ctx)
        return 0;

    return 1;
}


int _dstc_lock_context_timeout(dstc_context_t* ctx, struct timespec* abs_timeout, int line)
{

    if (!pthread_mutex_trylock(&ctx->lock))
        return ENOTBLK;

// Apple does not have pthread_mutex_timedlock(), so we have
// to emulate it using stupid busy wait.
#if __APPLE__
    int result = 0;
    msec_timestamp_t abs_timeout_msec =
        (msec_timestamp_t) abs_timeout->tv_sec * 1000 +
        abs_timeout->tv_nsec / 1000000;

    do
    {
        struct timespec ts;
        int status = -1;

        result = pthread_mutex_trylock(&ctx->lock);
        if (!result || result != EBUSY)
            return result;

        //

        ts.tv_sec = 0;
        ts.tv_sec = 5000000;

        // Jesus this is ugly
        while (status == -1)
            status = nanosleep(&ts, &ts);
    }
    while (result == EBUSY && dstc_msec_monotonic_timestamp() < abs_timeout_msec);
    return ETIME;

#else

    if (pthread_mutex_timedlock(&ctx->lock, abs_timeout)) {
        return ETIME;
    }
    return 0;
#endif
}

int __dstc_lock_context(dstc_context_t* ctx, int line)
{
    pthread_mutex_lock(&ctx->lock);
    return 0;
}


void __dstc_unlock_context(dstc_context_t* ctx, int line)
{
    pthread_mutex_unlock(&ctx->lock);
}

int __dstc_lock_and_init_context_timeout(dstc_context_t* ctx, struct timespec* abs_timeout, int line)
{
    int ret = 0;

    ret = _dstc_lock_context_timeout(ctx, abs_timeout, line);

    if (ret && ret != ENOTBLK)
        return ret;

    if (!_dstc_context_initialized(ctx))
        return dstc_setup();

    return ret;
}

int __dstc_lock_and_init_context(dstc_context_t* ctx, int line)
{
    int ret = 0;

    ret = __dstc_lock_context(ctx, line);

    if (ret)
        return ret;

    if (!_dstc_context_initialized(ctx))
        return dstc_setup();

    return 0;
}


// ctx must be non-null and locked
static uint32_t _dstc_payload_buffer_in_use(dstc_context_t* ctx)
{
    return ctx->pub_buffer_ind;
}

static uint32_t _dstc_get_socket_count(dstc_context_t* ctx)
{
    // Grab the count of all open sockets.
    return rmc_sub_get_socket_count(ctx->sub_ctx) +
        rmc_pub_get_socket_count(ctx->pub_ctx);
}

// ctx must be non-null and locked
static uint32_t _dstc_payload_buffer_available(dstc_context_t* ctx)
{
    return  sizeof(ctx->pub_buffer) - ctx->pub_buffer_ind;
}

// ctx must be non-null and locked
static uint8_t* _dstc_payload_buffer(dstc_context_t* ctx)
{
    return ctx->pub_buffer;
}

// ctx must be non-null and locked
static uint8_t* _dstc_payload_buffer_alloc(dstc_context_t* ctx, uint32_t size)
{
    uint8_t* res = 0;
    if (_dstc_payload_buffer_available(ctx) < size)
        return 0;

    res = _dstc_payload_buffer(ctx) + _dstc_payload_buffer_in_use(ctx);
    ctx->pub_buffer_ind += size;
    return res;
}


// ctx must be non-null and locked
static uint8_t* _dstc_payload_buffer_empty(dstc_context_t* ctx)
{
    ctx->pub_buffer_ind = 0;
    return 0;
}

static msec_timestamp_t _dstc_get_next_timeout_abs(void)
{
    // Prep for future, caller-provided contexct.
    dstc_context_t* ctx = &_dstc_default_context;

    usec_timestamp_t sub_event_tout_ts = 0;
    usec_timestamp_t pub_event_tout_ts = 0;

    _dstc_lock_and_init_context(ctx);

    rmc_pub_timeout_get_next(ctx->pub_ctx, &pub_event_tout_ts);
    rmc_sub_timeout_get_next(ctx->sub_ctx, &sub_event_tout_ts);

    _dstc_unlock_context(ctx);

    // Figure out the shortest event timeout between pub and sub context
    if (pub_event_tout_ts == -1 && sub_event_tout_ts == -1)
        return -1;

    if (pub_event_tout_ts == -1 && sub_event_tout_ts != -1)
        return sub_event_tout_ts / 1000;

    if (pub_event_tout_ts != -1 && sub_event_tout_ts == -1)
        return pub_event_tout_ts / 1000;

    return (pub_event_tout_ts < sub_event_tout_ts)?
        (pub_event_tout_ts / 1000):(sub_event_tout_ts / 1000);
}

// Retrieve a function pointer by name previously registered with
// dstc_register_server_function()
//
// ctx must be non-null and locked
static dstc_internal_dispatch_t _dstc_find_server_function(dstc_context_t* ctx,
                                                           char* name)
{
    int i = ctx->server_func_ind;
    while(i--) {
        if (!strcmp(ctx->server_func[i].func_name, name)) {
            return ctx->server_func[i].server_func;
        }
    }

    return (dstc_internal_dispatch_t) 0;
}


// ctx must be non-null and locked
static int _queue_pending_calls(dstc_context_t* ctx)
{

    // If we have pending data, and we are not suspended, queue the
    // payload with reliable multicast.
    if (rmc_pub_traffic_suspended(ctx->pub_ctx) == 0 &&
        // Do we have data that we need to queue?
        _dstc_payload_buffer_in_use(ctx) > 0) {
        uint8_t* rmc_data = malloc(_dstc_payload_buffer_in_use(&_dstc_default_context));

        if (!rmc_data) {
            RMC_LOG_FATAL("malloc(%d): %s", _dstc_payload_buffer_in_use(&_dstc_default_context), strerror(errno));
            exit(255);
        }

        memcpy(rmc_data, _dstc_payload_buffer(&_dstc_default_context), _dstc_payload_buffer_in_use(&_dstc_default_context));
        // This should never fail since we are not suspended.
        if (rmc_pub_queue_packet(_dstc_default_context.pub_ctx,
                                 rmc_data,
                                 _dstc_payload_buffer_in_use(&_dstc_default_context),
                                 0) != 0) {
            RMC_LOG_FATAL("Failed to queue packet.");
            exit(255);
        }

        // Was the queueing successful?
        RMC_LOG_DEBUG("Queued %d bytes from payload buffer.", _dstc_payload_buffer_in_use(&_dstc_default_context));
        // Empty payload buffer.
        _dstc_payload_buffer_empty(&_dstc_default_context);
    }
    return 0;
}


// Retrieve a callback function. Each time it is invoked, it will be deleted.
// dstc_register_server_function()
//
// ctx must be non-null and locked
static dstc_internal_dispatch_t _dstc_find_callback_by_func(dstc_context_t* ctx,
                                                            dstc_internal_dispatch_t func)
{
    int i = 0;

    while(i < ctx->callback_ind) {
        if (ctx->local_callback[i].callback == func) {
            dstc_internal_dispatch_t res = ctx->local_callback[i].callback;
            // Nil out the callback since it is a one-time shot thing.
            ctx->local_callback[i].callback = 0;
            ctx->local_callback[i].callback_ref = 0;
            return res;
        }
        ++i;
    }
    RMC_LOG_COMMENT("Did not find callback [%p]\n", func);
    return (dstc_internal_dispatch_t) 0;
}

// ctx must be non-null and locked
static dstc_internal_dispatch_t _dstc_find_callback_by_ref(dstc_context_t* ctx,
                                                           dstc_callback_t callback_ref)
{
    int i = 0;

    while(i < ctx->callback_ind) {
        if (ctx->local_callback[i].callback_ref == callback_ref) {
            dstc_internal_dispatch_t res = ctx->local_callback[i].callback;
            // Nil out the callback since it is a one-time shot thing.
            ctx->local_callback[i].callback = 0;
            ctx->local_callback[i].callback_ref = 0;
            return res;
        }
        ++i;
    }
    RMC_LOG_COMMENT("Did not find callback [%lX]", callback_ref);
    return (dstc_internal_dispatch_t) 0;
}

// Activate a client-side callback that can be invoked from a remote
// DSTC function called from the client.  Called by the
// CLIENT_CALLBACK_ARG() macro to register a relationship between a
// callback reference integer and a pointer to the dispatch function
// that handles the incoming callback from the remote DSTC function.
// callback_ref is a pointer to the callback function, but can be any
// unique uint64_t. This integer is passed as a reference to the
// remote DSTC function, which will send it back to the client
// in order to invoke the local callback.
//
// client-side dstc_process_function_call() will detect that
// a callback is being invoked and will use _dstc_find_callback_by_ref()
// to map the provided reference callback integer to a dispatch function,
// which is then called.
// _dstc_find_callback_by_ref() will also de-activate the callback,
// stopping it from being invoked multiple time.
//
dstc_callback_t dstc_activate_callback(dstc_context_t* ctx,
                                       dstc_callback_t callback_ref,
                                       dstc_internal_dispatch_t callback)
{
    int ind = 0;

    if (!ctx)
        ctx = &_dstc_default_context;

    // Find a previously freed slot, or allocate a new one
    while(ind < ctx->callback_ind) {
        if (!ctx->local_callback[ind].callback)
            break;
        ++ind;
    }

    // Are we out of memory
    if (ind == SYMTAB_SIZE) {
        RMC_LOG_FATAL("Out of memory trying to register callback. SYMTAB_SIZE=%d\n", SYMTAB_SIZE);
        exit(255);
    }
    ctx->local_callback[ind].callback_ref = callback_ref;
    ctx->local_callback[ind].callback = callback;
    RMC_LOG_COMMENT("Registered server callback [%llX] to %p. Index[%d]",
                    callback_ref, callback, ind);

    // If we are allocating a new slot (not reusing an earlier one).
    // then bump callback_ind to the new max index in use.
    if (ind == ctx->callback_ind)
        ctx->callback_ind++;

    return callback_ref;
}

// Register a remote function as provided by the remote DSTC server
// through a control message call processed by
// dstc_subscriber_control_message_cb()
//

// ctx must be non-null and locked
static void dstc_register_remote_function(dstc_context_t* ctx,
                                          rmc_node_id_t node_id,
                                          char* func_name)
{
    int ind = 0;
    dstc_remote_node_t* remote = 0;

    // See if the node has registered any prior functions
    // If so, check that we don't have a duplicate and then register
    // the new function.
    ind = ctx->remote_node_ind;
    while(ind--) {
        if (node_id == ctx->remote_node[ind].node_id &&
            !strcmp(func_name, ctx->remote_node[ind].func_name)) {
            RMC_LOG_WARNING("Remote function [%s] registered several times by node [0x%X]",
                            func_name, node_id);
            return;
        }
    }

    if (ctx->remote_node_ind == SYMTAB_SIZE) {
        RMC_LOG_FATAL("Out of memory trying to register remote func. SYMTAB_SIZE=%d\n", SYMTAB_SIZE);
        exit(255);
    }

    remote = &ctx->remote_node[ctx->remote_node_ind];
    remote->node_id = node_id;
    strcpy(remote->func_name, func_name);

    ctx->remote_node_ind++;
    RMC_LOG_INFO("Remote [%s] now supported by new node [0x%X]",
                 func_name, node_id);
    return;
}


// Remove all functions previously registered by node_id through
// the dstc_register_remote_function() call.
//
// ctx must be non-null and locked
static void dstc_unregister_remote_node(dstc_context_t* ctx,
                                        rmc_node_id_t node_id)
{
    int ind = ctx->remote_node_ind;

    while(ind--) {
        if (node_id == ctx->remote_node[ind].node_id) {
            RMC_LOG_INFO("Uhregistering node [0x%X] function [%s]",
                         ctx->remote_node[ind].node_id,
                         ctx->remote_node[ind].func_name);

            ctx->remote_node[ind].node_id = 0;
            ctx->remote_node[ind].func_name[0] = 0;
        }
    }
}


// ctx must be set and locked.
static uint32_t dstc_process_function_call(dstc_context_t* ctx,
                                           uint8_t* data,
                                           uint32_t data_len)
{
    dstc_header_t* call = (dstc_header_t*) data;
    dstc_internal_dispatch_t local_func_ptr = 0;
    dstc_callback_t callback_ref = 0;

    if (data_len < sizeof(dstc_header_t)) {
        RMC_LOG_WARNING("Packet header too short! Wanted %ld bytes, got %d",
                        sizeof(dstc_header_t), data_len);
        return data_len; // Emtpy buffer
    }

    if (data_len - sizeof(dstc_header_t) < call->payload_len) {
        RMC_LOG_WARNING("Packet payload too short! Wanted %d bytes, got %d",
                        call->payload_len, data_len - sizeof(dstc_header_t));
        return data_len; // Emtpy buffer
    }

    // Retrieve function pointer from name, as previously
    // registered with dstc_register_server_function()
    RMC_LOG_DEBUG("DSTC Serve: node_id[%lu] name[%s] payload_len[%d]",
                  call->node_id,
                  call->payload,
                  call->payload_len - strlen((char*) call->payload) - 1);

    // If the name is not nil-len, then we have an actual server function we need
    // to find and invoke.
    if (call->payload[0]) {
        size_t name_len = strlen((char*) call->payload);
        local_func_ptr = _dstc_find_server_function(ctx, (char*) call->payload);

        if (!local_func_ptr) {
            RMC_LOG_DEBUG("Function [%s] not loaded. Ignored", call->payload);
            return sizeof(dstc_header_t) + call->payload_len;
        }

        RMC_LOG_DEBUG("Making local function call node_id[%u] func_name[%s] payload_len[%u]",
                      call->node_id,
                      call->payload,
                      call->payload_len - name_len - 1);

        (*local_func_ptr)(0, // Callback ref is 0
                          call->node_id,
                          call->payload, // function name
                          call->payload + name_len + 1, // Payload
                          call->payload_len - name_len - 1);  // Payload len

        return sizeof(dstc_header_t) + call->payload_len;
    }

    // If name is nil-len, then the eight bytes after the initial \0 is
    // the callback reference value
    callback_ref = *((dstc_callback_t*)(call->payload + 1));
    local_func_ptr = _dstc_find_callback_by_ref(ctx, callback_ref);

    if (!local_func_ptr) {
        RMC_LOG_COMMENT("Callback [%llu] not loaded. Ignored", (long long unsigned) callback_ref);
        return sizeof(dstc_header_t) + call->payload_len;
    }
    (*local_func_ptr)(callback_ref,
                      call->node_id,
                      call->payload, // Funcation name. Always ""
                      call->payload + 1 + sizeof(uint64_t),// Payload after nil name and uint64_t
                      call->payload_len - 1 - sizeof(uint64_t));  // Payload len

    return sizeof(dstc_header_t) + call->payload_len;
}

static void dstc_subscription_complete(rmc_sub_context_t* sub_ctx,
                                       uint32_t listen_ip,
                                       in_port_t listen_port,
                                       rmc_node_id_t node_id)
{
    dstc_context_t* ctx = (dstc_context_t*) rmc_sub_user_data(sub_ctx).ptr;
    int ind = 0;

    _dstc_lock_context(ctx);
    ind = ctx->server_func_ind;


    RMC_LOG_COMMENT("Subscription complete. Sending supported functions.");

    // Retrieve function pointer from name, as previously
    // registered with dstc_registerctx->local_function()
    // Include null terminator for an easier life.
    while(ind--) {
        RMC_LOG_COMMENT("  [%s]", ctx->server_func[ind].func_name);
        dstc_control_message_t ctl = {
            .node_id = rmc_pub_node_id(ctx->pub_ctx)
        };

        strcpy(ctl.name, ctx->server_func[ind].func_name);

        rmc_sub_write_control_message_by_node_id(sub_ctx,
                                                 node_id,
                                                 &ctl,
                                                 sizeof(rmc_node_id_t) +
                                                 sizeof(uint8_t) +
                                                 strlen(ctl.name) + 1);

    }

    _dstc_unlock_context(ctx);
    RMC_LOG_COMMENT("Done sending functions");
    return;
}

static void dstc_process_incoming(rmc_sub_context_t* sub_ctx)
{
    rmc_sub_packet_t* pack = 0;
    dstc_context_t* ctx = (dstc_context_t*) rmc_sub_user_data(sub_ctx).ptr;

    RMC_LOG_DEBUG("Processing incoming");

    _dstc_lock_context(ctx);
    while((pack = rmc_sub_get_next_dispatch_ready(sub_ctx))) {
        uint32_t ind = 0;
        void* payload = rmc_sub_packet_payload(pack);
        payload_len_t payload_len = rmc_sub_packet_payload_len(pack);

        RMC_LOG_DEBUG("Got packet. payload_len[%d]", payload_len);

        // We need to mark the packet as dispatched before we make the function calls,
        // Since any calls to dstc_process_events() from inside the invoked funciton
        // would lead to recursion.
        //
        rmc_sub_packet_dispatched_keep_payload(sub_ctx, pack);

        while(ind < payload_len) {
            RMC_LOG_DEBUG("Processing function call. ind[%d]", ind);
            ind += dstc_process_function_call(ctx,
                                              ((uint8_t*) payload + ind),
                                              payload_len - ind);
        }
        free(payload);
    }
    _dstc_unlock_context(ctx);
    return;
}


static void dstc_subscriber_control_message_cb(rmc_pub_context_t* pub_ctx,
                                               uint32_t publisher_address,
                                               uint16_t publisher_port,
                                               rmc_node_id_t node_id,
                                               void* payload,
                                               payload_len_t payload_len)
{
    dstc_context_t* ctx = (dstc_context_t*) rmc_pub_user_data(pub_ctx).ptr;

    RMC_LOG_DEBUG("Processing incoming");

    _dstc_lock_context(ctx);
    dstc_control_message_t *ctl = (dstc_control_message_t*) payload;

    dstc_register_remote_function(ctx, ctl->node_id, ctl->name);
    _dstc_unlock_context(ctx);
    return;
}

static void dstc_subscriber_disconnect_cb(rmc_pub_context_t* pub_ctx,
                                          uint32_t publisher_address,
                                          uint16_t publisher_port)
{
    dstc_context_t* ctx = (dstc_context_t*) rmc_pub_user_data(pub_ctx).ptr;

    RMC_LOG_DEBUG("Processing incoming");

    _dstc_lock_context(ctx);

    dstc_unregister_remote_node(ctx, rmc_pub_node_id(pub_ctx));
    _dstc_unlock_context(ctx);
    return;
}

static void free_published_packets(void* pl, payload_len_t len, user_data_t dt)
{
    RMC_LOG_DEBUG("Freeing %p", pl);
    free(pl);
}

static msec_timestamp_t _dstc_msec_monotonic_timestamp(struct timespec* abs_time_res)
{
    clock_gettime(CLOCK_MONOTONIC, abs_time_res);
    return (msec_timestamp_t) abs_time_res->tv_sec * 1000 + abs_time_res->tv_nsec / 1000000;
}

static int _dstc_get_timeout_msec_rel(msec_timestamp_t current_time)
{
    msec_timestamp_t tout = _dstc_get_next_timeout_abs();

    if (tout == -1)
        return -1;

    // Convert to relative timestamp.
    tout -= current_time;

    if (tout < 0)
        return 0;

    return tout + 1;
}

// ctx must be set and locked
static int dstc_setup_internal(dstc_context_t* ctx,
                               rmc_node_id_t node_id,
                               int max_dstc_nodes,
                               char* multicast_group_addr,
                               int multicast_port,
                               char* multicast_iface_addr,
                               int mcast_ttl,
                               char* control_listen_iface_addr,
                               int control_listen_port,
                               int epoll_fd_arg) // Ignored by non Linux/Android
{

#if (defined(__linux__) || defined(__ANDROID__)) && !defined(USE_POLL)
    if (!ctx || epoll_fd_arg == -1)
        return EINVAL;

    ctx->epoll_fd = epoll_fd_arg;

#else
    // Setup a poll vector to use.
    int ind = sizeof(ctx->poll_elem_array) / sizeof(ctx->poll_elem_array[0]);
    if (!ctx)
        return EINVAL;

    ctx->poll_hash  = (poll_elem_t*) 0;

    while(ind--)
        ctx->poll_elem_array[ind]  = (poll_elem_t) {
            .pfd = {
                .fd = -1, // Indicates that element is not allocated
                .events = 0x00,
                .revents = 0x00},
            .user_data = 0
        };
#endif

    ctx->remote_node_ind = 0;
    ctx->callback_ind = 0;
    ctx->pub_buffer_ind = 0;
    ctx->pub_ctx = 0;
    ctx->sub_ctx = 0;

    // Do not touch server_func* and client_func* members.
    // since they may have been updated by register_[client,server]_function()
    // constructor functions.

    rmc_log_set_start_time();
    rmc_pub_init_context(&ctx->pub_ctx,
                         node_id, // Node ID
                         multicast_group_addr, multicast_port,
                         multicast_iface_addr,  // Use any NIC address for multicast transmit.
                         control_listen_iface_addr, // Use any NIC address for listen control port.
                         control_listen_port, // Use ephereal tcp port for tcp control
                         user_data_ptr(ctx),
                         // Different versions of
                         // poll_(add|modify|remote) used depending on
                         // Linux/Android/other See poll.c and epoll.c
                         poll_add_pub, poll_modify_pub, poll_remove,
                         DSTC_MAX_CONNECTIONS,
                         free_published_packets);

    // Setup a callback for subscriber disconnect, meaning that remote nodes
    // with functions that we can call can no longer be used.
    rmc_pub_set_subscriber_disconnect_callback(ctx->pub_ctx,
                                               dstc_subscriber_disconnect_cb);

    // Setup a subscriber callback, allowing us to know when a subscribe that can
    // execute the function has attached.
    rmc_pub_set_control_message_callback(ctx->pub_ctx, dstc_subscriber_control_message_cb);

    rmc_pub_throttling(ctx->pub_ctx,
                       SUSPEND_TRAFFIC_THRESHOLD,
                       RESTART_TRAFFIC_THRESHOLD);

    // Subscriber init.
    rmc_sub_init_context(&ctx->sub_ctx,
                         // Reuse pub node id to detect and avoid loopback messages
                         rmc_pub_node_id(ctx->pub_ctx),
                         multicast_group_addr, multicast_port,
                         multicast_iface_addr,  // Use any NIC address for multicast transmit.
                         user_data_ptr(ctx),
                         // Different versions of
                         // poll_(add|modify|remote) used depending on
                         // Linux/Android/other See poll.c and epoll.c
                         poll_add_sub, poll_modify_sub, poll_remove,
                         DSTC_MAX_CONNECTIONS,
                         0,0);

    rmc_sub_set_packet_ready_callback(ctx->sub_ctx, dstc_process_incoming);
    rmc_sub_set_subscription_complete_callback(ctx->sub_ctx, dstc_subscription_complete);

    rmc_pub_set_multicast_ttl(ctx->pub_ctx, mcast_ttl);
    rmc_pub_activate_context(ctx->pub_ctx);
    rmc_sub_activate_context(ctx->sub_ctx);


    RMC_LOG_COMMENT("sub[%d] pub[%d] node[%d] pub[%p] sub[%p]",
                    rmc_sub_get_socket_count(ctx->sub_ctx),
                    rmc_pub_get_socket_count(ctx->pub_ctx),
                    max_dstc_nodes,
                    ctx->sub_ctx,
                    ctx->pub_ctx);

    RMC_LOG_INFO("Node ID[0x%X]", rmc_sub_node_id(ctx->sub_ctx));
    // Start ticking announcements as a client that the server will connect back to.
    // Only do announce if we have client services that requires servers to connect
    // back to us as a subsriber in order to make their remote functions available.
    if (ctx->client_func_ind || ctx->client_callback_count) {
        RMC_LOG_INFO("There are %d DSTC_CLIENT() and %d DSTC_CALLBACK() functions declared. Will send out announce.",
                     ctx->client_func_ind, ctx->client_callback_count);
        rmc_pub_set_announce_interval(ctx->pub_ctx, 200000); // Start ticking announces.
    }
    else
        RMC_LOG_INFO("No DSTC_CLIENT() or DSTC_CALLBACK() functions declared. Will not send out announce.");


    return 0;
}

static int _dstc_queue(dstc_context_t* ctx,
                       char* name,
                       dstc_callback_t callback_ref,
                       uint8_t* arg,
                       uint32_t arg_sz)
{
    // Will be freed by RMC on confirmed delivery
    dstc_header_t *call = 0;
    uint16_t id_len = 0;
    size_t name_len = name?strlen(name):0;;

    if ((!name || name[0] == 0) && !callback_ref) {
        RMC_LOG_ERROR("dstc_queue() needs either name or callback_ref to be set.");
        return EINVAL;
    }

    if (!_dstc_context_initialized(ctx))
        dstc_setup();

    // FIXME: Stuff multiple calls into a single packet. Queue packet
    //        either at timeout (1-2 msec) or when packet is full
    //        (RMC_MAX_PAYLOAD)

    id_len = callback_ref?(sizeof(uint64_t) + 1):(name_len+1);
    call = (dstc_header_t*) _dstc_payload_buffer_alloc(ctx,
                                                       sizeof(dstc_header_t) + id_len + arg_sz);

    // If alloc failed, then we do not have enough space in the
    // payload buffer to store the new call.  Return EBUSY, telling
    // the calling program to run dstc_process_events() or
    // dstc_process_events() for a bit and try again.
    //
    // We will ignore buffer mode here and send the data out via RMC
    // since we need to get our buffer space back.
    // t
    if (!call) {
        // Try to empty buffer.
        _queue_pending_calls(ctx);
        return EBUSY;
    }

    call->node_id = dstc_get_node_id();

    // If this is a regular function call, then copy in the function
    // name, including terminating null character, followed by the
    // payload.
    //
    // If this is a invocation of a previously registered callback, then
    // install a null character as the first byte of payload, followed by
    // the eight bytes of the callback reference that we want invoked,
    // followed by the payload
    //
    if (name) {
        memcpy(call->payload, name, name_len + 1);
        call->payload_len = name_len + 1 + arg_sz;
        memcpy(call->payload + name_len + 1, arg, arg_sz);
    } else {
        call->payload[0] = 0;
        memcpy(call->payload + 1, (uint64_t*) &callback_ref, sizeof(uint64_t));
        call->payload_len = 1 + sizeof(uint64_t) + arg_sz;
        memcpy(call->payload + 1 + sizeof(uint64_t), arg, arg_sz);
    }

    RMC_LOG_DEBUG("DSTC Queue: node_id[%lu] name[%s]/callback_ref[%llu] payload_len[%d] in_use[%d]",
                  call->node_id,
                  name?name:"nil",
                  callback_ref,
                  call->payload_len,
                  _dstc_payload_buffer_in_use(ctx));


    // If we have pending calls in the DSTC circular buffer, try to
    // queue them with RMC.  This may fail if we are currently
    // suspended from sending traffic over RMC due to congestion.
    //
    // If we are in buffered mode, trying to collect as many calls
    // as possible into a single RMC (UDP multicast) packet,
    // then we will not try to queue the call for now.
    // Please see above for queueing calls when our DSTC outbound
    // buffer is full.
    if (!ctx->pub_is_buffering)
        _queue_pending_calls(ctx);

    return 0;
}

//
// ---------------------------------------------------------
// Functions invoked by DSTC_*() macros
// ---------------------------------------------------------
//

//
// Register a function name - pointer relationship.
// Called by file constructor function _dstc_register_server_[name]()
// generated by DSTC_SERVER() macro.
//
// ctx can be unlocked and/or null (for default context)
void dstc_register_server_function(dstc_context_t* ctx,
                                   char* name,
                                   dstc_internal_dispatch_t server_func)
{
    int ind = 0;

    if (!ctx)
        ctx = &_dstc_default_context;

    _dstc_lock_context(ctx);

    ind = ctx->server_func_ind;
    if (ind == SYMTAB_SIZE - 1) {
        RMC_LOG_FATAL("Out of memory trying to register server function. SYMTAB_SIZE=%d\n", SYMTAB_SIZE);
        exit(255);
    }

    strcpy(ctx->server_func[ind].func_name, name);
    ctx->server_func[ind].server_func = server_func;
    ctx->server_func_ind++;
    _dstc_unlock_context(ctx);
}

// ctx can be unlocked and/or null (for default context)
// Called by file constructor function _dstc_register_client_[name]()
// generated by DSTC_CLIENT() macro.

//
void dstc_register_client_function(dstc_context_t* ctx,
                                   char* name,
                                   void *client_func)
{
    int ind = 0;

    if (!ctx)
        ctx = &_dstc_default_context;

    _dstc_lock_context(ctx);

    ind = ctx->client_func_ind;
    if (ind == SYMTAB_SIZE - 1) {
        RMC_LOG_FATAL("Out of memory trying to register client function. SYMTAB_SIZE=%d\n", SYMTAB_SIZE);
        exit(255);
    }

    strcpy(ctx->client_func[ind].func_name, name);
    ctx->client_func[ind].client_func = client_func;
    ctx->client_func_ind++;
    _dstc_unlock_context(ctx);
}


// Register a function name - pointer relationship.
//
// Called by file constructor function
// _dstc_register_callback_sedrver_[name]() generated by
// DSTC_CLIENT_CALLBACK_ARG() macro.
//
void dstc_register_callback_server(dstc_context_t* ctx,
                                   dstc_callback_t callback_ref,
                                   dstc_internal_dispatch_t callback)
{

    int ind = 0;
    if (!ctx)
        ctx = &_dstc_default_context;

    _dstc_lock_context(ctx);


    // Find a previously freed slot, or allocate a new one
    while(ind < ctx->callback_ind) {
        if (!ctx->local_callback[ind].callback)
            break;
        ++ind;
    }

    // Are we out of memory
    if (ind == SYMTAB_SIZE) {
        RMC_LOG_FATAL("Out of memory trying to register callback. SYMTAB_SIZE=%d\n", SYMTAB_SIZE);
        exit(255);
    }
    ctx->local_callback[ind].callback_ref = callback_ref;
    ctx->local_callback[ind].callback = callback;
    RMC_LOG_COMMENT("Registered server callback [%llX] to %p. Index[%d]",
                    callback_ref, callback, ind);

    // If we are allocating a new slot (not reusing an earlier one).
    // then bump callback_ind to the new max index in use.
    if (ind == ctx->callback_ind)
        ctx->callback_ind++;

    _dstc_unlock_context(ctx);
    return;
}

// Register a callback function name - pointer relationship.
//
// Called by file constructor function
// _dstc_register_callback_[name]() generated by DSTC_CALLBACK()
// macro.
//
// For now, we just bump a counter to figure out if we should
// send out announce messages or not.
//
void dstc_register_callback_client(dstc_context_t* ctx,
                                   char* name, void* callback)
{
    if (!ctx)
        ctx = &_dstc_default_context;

    _dstc_lock_context(ctx);
    ctx->client_callback_count++;
    _dstc_unlock_context(ctx);
}


// Returns EBUSY if outbound queues are full
int dstc_queue_callback(dstc_context_t* ctx, dstc_callback_t addr, uint8_t* arg, uint32_t arg_sz)
{
    int res = 0;
    if (!ctx)
        ctx = &_dstc_default_context;

    _dstc_lock_and_init_context(ctx);
    // Call with zero namelen to treat name as a 64bit integer.
    // This integer will be mapped by the received through the
    // ctx->local_callback
    // table to a pending callback function.
    res = _dstc_queue(ctx, 0, addr, arg, arg_sz);
    _dstc_unlock_context(ctx);
    return res;
}

// Returns EBUSY if outbound queues are full
int dstc_queue_func(dstc_context_t* ctx, char* name, uint8_t* arg, uint32_t arg_sz)
{
    int res = 0;

    if (!ctx)
        ctx = &_dstc_default_context;

    _dstc_lock_context(ctx);
    res = _dstc_queue(ctx, name, 0, arg, arg_sz);
    _dstc_unlock_context(ctx);
    return res;
}

//
// ---------------------------------------------------------
// Externally callable functions
 // ---------------------------------------------------------
//

void dstc_cancel_callback(dstc_internal_dispatch_t callback)
{
    // Prep for future, caller-provided contexct.
    dstc_context_t* ctx = &_dstc_default_context;

    _dstc_lock_and_init_context(ctx);

    // Will delete the callback.


    _dstc_find_callback_by_func(ctx, callback);
    _dstc_unlock_context(ctx);
}

uint8_t dstc_remote_function_available_by_name(char* func_name)
{
    // Prep for future, caller-provided contexct.
    dstc_context_t* ctx = &_dstc_default_context;

    int ind = 0;

    _dstc_lock_and_init_context(ctx);

    // Scan all remotely registered nodes and their functions
    // to see if you can find one with a matching na,e
    ind = ctx->remote_node_ind;
    while(ind--) {
        if (ctx->remote_node[ind].node_id != 0 &&
            !strcmp(func_name, ctx->remote_node[ind].func_name)) {
            _dstc_unlock_context(ctx);
            return 1;
        }

    }
    RMC_LOG_DEBUG("Could not find a remote node that had registered function %s", func_name);
    _dstc_unlock_context(ctx);
    return 0;
}


uint8_t dstc_remote_function_available(void* client_func)
{
    // Prep for future, caller-provided contexct.
    dstc_context_t* ctx = &_dstc_default_context;

    int ind = 0;
    uint8_t res = 0;

    _dstc_lock_and_init_context(ctx);

    // Find the string name for the dstc_[func_name] function
    // pointer provided in client_func

    ind = ctx->client_func_ind;
    while(ind--)
        if (ctx->client_func[ind].client_func == client_func)
            break;

    // No hit?
    if (ind == -1) {
        _dstc_unlock_context(ctx);
        return 0;
    }

    res = dstc_remote_function_available_by_name(ctx->client_func[ind].func_name);
    _dstc_unlock_context(ctx);
    return res;
}

msec_timestamp_t dstc_msec_monotonic_timestamp(void)
{
    struct timespec res;
    return _dstc_msec_monotonic_timestamp(&res);
}



int dstc_get_timeout_msec_rel(void)
{
    return _dstc_get_timeout_msec_rel(dstc_msec_monotonic_timestamp());
}

static int _dstc_process_timeout(dstc_context_t* ctx)
{
    // If either of the timeout processor fails in with EAGAIN, then they
    // tried resending un-acknolwedged packets but encountered full transmissions
    // queues in rmc.
    // In that case process events until the queues are sent out on the network
    // and are cleared up.
    if (rmc_pub_timeout_process(ctx->pub_ctx) == EAGAIN ||
        rmc_sub_timeout_process(ctx->sub_ctx) == EAGAIN) {
         return EAGAIN;
    }
    return 0;
}


static int _dstc_process_pending_events(dstc_context_t* ctx)
{
    while(_dstc_process_single_event(ctx, 0) != ETIME)
       ;
    return 0;
}


int dstc_process_pending_events(void)
{
    dstc_context_t* ctx = &_dstc_default_context;
    _dstc_lock_and_init_context(ctx);
    _dstc_process_pending_events(ctx);
    _dstc_unlock_context(ctx);

    return 0;
}

int dstc_process_events(int timeout_rel)
{
    // Prep for future, caller-provided contexct.
    dstc_context_t* ctx = &_dstc_default_context;
    struct timespec abs_time = { 0 };
    msec_timestamp_t start_time = 0;
    int next_dstc_timeout_rel = 0;
    int next_dstc_timeout_abs = 0;
    int retval = 0;
    int lock_res = 0;

    if (timeout_rel == 0) {
        while(1) {
            _dstc_lock_context(ctx);
            if (_dstc_process_single_event(ctx, 0) == ETIME) {
                _dstc_unlock_context(ctx);
                return ETIME;
            }
            _dstc_unlock_context(ctx);
        }
    }

    start_time = _dstc_msec_monotonic_timestamp(&abs_time);
    next_dstc_timeout_rel = _dstc_get_timeout_msec_rel(start_time);
    next_dstc_timeout_abs = start_time + next_dstc_timeout_rel;

//    printf("Timeout_rel[%d]\n", timeout_rel);

    // If we have infinite timeout, and no pending dstc timeouts,
    // wait forever for incoming traffic.
    if (timeout_rel == -1 && next_dstc_timeout_rel == -1) {
        _dstc_lock_context(ctx);
        _dstc_process_single_event(ctx, -1);
        _dstc_unlock_context(ctx);
        return 0;
    }

    // We have specified infinite timeout, however DSTC has
    // its own timeout that we need to adhere to.
    if (timeout_rel == -1)
        timeout_rel = next_dstc_timeout_rel;
    else
        // We have an actual timeout specified, pick the shortest
        // timeout of that and the next DSTC timeout.
        timeout_rel = (next_dstc_timeout_rel != -1 &&
                       next_dstc_timeout_rel < timeout_rel)?
            next_dstc_timeout_rel:timeout_rel;

    // At this point, we are guarnteed that timeout_rel is not -1 (infinite)
    // and specifies a timeout duration, which may be 0.

    // Do we have a zero timeout, or a timeout that has already
    // expired?
    if (timeout_rel <= 0) {
        _dstc_lock_context(ctx);
        _dstc_process_single_event(ctx, 0);
        _dstc_process_timeout(ctx);
        _dstc_unlock_context(ctx);
        return ETIME;
    }

    // We have an actual timeout value
    // Adjust absolute time to the time
    // when we need the mutex lock to stop
    // waiting for acquisition.
    //
    abs_time.tv_nsec += timeout_rel * 1000000;
    abs_time.tv_sec += abs_time.tv_nsec / 1000000000;
    abs_time.tv_nsec = abs_time.tv_nsec % 1000000000;

    // Lock with the given timeout.
    lock_res = _dstc_lock_and_init_context_timeout(ctx, &abs_time);

    // Did we time out?
    if (lock_res == ETIME) {
        _dstc_process_timeout(ctx);
        _dstc_unlock_context(ctx);
        return ETIME;
    }

    // Did we not get the lock immediately, and had to wait for it?
    // If so recalcuate how much time we have left.
    if (lock_res != ENOTBLK)
        timeout_rel = next_dstc_timeout_abs - dstc_msec_monotonic_timestamp();

    if (timeout_rel < 0)
        timeout_rel = 0;
    retval = _dstc_process_single_event(ctx, timeout_rel);

    // Did we time out?
    if (retval == ETIME)
        _dstc_process_timeout(ctx);

    _dstc_unlock_context(ctx);
    return retval;
}

int dstc_process_timeout(void)
{
    // Prep for future, caller-provided contexct.
    dstc_context_t* ctx = &_dstc_default_context;
    int res = 0;

   _dstc_lock_and_init_context(ctx);

    res = _dstc_process_timeout(ctx);
    _dstc_unlock_context(ctx);
    return res;
}

void dstc_buffer_client_calls(void)
{
    // Prep for future, caller-provided contexct.
    dstc_context_t* ctx = &_dstc_default_context;
    _dstc_lock_and_init_context(ctx);
    ctx->pub_is_buffering = 1;
    _dstc_unlock_context(ctx);
}

void dstc_flush_client_calls(void)
{
    // Prep for future, caller-provided contexct.
    dstc_context_t* ctx = &_dstc_default_context;

    _dstc_lock_and_init_context(ctx);

    // Dump buffer into RMC
    _queue_pending_calls(ctx);
    _dstc_unlock_context(ctx);
}

void dstc_unbuffer_client_calls(void)
{
    // Prep for future, caller-provided contexct.
    dstc_context_t* ctx = &_dstc_default_context;

    _dstc_lock_and_init_context(ctx);
    ctx->pub_is_buffering = 0;
    // Dump buffer into RMC
    _queue_pending_calls(ctx);
    _dstc_unlock_context(ctx);
}


uint32_t dstc_get_socket_count(void)
{
    // Prep for future, caller-provided contexct.
    dstc_context_t* ctx = &_dstc_default_context;
    uint32_t res = 0;

    _dstc_lock_and_init_context(ctx);
    res = _dstc_get_socket_count(ctx);

    _dstc_unlock_context(ctx);
    return res;
}


rmc_node_id_t dstc_get_node_id(void)
{
    dstc_context_t* ctx = &_dstc_default_context;
    rmc_node_id_t res = 0;
    _dstc_lock_and_init_context(ctx);

    res = rmc_pub_node_id(ctx->pub_ctx);
    _dstc_unlock_context(ctx);

    return res;
}

int dstc_setup_epoll(int epoll_fd_arg)
{
    char* node_id = getenv(DSTC_ENV_NODE_ID);
    char* max_dstc_nodes = getenv(DSTC_ENV_MAX_NODES);
    char *multicast_group_addr = getenv(DSTC_ENV_MCAST_GROUP_ADDR);
    char *multicast_iface_addr = getenv(DSTC_ENV_MCAST_IFACE_ADDR);
    char *multicast_port = getenv(DSTC_ENV_MCAST_GROUP_PORT);
    char *control_listen_iface_addr = getenv(DSTC_ENV_CONTROL_LISTEN_IFACE);
    char *control_listen_port = getenv(DSTC_ENV_CONTROL_LISTEN_PORT);
    char *mcast_ttl = getenv(DSTC_ENV_MCAST_TTL);
    char *log_level = getenv(DSTC_ENV_LOG_LEVEL);
    int res = 0;

    rmc_set_log_level(log_level?atoi(log_level):RMC_LOG_LEVEL_ERROR);

    RMC_LOG_COMMENT("%s: %s", DSTC_ENV_NODE_ID, node_id?node_id:"[not set]");
    RMC_LOG_COMMENT("%s: %s", DSTC_ENV_MAX_NODES, max_dstc_nodes?max_dstc_nodes:"[not set]");
    RMC_LOG_COMMENT("%s: %s", DSTC_ENV_MCAST_GROUP_ADDR, multicast_group_addr?multicast_group_addr:"[not set]");
    RMC_LOG_COMMENT("%s: %s", DSTC_ENV_MCAST_IFACE_ADDR, multicast_iface_addr?multicast_iface_addr:"[not set]");
    RMC_LOG_COMMENT("%s: %s", DSTC_ENV_MCAST_GROUP_PORT, multicast_port?multicast_port:"[not set]");
    RMC_LOG_COMMENT("%s: %s", DSTC_ENV_MCAST_TTL, mcast_ttl?mcast_ttl:"[not set]");
    RMC_LOG_COMMENT("%s: %s", DSTC_ENV_CONTROL_LISTEN_IFACE, control_listen_iface_addr?control_listen_iface_addr:"[not set]");
    RMC_LOG_COMMENT("%s: %s", DSTC_ENV_CONTROL_LISTEN_PORT, control_listen_port?control_listen_port:"[not set]");

    dstc_context_t* ctx = &_dstc_default_context;

    _dstc_lock_context(ctx);
    res =  dstc_setup_internal(ctx,
                               (node_id?((rmc_node_id_t) strtoul(node_id, 0, 0)):0),
                               (max_dstc_nodes?atoi(max_dstc_nodes):DEFAULT_MAX_DSTC_NODES),
                               multicast_group_addr?multicast_group_addr:DEFAULT_MCAST_GROUP_ADDRESS,
                               (multicast_port?atoi(multicast_port):DEFAULT_MCAST_GROUP_PORT),
                               multicast_iface_addr,
                               (mcast_ttl?atoi(mcast_ttl):DEFAULT_MCAST_TTL),
                               control_listen_iface_addr,
                               (control_listen_port?atoi(control_listen_port):0),
                               epoll_fd_arg);

    _dstc_unlock_context(ctx);
    return res;
}


int dstc_setup(void)
{
#if (defined(__linux__) || defined(__ANDROID__)) && !defined(USE_POLL)
    return dstc_setup_epoll(epoll_create(1));
#else
    return dstc_setup_epoll(-1);
#endif
}

int dstc_setup2(int epoll_fd_arg, // Ignored by non Android/Linux use
                rmc_node_id_t node_id,
                int max_dstc_nodes,
                char* multicast_group_addr,
                int multicast_port,
                char* multicast_iface_addr,
                int mcast_ttl,
                char* control_listen_iface_addr,
                int control_listen_port,
                int log_level)
{
    dstc_context_t* ctx = &_dstc_default_context;
    int res = 0;

    if (_dstc_context_initialized(ctx))
        return EBUSY;

    rmc_set_log_level(log_level);


    _dstc_lock_context(ctx);

    res = dstc_setup_internal(ctx,
                              node_id,
                              max_dstc_nodes,
                              multicast_group_addr,
                              multicast_port,
                              multicast_iface_addr,
                              mcast_ttl,
                              control_listen_iface_addr,
                              control_listen_port,
#if (defined(__linux__) || defined(__ANDROID__)) && !defined(USE_POLL)
                              (epoll_fd_arg != -1)?epoll_fd_arg:epoll_create(1)
#else
                              -1
#endif
        );
    _dstc_unlock_context(ctx);
    return res;
}
