
#ifndef __HOOK_CONNECT_READ_WRITE_H__
#define __HOOK_CONNECT_READ_WRITE_H__ 1

DLL_PUBLIC ssize_t INTERPOSE(read)(int fd, void *buf, size_t nbyte);
extern ssize_t (* __real_read)(int fd, void *buf, size_t nbyte);
int __real_read_init(void);

#ifndef DONT_BYPASS_HOOKS
# define read __real_read
#endif

#endif
