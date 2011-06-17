
#include "common.h"
#include "msgpack-extensions.h"
#include "filter.h"
#include "utils.h"

Filter *filter_get(void)
{
    return sixjack_get_context()->filter;
}

int filter_send_message(Filter * const filter)
{
    ssize_t written = safe_write(filter->upipe_stdin.fd_write,
                                 filter->msgpack_sbuffer->data,
                                 filter->msgpack_sbuffer->size, -1);
    msgpack_sbuffer_clear(filter->msgpack_sbuffer);    
    if (written != (ssize_t) 0) {
        return -1;
    }    
    return 0;
}

msgpack_unpacked *filter_receive_message(Filter * const filter)
{
    msgpack_unpacker * const msgpack_unpacker = &filter->msgpack_unpacker;    
    msgpack_unpacker_destroy(msgpack_unpacker);
    msgpack_unpacker_init(msgpack_unpacker, FILTER_UNPACK_BUFFER_SIZE);
    msgpack_unpacker_reserve_buffer(msgpack_unpacker,
                                    FILTER_READ_BUFFER_SIZE);
    msgpack_unpacked * const message = &filter->message;
    
    ssize_t readnb;    
    while (msgpack_unpacker_next(msgpack_unpacker, message) == false) {
        assert(msgpack_unpacker_buffer_capacity(msgpack_unpacker) > 0U);
        readnb = safe_read_partial
            (filter->upipe_stdout.fd_read,
                msgpack_unpacker_buffer(msgpack_unpacker),
                msgpack_unpacker_buffer_capacity(msgpack_unpacker));
        if (readnb <= (ssize_t) 0) {
            assert(0);
            return NULL;
        }
        msgpack_unpacker_buffer_consumed(msgpack_unpacker, readnb);
    }
    assert(message->data.type == MSGPACK_OBJECT_MAP);
    
    return message;
}

FilterReplyResult filter_parse_common_reply_map(const msgpack_object_map * const map,
                                                int * const ret,
                                                int * const ret_errno,
                                                const int fd)
{
    const msgpack_object * const obj_version =
        msgpack_get_map_value_for_key(map, "version");
    assert(obj_version != NULL);
    assert(obj_version->type == MSGPACK_OBJECT_POSITIVE_INTEGER);
    assert(obj_version->via.u64 == VERSION_MAJOR);
    
    const msgpack_object * const obj_return_code =
        msgpack_get_map_value_for_key(map, "return_code");
    if (obj_return_code != NULL) {
        assert(obj_return_code->type == MSGPACK_OBJECT_POSITIVE_INTEGER ||
               obj_return_code->type == MSGPACK_OBJECT_NEGATIVE_INTEGER);
        const int64_t new_return_code = obj_return_code->via.i64;
        assert(new_return_code >= INT_MIN && new_return_code <= INT_MAX);
        *ret = new_return_code;
    }
    
    const msgpack_object * const obj_ret_errno =
        msgpack_get_map_value_for_key(map, "errno");
    if (obj_ret_errno != NULL) {
        assert(obj_ret_errno->type == MSGPACK_OBJECT_POSITIVE_INTEGER ||
               obj_ret_errno->type == MSGPACK_OBJECT_NEGATIVE_INTEGER);
        const int64_t new_ret_errno = obj_ret_errno->via.i64;
        assert(new_ret_errno >= INT_MIN && new_ret_errno <= INT_MAX);
        *ret_errno = new_ret_errno;
    }
    
    const msgpack_object * const obj_force_close =
        msgpack_get_map_value_for_key(map, "force_close");
    if (obj_force_close != NULL) {
        assert(obj_force_close->type == MSGPACK_OBJECT_BOOLEAN);
        const bool force_close = obj_force_close->via.boolean;
        if (force_close != false) {
            close(fd);
        }
    }    

    const msgpack_object * const obj_bypass =
        msgpack_get_map_value_for_key(map, "bypass");
    if (obj_bypass != NULL) {
        assert(obj_bypass->type == MSGPACK_OBJECT_BOOLEAN);
        const bool bypass = obj_bypass->via.boolean;
        if (bypass != false) {
            return FILTER_REPLY_BYPASS;
        }
    }
    return FILTER_REPLY_PASS;
}

