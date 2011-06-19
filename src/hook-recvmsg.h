
#ifndef __HOOK_CONNECT_RECVMSG__H__
#define __HOOK_CONNECT_RECVMSG__H__ 1

DLL_PUBLIC ssize_t INTERPOSE(recvmsg)(int fd, struct msghdr *msg, int flags);
extern ssize_t (* __real_recvmsg)(int fd, struct msghdr *msg, int flags);
int __real_recvmsg_init(void);

#ifndef DONT_BYPASS_HOOKS
# define recvmsg __real_recvmsg
#endif

#endif
