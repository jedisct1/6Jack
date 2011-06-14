
#ifndef __HOOK_CONNECT_BIND_H__
#define __HOOK_CONNECT_BIND_H__ 1

DLL_PUBLIC int INTERPOSE(connect)(int fd, const struct sockaddr *sa,
                                  socklen_t sa_len);
extern int (* __real_connect)(int fd, const struct sockaddr *sa,
                              socklen_t sa_len);
int __real_connect_init(void);

DLL_PUBLIC int INTERPOSE(bind)(int fd, const struct sockaddr *sa,
                               socklen_t sa_len);
extern int (* __real_bind)(int fd, const struct sockaddr *sa,
                           socklen_t sa_len);
int __real_bind_init(void);

#ifndef DONT_BYPASS_HOOKS
# define connect __real_connect
# define bind __real_bind
#endif

#endif
