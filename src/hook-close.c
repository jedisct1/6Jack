
#define DEFINE_HOOK_GLOBALS 1
#define DONT_BYPASS_HOOKS   1

#include "common.h"
#include "filter.h"
#include "hook-close.h"

int (* __real_close)(int fd);

static FilterReplyResult filter_parse_reply(FilterReplyResultBase * const rb)
{
    msgpack_unpacked * const message = filter_receive_message(rb->filter);
    const msgpack_object_map * const map = &message->data.via.map;
    FilterReplyResult reply_result = filter_parse_common_reply_map(rb, map);
    
    return reply_result;
}

static FilterReplyResult filter_apply(FilterReplyResultBase * const rb,
                                      const struct sockaddr_storage * const sa_local,
                                      const socklen_t sa_local_len,
                                      const struct sockaddr_storage * const sa_remote,
                                      const socklen_t sa_remote_len)
{
    filter_before_apply(rb, 0U, "close",
                        sa_local, sa_local_len, sa_remote, sa_remote_len);    
    if (filter_send_message(rb->filter) != 0) {
        return FILTER_REPLY_RESULT_ERROR;
    }
    return filter_parse_reply(rb);
}

int __real_close_init(void)
{
#ifdef USE_INTERPOSERS
    __real_close = close;
#else
    if (__real_close == NULL) {
        __real_close = dlsym(RTLD_NEXT, "close");
        assert(__real_close != NULL);
    }
#endif
    return 0;
}

int INTERPOSE(close)(int fd)
{
    __real_close_init();
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
        .pre = true,
        .filter = filter_get(), .ret = &ret, .ret_errno = &ret_errno, .fd = fd,
    };    
    if (bypass_filter == false &&
        filter_apply(&rb, sa_local_, sa_local_len, sa_remote_, sa_remote_len)
        == FILTER_REPLY_BYPASS) {
        bypass_call = true;
    }
    if (bypass_call == false) {
        ret = __real_close(fd);
        ret_errno = errno;
    }
    if (bypass_filter == false) {
        rb.pre = false;
        filter_apply(&rb, sa_local_, sa_local_len, sa_remote_, sa_remote_len);
    }
    errno = ret_errno;
    
    return ret;
}
