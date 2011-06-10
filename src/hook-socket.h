
#ifndef __HOOK_SOCKET_H__
#define __HOOK_SOCKET_H__ 1

DLL_PUBLIC  int INTERPOSE(socket)(int domain, int type, int protocol);
HOOK_GLOBAL int (* __real_socket)(int domain, int type, int protocol);
#ifndef DONT_BYPASS_HOOKS
# define socket __real_socket
#endif

#endif
