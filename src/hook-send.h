
#ifndef __HOOK_SEND_H__
#define __HOOK_SEND_H__ 1

DLL_PUBLIC ssize_t INTERPOSE(send)(int fd, const void *buf, size_t nbyte,
                                   int flags);
extern ssize_t (* __real_send)(int fd, const void *buf, size_t nbyte,
                               int flags);
int __real_send_init(void);

#ifndef DONT_BYPASS_HOOKS
# define send __real_send
#endif

#endif
