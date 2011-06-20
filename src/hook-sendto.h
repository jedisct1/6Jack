
#ifndef __HOOK_CONNECT_SENDTO__H__
#define __HOOK_CONNECT_SENDTO__H__ 1

DLL_PUBLIC ssize_t INTERPOSE(sendto)(int fd, const void *buf, size_t nbyte,
                                     int flags,
                                     const struct sockaddr *sa_remote,
                                     socklen_t sa_remote_len);
extern ssize_t (* __real_sendto)(int fd, const void *buf, size_t nbyte,
                                 int flags,
                                 const struct sockaddr *sa_remote,
                                 socklen_t sa_remote_len);
int __real_sendto_init(void);

#ifndef DONT_BYPASS_HOOKS
# define sendto __real_sendto
#endif

#endif
