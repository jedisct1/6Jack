
#define DEFINE_HOOK_GLOBALS 1
#define DONT_BYPASS_HOOKS   1

#include "common.h"
#include "filter.h"
#include "hook-recv.h"

ssize_t (* __real_recv)(int fd, void *buf, size_t nbyte, int flags);

static FilterReplyResult filter_parse_reply(const bool pre,
                                            Filter * const filter,
                                            int * const ret,
                                            int * const ret_errno,
                                            const int fd,
                                            void * const buf,
                                            size_t * const nbyte,
                                            int * const flags)
{
    msgpack_unpacked * const message = filter_receive_message(filter);
    const msgpack_object_map * const map = &message->data.via.map;
    FilterReplyResult reply_result =
        filter_parse_common_reply_map(map, ret, ret_errno, fd);

    if (pre != false) {
        const msgpack_object * const obj_nbyte =
            msgpack_get_map_value_for_key(map, "nbyte");
        if (obj_nbyte != NULL &&
            obj_nbyte->type == MSGPACK_OBJECT_POSITIVE_INTEGER) {
            const int64_t new_nbyte = obj_nbyte->via.i64;
            if (new_nbyte <= INT_MAX) {
                *nbyte = new_nbyte;
            }
        }
        const msgpack_object * const obj_flags =
            msgpack_get_map_value_for_key(map, "flags");
        if (obj_flags != NULL &&
            (obj_flags->type == MSGPACK_OBJECT_POSITIVE_INTEGER ||
             obj_flags->type == MSGPACK_OBJECT_NEGATIVE_INTEGER)) {
            const int64_t new_flags = obj_flags->via.i64;
            if (new_flags >= INT_MIN && new_flags <= INT_MAX) {
                *flags = new_flags;
            }
        }
    } else {
        const msgpack_object * const obj_data =
            msgpack_get_map_value_for_key(map, "data");
        if (obj_data != NULL && obj_data->type == MSGPACK_OBJECT_RAW &&
            obj_data->via.raw.size <= (uint32_t) *nbyte && *ret > 0) {
            memcpy(buf, obj_data->via.raw.ptr, obj_data->via.raw.size);
            *ret = (int) obj_data->via.raw.size;
        }
    }    
    return reply_result;
}

static FilterReplyResult filter_apply(const bool pre, int * const ret,
                                      int * const ret_errno, const int fd,
                                      const struct sockaddr_storage * const sa_local,
                                      const socklen_t sa_local_len,
                                      const struct sockaddr_storage * const sa_remote,
                                      const socklen_t sa_remote_len,
                                      void *buf, size_t * const nbyte,
                                      int * const flags)
{
    Filter * const filter = filter_get();
    msgpack_packer * const msgpack_packer = filter->msgpack_packer;
    filter_before_apply(pre, *ret, *ret_errno, fd, 2U, "recv",
                        sa_local, sa_local_len, sa_remote, sa_remote_len);

    msgpack_pack_mstring(msgpack_packer, "flags");
    msgpack_pack_int(msgpack_packer, *flags);
    if (pre != false) {
        msgpack_pack_mstring(msgpack_packer, "nbyte");
        msgpack_pack_unsigned_long(msgpack_packer, *nbyte);
    } else if (*ret <= 0) {
        msgpack_pack_mstring(msgpack_packer, "data");
        msgpack_pack_nil(msgpack_packer);
    } else {
        assert((size_t) *ret <= *nbyte);
        msgpack_pack_mstring(msgpack_packer, "data");
        msgpack_pack_raw(msgpack_packer, *ret);
        msgpack_pack_raw_body(msgpack_packer, buf, *ret);
    }
    if (filter_send_message(filter) != 0) {
        return FILTER_REPLY_RESULT_ERROR;
    }
    return filter_parse_reply(pre, filter, ret, ret_errno, fd, buf, nbyte,
                              flags);
}

int __real_recv_init(void)
{
#ifdef USE_INTERPOSERS
    __real_recv = recv;
#else
    if (__real_recv == NULL) {
        __real_recv = dlsym(RTLD_NEXT, "recv");
        assert(__real_recv != NULL);
    }
#endif
    return 0;
}

ssize_t INTERPOSE(recv)(int fd, void *buf, size_t nbyte, int flags)
{
    __real_recv_init();
    const bool bypass_filter =
        getenv("SIXJACK_BYPASS") != NULL || is_socket(fd) == false;
    struct sockaddr_storage sa_local, *sa_local_ = &sa_local;
    struct sockaddr_storage sa_remote, *sa_remote_ = &sa_remote;
    socklen_t sa_local_len, sa_remote_len;
    get_sock_info(fd, &sa_local_, &sa_local_len, &sa_remote_, &sa_remote_len);
    int ret = 0;
    int ret_errno = 0;    
    bool bypass_call = false;
    size_t new_nbyte = nbyte;
    if (bypass_filter == false &&
        filter_apply(true, &ret, &ret_errno, fd, sa_local_, sa_local_len,
                     sa_remote_, sa_remote_len, NULL, &new_nbyte, &flags)
        == FILTER_REPLY_BYPASS) {
        bypass_call = true;
    }
    if (bypass_call == false) {
        ssize_t ret_ = __real_recv(fd, buf, new_nbyte, flags);
        ret_errno = errno;
        ret = (int) ret_;
        assert((ssize_t) ret_ == ret);
    }
    if (bypass_filter == false) {
        filter_apply(false, &ret, &ret_errno, fd, sa_local_, sa_local_len,
                     sa_remote_, sa_remote_len, buf, &new_nbyte, &flags);
    }
    errno = ret_errno;
    
    return ret;
}
