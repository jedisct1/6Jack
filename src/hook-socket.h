
#ifndef __HOOK_SOCKET_H__
#define __HOOK_SOCKET_H__ 1

HOOK_GLOBAL int (* __real_socket)(int domain, int type, int protocol);
DLL_PUBLIC  int INTERPOSE(socket)(int domain, int type, int protocol);

#endif
