
#ifndef __HOOK_CONNECT_RECVFROM__H__
#define __HOOK_CONNECT_RECVFROM__H__ 1

DLL_PUBLIC ssize_t INTERPOSE(recvfrom)(int fd, void *buf, size_t nbyte,
                                       int flags, struct sockaddr *sa_remote,
                                       socklen_t *sa_remote_len);
extern ssize_t (* __real_recvfrom)(int fd, void *buf, size_t nbyte,
                                   int flags, struct sockaddr *sa_remote,
                                   socklen_t *sa_remote_len);
int __real_recvfrom_init(void);

#ifndef DONT_BYPASS_HOOKS
# define recvfrom __real_recvfrom
#endif

#endif
