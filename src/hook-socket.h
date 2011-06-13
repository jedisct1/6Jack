
#ifndef __HOOK_SOCKET_H__
#define __HOOK_SOCKET_H__ 1

DLL_PUBLIC int INTERPOSE(socket)(int domain, int type, int protocol);
extern int (* __real_socket)(int domain, int type, int protocol);
int __real_socket_init(void);

#ifndef DONT_BYPASS_HOOKS
# define socket __real_socket
#endif

#endif
