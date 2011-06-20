
#define DEFINE_HOOK_GLOBALS 1
#define DONT_BYPASS_HOOKS   1

#include "common.h"
#include "filter.h"
#include "hook-sendto.h"

ssize_t (* __real_sendto)(int fd, const void *buf, size_t nbyte, int flags,
                          const struct sockaddr *sa_remote,
                          socklen_t sa_remote_len);

static FilterReplyResult filter_parse_reply(FilterReplyResultBase * const rb,
                                            const void * * const buf,
                                            size_t * const nbyte,
                                            int * const flags,
                                            struct sockaddr_storage * const sa,
                                            socklen_t * const sa_len)
{
    msgpack_unpacked * const message = filter_receive_message(rb->filter);
    const msgpack_object_map * const map = &message->data.via.map;
    FilterReplyResult reply_result = filter_parse_common_reply_map(rb, map);

    if (rb->pre == false) {
        return reply_result;
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
    const msgpack_object * const obj_data =
        msgpack_get_map_value_for_key(map, "data");
    if (obj_data != NULL && obj_data->type == MSGPACK_OBJECT_RAW &&
        *rb->ret > 0) {
        *buf = obj_data->via.raw.ptr;
        *nbyte = (size_t) obj_data->via.raw.size;        
        *rb->ret = (int) *nbyte;
        assert((size_t) *rb->ret == *nbyte);
    }
    filter_overwrite_sa_with_reply_map(map, "remote_host", "remote_port",
                                       sa, sa_len);
    return reply_result;
}

static FilterReplyResult filter_apply(FilterReplyResultBase * const rb,
                                      const struct sockaddr_storage * const sa_local,
                                      const socklen_t sa_local_len,
                                      struct sockaddr_storage * const sa_remote,
                                      socklen_t * const sa_remote_len,
                                      const void * * const buf,
                                      size_t * const nbyte, int * const flags)
{
    msgpack_packer * const msgpack_packer = rb->filter->msgpack_packer;
    filter_before_apply(rb, 2U, "sendto",
                        sa_local, sa_local_len, sa_remote, *sa_remote_len);
    msgpack_pack_mstring(msgpack_packer, "flags");
    msgpack_pack_int(msgpack_packer, *flags);    
    msgpack_pack_mstring(msgpack_packer, "data");
    msgpack_pack_raw(msgpack_packer, *nbyte);
    msgpack_pack_raw_body(msgpack_packer, *buf, *nbyte);
    if (filter_send_message(rb->filter) != 0) {
        return FILTER_REPLY_RESULT_ERROR;
    }
    return filter_parse_reply(rb, buf, nbyte, flags, sa_remote, sa_remote_len);
}

int __real_sendto_init(void)
{
#ifdef USE_INTERPOSERS
    __real_sendto = sendto;
#else
    if (__real_sendto == NULL) {
        __real_sendto = dlsym(RTLD_NEXT, "sendto");
        assert(__real_sendto != NULL);
    }
#endif
    return 0;
}

ssize_t INTERPOSE(sendto)(int fd, const void *buf, size_t nbyte, int flags,
                          const struct sockaddr *sa_remote,
                          socklen_t sa_remote_len)
{
    __real_sendto_init();
    const bool bypass_filter =
        getenv("SIXJACK_BYPASS") != NULL || is_socket(fd) == false;
    struct sockaddr_storage sa_local, *sa_local_ = &sa_local;
    socklen_t sa_local_len;
    get_sock_info(fd, &sa_local_, &sa_local_len, NULL, (socklen_t) 0U);
    int ret = 0;
    int ret_errno = 0;    
    bool bypass_call = false;
    struct sockaddr_storage sa_remote_;
    socklen_t sa_remote_len_ = sa_remote_len;
    assert(sa_remote_len <= sizeof sa_remote_len_);
    memcpy(&sa_remote_, sa_remote, sa_remote_len);
    size_t new_nbyte = nbyte;
    FilterReplyResultBase rb = {
        .pre = true, .ret = &ret, .ret_errno = &ret_errno, .fd = fd
    };
    if (bypass_filter == false && (rb.filter = filter_get()) &&
        filter_apply(&rb, sa_local_, sa_local_len,
                     &sa_remote_, &sa_remote_len_, &buf, &new_nbyte, &flags)
        == FILTER_REPLY_BYPASS) {
        bypass_call = true;
    }
    if (bypass_call == false) {
        ssize_t ret_ = __real_sendto(fd, buf, new_nbyte, flags,
                                     (const struct sockaddr *) &sa_remote_,
                                     sa_remote_len_);
        ret_errno = errno;
        ret = (int) ret_;
        assert((ssize_t) ret_ == ret);
    }
    if (bypass_filter == false) {
        rb.pre = false;
        filter_apply(&rb, sa_local_, sa_local_len,
                     &sa_remote_, &sa_remote_len_, NULL, &new_nbyte, NULL);
    }
    errno = ret_errno;
    
    return ret;
}
