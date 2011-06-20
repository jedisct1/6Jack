
#ifndef __HOOK_CONNECT_SENDMSG__H__
#define __HOOK_CONNECT_SENDMSG__H__ 1

DLL_PUBLIC ssize_t INTERPOSE(sendmsg)(int fd, const struct msghdr *msg,
                                      int flags);
extern ssize_t (* __real_sendmsg)(int fd, const struct msghdr *msg, int flags);
int __real_sendmsg_init(void);

#ifndef DONT_BYPASS_HOOKS
# define sendmsg __real_sendmsg
#endif

#endif
