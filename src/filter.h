
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

Filter *filter_get(void);

int filter_before_apply(const int ret, const int ret_errno, const int fd,
                        const unsigned int nongeneric_items,
                        const char * const function,
                        const bool include_net_info);

int filter_send_message(Filter * const filter);

msgpack_unpacked *filter_receive_message(Filter * const filter);

int filter_parse_common_reply_map(const msgpack_object_map * const map,
                                  int * const ret, int * const ret_errno,
                                  const int fd);
#endif
