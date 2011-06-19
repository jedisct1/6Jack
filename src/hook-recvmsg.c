
#define DEFINE_HOOK_GLOBALS 1
#define DONT_BYPASS_HOOKS   1

#include "common.h"
#include "filter.h"
#include "hook-recvmsg.h"

ssize_t (* __real_recvmsg)(int fd, struct msghdr *msg, int flags);

static FilterReplyResult filter_parse_reply(const bool pre,
                                            Filter * const filter,
                                            int * const ret,
                                            int * const ret_errno,
                                            const int fd,
                                            struct msghdr * const msg,
                                            int * const flags)
{
    msgpack_unpacked * const message = filter_receive_message(filter);
    const msgpack_object_map * const map = &message->data.via.map;
    FilterReplyResult reply_result =
        filter_parse_common_reply_map(map, ret, ret_errno, fd);

    if (pre != false) {
        const msgpack_object * const obj_flags =
            msgpack_get_map_value_for_key(map, "flags");
        if (obj_flags != NULL &&
            (obj_flags->type == MSGPACK_OBJECT_POSITIVE_INTEGER ||
             obj_flags->type == MSGPACK_OBJECT_NEGATIVE_INTEGER)) {
            const int64_t new_flags = obj_flags->via.i64;
            if (new_flags >= INT_MIN && new_flags <= INT_MAX) {
                *flags = new_flags;
                (void) flags;
            }
        }
    } else {
        const msgpack_object * const obj_data =
            msgpack_get_map_value_for_key(map, "data");
        if (obj_data != NULL && obj_data->type == MSGPACK_OBJECT_RAW) {
            struct iovec * const vecs = msg->msg_iov;            
            const char *data_pnt = obj_data->via.raw.ptr;
            size_t data_remaining = (size_t) obj_data->via.raw.size;
            size_t copy_to_vec;
            int i_vecs = 0;
            size_t max_nbytes = (size_t) 0U;
            while (i_vecs < msg->msg_iovlen) {
                max_nbytes += vecs[i_vecs].iov_len;
                i_vecs++;
            }
            assert(max_nbytes >= data_remaining);
            i_vecs = 0;
            while (i_vecs < msg->msg_iovlen && data_remaining > (size_t) 0U) {
                if (data_remaining < vecs[i_vecs].iov_len) {
                    copy_to_vec = data_remaining;
                } else {
                    copy_to_vec = vecs[i_vecs].iov_len;
                }
                assert(data_remaining >= copy_to_vec);
                assert(vecs[i_vecs].iov_len >= copy_to_vec);
                memcpy(vecs[i_vecs].iov_base, data_pnt, copy_to_vec);
                data_remaining -= copy_to_vec;
                i_vecs++;
            }
            *ret = (int) obj_data->via.raw.size;
            assert((uint32_t) *ret == obj_data->via.raw.size);
        }
        filter_overwrite_sa_with_reply_map(map, "remote_host", "remote_port",
                                           msg->msg_name, &msg->msg_namelen);
    }    
    return reply_result;
}

static FilterReplyResult filter_apply(const bool pre, int * const ret,
                                      int * const ret_errno, const int fd,
                                      const struct sockaddr_storage * const sa_local,
                                      const socklen_t sa_local_len,
                                      struct msghdr *msg, size_t * const nbyte,
                                      int * const flags)
{
    Filter * const filter = filter_get();
    msgpack_packer * const msgpack_packer = filter->msgpack_packer;

    if (pre != false) {
        filter_before_apply(pre, *ret, *ret_errno, fd, 2U, "recvmsg",
                            sa_local, sa_local_len, NULL, (socklen_t) 0U);
    } else {
        filter_before_apply(pre, *ret, *ret_errno, fd, 2U, "recvmsg",
                            sa_local, sa_local_len,
                            msg->msg_name, msg->msg_namelen);
    }
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
        msgpack_pack_raw(msgpack_packer, *nbyte);
        size_t data_remaining = *nbyte;
        size_t read_from_vec;
        struct iovec * const vecs = msg->msg_iov;
        int i_vecs = 0;
        while (i_vecs < msg->msg_iovlen && data_remaining > (size_t) 0U) {
            if (data_remaining < vecs[i_vecs].iov_len) {
                read_from_vec = data_remaining;
            } else {
                read_from_vec = vecs[i_vecs].iov_len;
            }
            assert(data_remaining >= read_from_vec);
            assert(vecs[i_vecs].iov_len >= read_from_vec);
            msgpack_pack_raw_body(msgpack_packer, vecs[i_vecs].iov_base,
                                  read_from_vec);
            data_remaining -= read_from_vec;
            i_vecs++;
        }
    }
    if (filter_send_message(filter) != 0) {
        return FILTER_REPLY_RESULT_ERROR;
    }    
    return filter_parse_reply(pre, filter, ret, ret_errno, fd, msg, flags);
}

int __real_recvmsg_init(void)
{
#ifdef USE_INTERPOSERS
    __real_recvmsg = recvmsg;
#else
    if (__real_recvmsg == NULL) {
        __real_recvmsg = dlsym(RTLD_NEXT, "recvmsg");
        assert(__real_recvmsg != NULL);
    }
#endif
    return 0;
}

ssize_t INTERPOSE(recvmsg)(int fd, struct msghdr *msg, int flags)
{
    __real_recvmsg_init();
    const bool bypass_filter =
        getenv("SIXJACK_BYPASS") != NULL || is_socket(fd) == false;
        
    struct sockaddr_storage sa_local, *sa_local_ = &sa_local;
    socklen_t sa_local_len;
    get_sock_info(fd, &sa_local_, &sa_local_len, NULL, (socklen_t) 0U);
    int ret = 0;
    int ret_errno = 0;    
    bool bypass_call = false;
    size_t nbyte = (size_t) 0U;
    struct iovec * const vecs = msg->msg_iov;
    int i_vecs = 0;
    while (i_vecs < msg->msg_iovlen) {
        assert(SIZE_T_MAX - nbyte >= vecs[i_vecs].iov_len);
        nbyte += vecs[i_vecs].iov_len;
        i_vecs++;
    }
    size_t new_nbyte = nbyte;
    if (bypass_filter == false &&
        filter_apply(true, &ret, &ret_errno, fd, sa_local_, sa_local_len, msg,
                     &new_nbyte, &flags)
        == FILTER_REPLY_BYPASS) {
        bypass_call = true;
    }
    if (bypass_call == false) {
        ssize_t ret_ = __real_recvmsg(fd, msg, flags);
        ret_errno = errno;
        ret = (int) ret_;
        assert((ssize_t) ret_ == ret);
    }
    if (bypass_filter == false) {
        new_nbyte = ret;
        filter_apply(false, &ret, &ret_errno, fd, sa_local_, sa_local_len, msg,
                     &new_nbyte, &flags);
    }
    errno = ret_errno;
    
    return ret;
}
