
#ifndef __HOOK_WRITE_H__
#define __HOOK_WRITE_H__ 1

DLL_PUBLIC ssize_t INTERPOSE(write)(int fd, const void *buf, size_t nbyte);
extern ssize_t (* __real_write)(int fd, const void *buf, size_t nbyte);
int __real_write_init(void);

#ifndef DONT_BYPASS_HOOKS
# define write __real_write
#endif

#endif
