
#ifndef __HOOK_WRITEV_H__
#define __HOOK_WRITEV_H__ 1

#include <sys/uio.h>

DLL_PUBLIC ssize_t INTERPOSE(writev)(int fd, const struct iovec *iov, int iovcnt);
extern ssize_t (* __real_writev)(int fd, const struct iovec *iov, int iovcnt);
int __real_writev_init(void);

#ifndef DONT_BYPASS_HOOKS
# define writev __real_writev
#endif

#endif
