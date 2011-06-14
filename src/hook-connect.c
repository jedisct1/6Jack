
#define DEFINE_HOOK_GLOBALS 1
#define DONT_BYPASS_HOOKS   1

#include "common.h"
#include "filter.h"
#include "hook-connect.h"

int (* __real_connect)(int fd, const struct sockaddr *sa, socklen_t sa_len);

static FilterReplyResult filter_parse_reply(Filter * const filter,
                                            int * const ret,
                                            int * const ret_errno,
                                            const int fd)
{
    msgpack_unpacked * const message = filter_receive_message(filter);
    const msgpack_object_map * const map = &message->data.via.map;
    FilterReplyResult reply_result =
        filter_parse_common_reply_map(map, ret, ret_errno, fd);
    
    return reply_result;
}

static FilterReplyResult filter_apply(const bool pre, int * const ret,
                                      int * const ret_errno, const int fd,
                                      struct sockaddr_storage * const sa,
                                      socklen_t * const sa_len)
{
    Filter * const filter = filter_get();
    filter_before_apply(pre, *ret, *ret_errno, fd, 0U, "connect",
                        NULL, (socklen_t) 0U, sa, *sa_len);

    if (filter_send_message(filter) != 0) {
        return -1;
    }    
    return filter_parse_reply(filter, ret, ret_errno, fd);
}

int __real_connect_init(void)
{
#ifdef USE_INTERPOSERS
    __real_connect = connect;
#else
    if (__real_connect == NULL) {
        __real_connect = dlsym(RTLD_NEXT, "connect");        
        assert(__real_connect != NULL);        
    }
#endif
    return 0;
}

int INTERPOSE(connect)(int fd, const struct sockaddr *sa, socklen_t sa_len)
{
    __real_connect_init();
    const bool bypass_filter = getenv("SIXJACK_BYPASS") != NULL;
    int ret = 0;
    int ret_errno = 0;    
    bool bypass_call = false;
    struct sockaddr_storage sa_;
    socklen_t sa_len_ = sa_len;
    assert(sa_len <= sizeof sa_);
    memcpy(&sa_, sa, sa_len);
    if (bypass_filter == false &&
        filter_apply(true, &ret, &ret_errno, fd, &sa_, &sa_len_)
        == FILTER_REPLY_BYPASS) {
        bypass_call = true;
    }
    if (bypass_call == false) {
        ret = __real_connect(fd, (struct sockaddr *) &sa_, sa_len_);
        ret_errno = errno;
    }
    if (bypass_filter == false) {
        filter_apply(false, &ret, &ret_errno, fd, &sa_, &sa_len_);
    }
    errno = ret_errno;
    
    return ret;
}
