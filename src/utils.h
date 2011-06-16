
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

bool is_socket(const int fd);

int get_name_info(const struct sockaddr_storage * const sa,
                  const socklen_t sa_len,
                  char host[NI_MAXHOST], in_port_t * const port);

int get_sock_info(const int fd,
                  struct sockaddr_storage * * const sa_local,
                  socklen_t * const sa_local_len,
                  struct sockaddr_storage * * const sa_remote,
                  socklen_t * const sa_remote_len);

#endif
