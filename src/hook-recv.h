
#ifndef __HOOK_CONNECT_RECV__H__
#define __HOOK_CONNECT_RECV__H__ 1

DLL_PUBLIC ssize_t INTERPOSE(recv)(int fd, void *buf, size_t nbyte, int flags);
extern ssize_t (* __real_recv)(int fd, void *buf, size_t nbyte, int flags);
int __real_recv_init(void);

#ifndef DONT_BYPASS_HOOKS
# define recv __real_recv
#endif

#endif
