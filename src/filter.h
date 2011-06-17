
#ifndef __FILTER_H__
#define __FILTER_H__ 1

#include "utils.h"
#include "msgpack-extensions.h"
#include "id-name.h"

#define FILTER_READ_BUFFER_SIZE   MSGPACK_UNPACKER_INIT_BUFFER_SIZE
#define FILTER_UNPACK_BUFFER_SIZE FILTER_READ_BUFFER_SIZE

typedef struct Filter_ {
    Upipe upipe_stdin;
    Upipe upipe_stdout;
    msgpack_sbuffer *msgpack_sbuffer;
    msgpack_packer *msgpack_packer;
    msgpack_unpacker msgpack_unpacker;
    msgpack_unpacked message;
    pid_t pid;
} Filter;

typedef enum FilterReplyResult_ {
    FILTER_REPLY_RESULT_ERROR = -1,
    FILTER_REPLY_PASS = 0,
    FILTER_REPLY_BYPASS = 1
} FilterReplyResult;

Filter *filter_get(void);

int filter_before_apply(const bool pre,
                        const int ret, const int ret_errno, const int fd,
                        const unsigned int nongeneric_items,
                        const char * const function,
                        const struct sockaddr_storage * const sa_local,
                        const socklen_t sa_local_len,
                        const struct sockaddr_storage * const sa_remote,
                        const socklen_t sa_remote_len);

int filter_send_message(Filter * const filter);

msgpack_unpacked *filter_receive_message(Filter * const filter);

FilterReplyResult filter_parse_common_reply_map(const msgpack_object_map * const map,
                                                int * const ret,
                                                int * const ret_errno,
                                                const int fd);

int filter_overwrite_sa_with_reply_map(const msgpack_object_map * const map,
                                       const char * const key_host,
                                       const char * const key_port,
                                       struct sockaddr_storage * const sa,
                                       socklen_t * const sa_len);
#endif
