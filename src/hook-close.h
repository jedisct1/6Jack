
#ifndef __HOOK_CLOSE_H__
#define __HOOK_CLOSE_H__ 1

DLL_PUBLIC int INTERPOSE(close)(int fd);
extern int (* __real_close)(int fd);
int __real_close_init(void);

#ifndef DONT_BYPASS_HOOKS
# define close __real_close
#endif

#endif
