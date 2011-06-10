
#ifndef __UTILS_H__
#define __UTILS_H__ 1

typedef struct Upipe_ {
    int fd_read;
    int fd_write;
} Upipe;

int upipe_init(Upipe * const upipe);
void upipe_free(Upipe * const upipe);

int safe_write(const int fd, const void * const buf_, size_t count,
               const int timeout);

ssize_t safe_read(const int fd, void * const buf_, size_t count);

ssize_t safe_read_partial(const int fd, void * const buf_,
                          const size_t max_count);

#endif
