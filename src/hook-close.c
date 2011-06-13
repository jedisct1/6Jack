
#define DEFINE_HOOK_GLOBALS 1
#define DONT_BYPASS_HOOKS   1

#include "common.h"
#include "filter.h"
#include "hook-close.h"

int (* __real_close)(int fd);

static int filter_parse_reply(Filter * const filter, int * const ret,
                              int * const ret_errno, const int fd)
{
    msgpack_unpacked * const message = filter_receive_message(filter);
    const msgpack_object_map * const map = &message->data.via.map;
    filter_parse_common_reply_map(map, ret, ret_errno, fd);
    
    return 0;
}

static int filter_apply(int * const ret, int * const ret_errno,
                        const int fd)
{
    Filter * const filter = filter_get();
    filter_before_apply(*ret, *ret_errno, fd, 0U, "close", true);
    
    if (filter_send_message(filter) != 0) {
        return -1;
    }    
    return filter_parse_reply(filter, ret, ret_errno, fd);
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
        getenv("SIXJACK_BYPASS") != NULL || is_socket(fd);
    int ret = __real_close(fd);
    int ret_errno = errno;
    if (bypass_filter == false) {
        filter_apply(&ret, &ret_errno, fd);
    }
    errno = ret_errno;
    
    return ret;
}
