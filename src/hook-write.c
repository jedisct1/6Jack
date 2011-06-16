
#define DEFINE_HOOK_GLOBALS 1
#define DONT_BYPASS_HOOKS   1

#include "common.h"
#include "filter.h"
#include "hook-write.h"

ssize_t (* __real_write)(int fd, const void *buf, size_t nbyte);

static FilterReplyResult filter_parse_reply(const bool pre,
                                            Filter * const filter,
                                            int * const ret,
                                            int * const ret_errno,
                                            const int fd,
                                            const void ** const buf,
                                            size_t * const nbyte)
{
    msgpack_unpacked * const message = filter_receive_message(filter);
    const msgpack_object_map * const map = &message->data.via.map;
    FilterReplyResult reply_result =
        filter_parse_common_reply_map(map, ret, ret_errno, fd);

    if (pre == false) {
        return reply_result;
    }
    const msgpack_object * const obj_data =
        msgpack_get_map_value_for_key(map, "data");
    if (obj_data->type == MSGPACK_OBJECT_RAW) {
        *buf = obj_data->via.raw.ptr;
        *nbyte = (size_t) obj_data->via.raw.size;
    }    
    return reply_result;
}

static FilterReplyResult filter_apply(const bool pre, int * const ret,
                                      int * const ret_errno, const int fd,
                                      const void ** const buf,
                                      size_t * const nbyte)
{
    Filter * const filter = filter_get();
    msgpack_packer * const msgpack_packer = filter->msgpack_packer;

    filter_before_apply(pre, *ret, *ret_errno, fd, 1U, "write",
                        NULL, (socklen_t) 0U, NULL, (socklen_t) 0U);
    msgpack_pack_mstring(msgpack_packer, "data");
    msgpack_pack_raw(msgpack_packer, *nbyte);
    msgpack_pack_raw_body(msgpack_packer, *buf, *nbyte);
    if (filter_send_message(filter) != 0) {
        return -1;
    }
    return filter_parse_reply(pre, filter, ret, ret_errno, fd, buf, nbyte);
}

int __real_write_init(void)
{
#ifdef USE_INTERPOSERS
    __real_write = write;
#else
    if (__real_write == NULL) {
        __real_write = dlsym(RTLD_NEXT, "write");
        assert(__real_write != NULL);
    }
#endif
    return 0;
}

ssize_t INTERPOSE(write)(int fd, const void *buf, size_t nbyte)
{
    __real_write_init();
    const bool bypass_filter =
        getenv("SIXJACK_BYPASS") != NULL || is_socket(fd) == false;
    int ret = 0;
    int ret_errno = 0;    
    bool bypass_call = false;
    size_t new_nbyte = nbyte;
    if (bypass_filter == false &&
        filter_apply(true, &ret, &ret_errno, fd, &buf, &new_nbyte)
        == FILTER_REPLY_BYPASS) {
        bypass_call = true;
    }
    if (bypass_call == false) {
        ssize_t ret_ = __real_write(fd, buf, new_nbyte);
        ret_errno = errno;        
        ret = (int) ret_;
        assert((ssize_t) ret_ == ret);
    }
    if (bypass_filter == false) {
        filter_apply(false, &ret, &ret_errno, fd, &buf, &new_nbyte);
    }
    errno = ret_errno;
    
    return ret;
}
