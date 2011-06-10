
#ifndef __SIXJACK_H__
#define __SIXJACK_H__ 1

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

typedef struct Context_ {
    bool initialized;
    Filter filter;
} Context;

Context *get_sixjack_context(void);
void free_sixjack_context(void);
Filter *get_filter(void);

#endif
