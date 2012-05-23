
#define DEFINE_HOOK_GLOBALS 1
#define DONT_BYPASS_HOOKS   1

#include "common.h"
#include "filter.h"
#include "hook-writev.h"

ssize_t (* __real_writev)(int fd, const struct iovec *iov, int iovcnt);

static FilterReplyResult filter_parse_reply(FilterReplyResultBase * const rb,
                                            const struct iovec * * const iov,
                                            int * const iovcnt)
{
    msgpack_unpacked * const message = filter_receive_message(rb->filter);
    const msgpack_object_map * const map = &message->data.via.map;
    FilterReplyResult reply_result = filter_parse_common_reply_map(rb, map);

    if (rb->pre == false) {
        return reply_result;
    }

    const msgpack_object * const obj_data =
        msgpack_get_map_value_for_key(map, "data");

    if (obj_data != NULL && obj_data->type == MSGPACK_OBJECT_ARRAY &&
        *rb->ret > 0) {
        int i, ret = 0;

        /* This leak as we don't know how the original application is keeping track of this */
        struct iovec *niovev = malloc(sizeof(struct iovec) * obj_data->via.array.size);
        
        *iovcnt = obj_data->via.array.size;
               
        for (i = 0; i < obj_data->via.array.size; i++) {
            msgpack_object obj_data_part = obj_data->via.array.ptr[i];
            
            assert(obj_data_part.type == MSGPACK_OBJECT_RAW);
            
            niovev[i].iov_len = (size_t) obj_data_part.via.raw.size;
            niovev[i].iov_base = obj_data_part.via.raw.ptr;
            
            ret += obj_data_part.via.raw.size;
        }
        
        *iov = niovev;
        *rb->ret = (int) ret;
    }    
    
    return reply_result;
}

static FilterReplyResult filter_apply(FilterReplyResultBase * const rb,
                                      const struct sockaddr_storage * const sa_local,
                                      const socklen_t sa_local_len,
                                      const struct sockaddr_storage * const sa_remote,
                                      const socklen_t sa_remote_len,
                                      const struct iovec * * const iov,
                                      int * const iovcnt)
{
    int i;
    msgpack_packer * const msgpack_packer = rb->filter->msgpack_packer;
    filter_before_apply(rb, 1U, "writev",
                        sa_local, sa_local_len, sa_remote, sa_remote_len);
    
    msgpack_pack_mstring(msgpack_packer, "data");
    msgpack_pack_array(msgpack_packer, *iovcnt);
    
    for (i = 0; i < *iovcnt; i++) {
        msgpack_pack_raw(msgpack_packer, (*(iov))[i].iov_len);
        msgpack_pack_raw_body(msgpack_packer, (*(iov))[i].iov_base,
                             (*(iov))[i].iov_len);
    }
    
    if (filter_send_message(rb->filter) != 0) {
        return FILTER_REPLY_RESULT_ERROR;
    }
    return filter_parse_reply(rb, iov, iovcnt);
}

int __real_writev_init(void)
{
#ifdef USE_INTERPOSERS
    __real_writev = writev;
#else
    if (__real_writev == NULL) {
        __real_writev = dlsym(RTLD_NEXT, "writev");
        assert(__real_writev != NULL);
    }
#endif
    return 0;
}

ssize_t INTERPOSE(writev)(int fd, const struct iovec *iov, int iovcnt)
{
    __real_writev_init();
    const bool bypass_filter =
        getenv("SIXJACK_BYPASS") != NULL || is_socket(fd) == false;
    struct sockaddr_storage sa_local, *sa_local_ = &sa_local;
    struct sockaddr_storage sa_remote, *sa_remote_ = &sa_remote;
    socklen_t sa_local_len, sa_remote_len;
    get_sock_info(fd, &sa_local_, &sa_local_len, &sa_remote_, &sa_remote_len);
    int ret = 0;
    int ret_errno = 0;    
    bool bypass_call = false;
    FilterReplyResultBase rb = {
        .pre = true, .ret = &ret, .ret_errno = &ret_errno, .fd = fd
    };
    if (bypass_filter == false && (rb.filter = filter_get()) &&
        filter_apply(&rb, sa_local_, sa_local_len,
                     sa_remote_, sa_remote_len, &iov, &iovcnt)
        == FILTER_REPLY_BYPASS) {
        bypass_call = true;
    }
    if (bypass_call == false) {
        ssize_t ret_ = __real_writev(fd, iov, iovcnt);
        ret_errno = errno;        
        ret = (int) ret_;
        assert((ssize_t) ret_ == ret);
    }
    if (bypass_filter == false) {
        rb.pre = false;
        filter_apply(&rb, sa_local_, sa_local_len,
                     sa_remote_, sa_remote_len, &iov, &iovcnt);
    }
    errno = ret_errno;
    
    return ret;
}
