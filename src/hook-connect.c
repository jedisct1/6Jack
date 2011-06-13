
#define DEFINE_HOOK_GLOBALS 1
#define DONT_BYPASS_HOOKS   1

#include "common.h"
#include "filter.h"
#include "hook-connect.h"

int (* __real_connect)(int fd, const struct sockaddr *sa, socklen_t sa_len);

static int filter_parse_reply(Filter * const filter, int * const ret,
                              int * const ret_errno, const int fd)
{
    msgpack_unpacked * const message = filter_receive_message(filter);
    const msgpack_object_map * const map = &message->data.via.map;
    filter_parse_common_reply_map(map, ret, ret_errno, fd);
    
    return 0;
}

static int filter_apply(int * const ret, int * const ret_errno,
                        const int fd, const struct sockaddr * const sa,
                        const socklen_t sa_len)
{
    (void) sa;
    (void) sa_len;
    Filter * const filter = filter_get();
    filter_before_apply(*ret, *ret_errno, fd, 0U, "connect", true);

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
    int ret = __real_connect(fd, sa, sa_len);
    int ret_errno = errno;
    if (bypass_filter == false) {
        filter_apply(&ret, &ret_errno, fd, sa, sa_len);
    }
    errno = ret_errno;
    
    return ret;
}