int filter_before_apply(const bool pre,
                        const int ret, const int ret_errno, const int fd,
                        const unsigned int nongeneric_items,
                        const char * const function,
                        const struct sockaddr_storage * const sa_local,
                        const socklen_t sa_local_len,
                        const struct sockaddr_storage * const sa_remote,
                        const socklen_t sa_remote_len)
{
    Filter * const filter = filter_get();
    const AppContext * const context = sixjack_get_context();    
    assert(filter->msgpack_packer != NULL);
    msgpack_packer * const msgpack_packer = filter->msgpack_packer;
    msgpack_packer_init(msgpack_packer, filter->msgpack_sbuffer,
                        msgpack_sbuffer_write);
    unsigned int items_count = nongeneric_items + 5U;
    if (pre == false) {
        items_count += 2U;
    }
    if (sa_local != NULL) {
        items_count += 2U;
    }
    if (sa_remote != NULL) {
        items_count += 2U;
    }
    msgpack_pack_map(msgpack_packer, items_count);
    
    msgpack_pack_mstring(msgpack_packer, "version");
    msgpack_pack_unsigned_short(msgpack_packer, VERSION_MAJOR);
    
    msgpack_pack_mstring(msgpack_packer, "filter_type");
    msgpack_pack_cstring(msgpack_packer, pre ? "PRE" : "POST");
    
    msgpack_pack_mstring(msgpack_packer, "pid");
    msgpack_pack_unsigned_int(msgpack_packer, context->pid);

    msgpack_pack_mstring(msgpack_packer, "function");
    msgpack_pack_cstring(msgpack_packer, function);

    msgpack_pack_mstring(msgpack_packer, "fd");
    msgpack_pack_int(msgpack_packer, fd);    

    if (pre == false) {
        msgpack_pack_mstring(msgpack_packer, "return_code");
        msgpack_pack_int(msgpack_packer, ret);
        
        msgpack_pack_mstring(msgpack_packer, "errno");
        msgpack_pack_int(msgpack_packer, ret_errno);
    }
    
    char host[NI_MAXHOST];
    in_port_t port;    
    if (sa_local != NULL) {
        get_name_info(sa_local, sa_local_len, host, &port);
        msgpack_pack_mstring(msgpack_packer, "local_host");
        msgpack_pack_cstring_or_nil(msgpack_packer, host);
        msgpack_pack_mstring(msgpack_packer, "local_port");
        msgpack_pack_unsigned_short(msgpack_packer, (unsigned short) port);
    }
    if (sa_remote != NULL) {
        get_name_info(sa_remote, sa_remote_len, host, &port);
        msgpack_pack_mstring(msgpack_packer, "remote_host");
        msgpack_pack_cstring_or_nil(msgpack_packer, host);
        msgpack_pack_mstring(msgpack_packer, "remote_port");
        msgpack_pack_unsigned_short(msgpack_packer, (unsigned short) port);
    }
    return 0;
}

int filter_overwrite_sa_with_reply_map(const msgpack_object_map * const map,
                                       const char * const key_host,
                                       const char * const key_port,
                                       struct sockaddr_storage * const sa,
                                       socklen_t * const sa_len)
{
    if (sa == NULL) {
        return 0;
    }
    const msgpack_object * const obj_host =
        msgpack_get_map_value_for_key(map, key_host);
    if (obj_host != NULL &&
        obj_host->type == MSGPACK_OBJECT_RAW &&
        obj_host->via.raw.size < NI_MAXHOST) {
        struct addrinfo *ai, hints;
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_flags = NI_NUMERICHOST | AI_ADDRCONFIG;
        char new_host[NI_MAXHOST];
        memcpy(new_host, obj_host->via.raw.ptr,
               obj_host->via.raw.size);
        new_host[obj_host->via.raw.size] = 0;
        const int gai_err = getaddrinfo(new_host, NULL, &hints, &ai);
        if (gai_err == 0) {
            assert(ai->ai_addrlen <= sizeof *sa);
            if (ai->ai_family == AF_INET &&
                *sa_len >= (socklen_t)
                sizeof(((struct sockaddr_in *) sa)->sin_addr.s_addr)) {
                assert((size_t) ai->ai_addrlen >=
                       sizeof(((struct sockaddr_in *) sa)->sin_addr.s_addr));
                memcpy(&((struct sockaddr_in *) sa)->sin_addr.s_addr,
                       &((struct sockaddr_in *) ai->ai_addr)->sin_addr.s_addr,
                       sizeof(((struct sockaddr_in *) sa)->sin_addr.s_addr));
                *sa_len = sa->ss_len = ai->ai_addrlen;
                sa->ss_family = ai->ai_family;
                sa->ss_len = ((struct sockaddr_storage *) ai->ai_addr)->ss_len;
            } else if (ai->ai_family == AF_INET6 &&
                      *sa_len >= (socklen_t)
                       sizeof(((struct sockaddr_in6 *) sa)->sin6_addr.s6_addr)) {
                assert((size_t) ai->ai_addrlen >=
                       sizeof(((struct sockaddr_in6 *) sa)->sin6_addr.s6_addr));
                memcpy(&((struct sockaddr_in6 *) sa)->sin6_addr.s6_addr,
                       &((struct sockaddr_in6 *) ai->ai_addr)->sin6_addr.s6_addr,
                       sizeof(((struct sockaddr_in6 *) sa)->sin6_addr.s6_addr));
                *sa_len = sa->ss_len = ai->ai_addrlen;
                sa->ss_family = ai->ai_family;
            }
            freeaddrinfo(ai);
        }
    }    

    const msgpack_object * const obj_port =
        msgpack_get_map_value_for_key(map, key_port);
    if (obj_port != NULL &&
        obj_port->type == MSGPACK_OBJECT_POSITIVE_INTEGER) {
        if (obj_port->via.i64 >= 0 &&
            obj_port->via.i64 <= 65535) {            
            if (sa->ss_family == AF_INET) {
                ((struct sockaddr_in *) sa)->sin_port =
                    htons((in_port_t) obj_port->via.i64);
            } else if (sa->ss_family == AF_INET6) {
                ((struct sockaddr_in6 *) sa)->sin6_port =
                    htons((in_port_t) obj_port->via.i64);
            }
        }
    }   
    return 0;
}
