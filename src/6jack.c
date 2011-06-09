#include <stdio.h>
#include <dlfcn.h>
#include <msgpack.h>
#include <stdlib.h>
#include <unistd.h>
#include <spawn.h>
#include <assert.h>
#include <poll.h>
#include <errno.h>
#include <sys/wait.h>
#include <limits.h>
#include <stdint.h>
#include <stdbool.h>

#define VERSION_MAJOR 1
#define FILTER_READ_BUFFER_SIZE   MSGPACK_UNPACKER_INIT_BUFFER_SIZE
#define FILTER_UNPACK_BUFFER_SIZE FILTER_READ_BUFFER_SIZE

typedef struct Upipe_ {
    int fd_read;
    int fd_write;
} Upipe;

typedef struct Filter_ {
    Upipe upipe_stdin;
    Upipe upipe_stdout;
    msgpack_sbuffer *msgpack_sbuffer;
    msgpack_packer *msgpack_packer;
    msgpack_unpacker *msgpack_unpacker;
    pid_t pid;
} Filter;

typedef struct Context_ {
    bool initialized;
    Filter filter;
} Context;

typedef struct IdName_ {
    int id;
    const char *name;
} IdName;

IdName pf_domains[] = {
    { PF_LOCAL,  "PF_LOCAL" },
    { PF_UNIX,   "PF_UNIX" },
    { PF_INET,   "PF_INET" },
    { PF_ROUTE,  "PF_ROUTE" },
    { PF_KEY,    "PF_KEY" },
    { PF_INET6,  "PF_INET6" },
    { PF_SYSTEM, "PF_SYSTEM" },    
    { PF_NDRV,   "PF_NDRV" },
    { -1,        NULL }
};

IdName sock_types[] = {
    { SOCK_STREAM,    "SOCK_STREAM" },
    { SOCK_DGRAM,     "SOCK_DGRAM" },
    { SOCK_RAW,       "SOCK_RAW" },
    { SOCK_SEQPACKET, "SOCK_SEQPACKET" },
    { SOCK_RDM,       "SOCK_RDM" },
    { -1,             NULL }
};

IdName ip_protos[] = {
    { IPPROTO_IP,      "IPPROTO_IP" },
    { IPPROTO_ICMP,    "IPPROTO_ICMP" },
    { IPPROTO_IGMP,    "IPPROTO_IGMP" },
    { IPPROTO_IPV4,    "IPPROTO_IPV4" },
    { IPPROTO_TCP,     "IPPROTO_TCP" },
    { IPPROTO_UDP,     "IPPROTO_UDP" },
    { IPPROTO_IPV6,    "IPPROTO_IPV6" },
    { IPPROTO_ROUTING, "IPPROTO_ROUTING" },
    { IPPROTO_FRAGMENT,"IPPROTO_FRAGMENT" },
    { IPPROTO_GRE,     "IPPROTO_GRE" },
    { IPPROTO_ESP,     "IPPROTO_ESP" },
    { IPPROTO_AH,      "IPPROTO_AH" },
    { IPPROTO_ICMPV6,  "IPPROTO_ICMPV6" },
    { IPPROTO_NONE,    "IPPROTO_NONE" },
    { IPPROTO_DSTOPTS, "IPPROTO_DSTOPTS" },
    { IPPROTO_IPCOMP,  "IPPROTO_IPCOMP" },
    { IPPROTO_PIM,     "IPPROTO_PIM" },
    { IPPROTO_PGM,     "IPPROTO_PGM" },
    { -1,              NULL }
};

extern char **environ;
Context *get_sixjack_context(void);
void free_sixjack_context(void);
Filter *get_filter(void);

int upipe_init(Upipe * const upipe)
{
    int fds[2];
    
    if (pipe(fds) != 0) {
        upipe->fd_read = upipe->fd_write = -1;
        return -1;
    }
    upipe->fd_read  = fds[0];
    upipe->fd_write = fds[1];
    
    return 0;
}

void upipe_free(Upipe * const upipe)
{
    if (upipe->fd_read != -1) {
        close(upipe->fd_read);
        upipe->fd_read = -1;
    }
    if (upipe->fd_write != -1) {
        close(upipe->fd_write);
        upipe->fd_write = -1;
    }
}

int safe_write(const int fd, const void * const buf_, size_t count,
               const int timeout)
{
    const char *buf = (const char *) buf_;
    ssize_t written;
    struct pollfd pfd;
    
    pfd.fd = fd;
    pfd.events = POLLOUT;
    
    while (count > (size_t) 0) {
        for (;;) {
            if ((written = write(fd, buf, count)) <= (ssize_t) 0) {
                if (errno == EAGAIN) {
                    if (poll(&pfd, (nfds_t) 1, timeout) == 0) {
                        errno = ETIMEDOUT;
                        return -1;
                    }
                } else if (errno != EINTR) {
                    return -1;
                }
                continue;
            }
            break;
        }
        buf += written;
        count -= written;
    }
    return 0;
}

ssize_t safe_read(const int fd, void * const buf_, size_t count)
{
    unsigned char *buf = (unsigned char *) buf_;
    ssize_t readnb;
    
    do {
        while ((readnb = read(fd, buf, count)) < (ssize_t) 0 &&
               errno == EINTR);
        if (readnb < (ssize_t) 0 || readnb > (ssize_t) count) {
            return readnb;
        }
        if (readnb == (ssize_t) 0) {
ret:
            return (ssize_t) (buf - (unsigned char *) buf_);
        }
        count -= readnb;
        buf += readnb;
    } while (count > (ssize_t) 0);
    goto ret;
}

ssize_t safe_read_partial(const int fd, void * const buf_,
                          const size_t max_count)
{
    unsigned char * const buf = (unsigned char * const) buf_;
    ssize_t readnb;

    while ((readnb = read(fd, buf, max_count)) < (ssize_t) 0 &&
           errno == EINTR);
    return readnb;
}

int msgpack_pack_lstring(msgpack_packer * const msgpack_packer,
                         const char * const str, const size_t str_len)
{
    if (msgpack_pack_raw(msgpack_packer, str_len) != 0) {
        return -1;
    }
    return msgpack_pack_raw_body(msgpack_packer, str, str_len);    
}

int msgpack_pack_cstring(msgpack_packer * const msgpack_packer,
                         const char * const str)
{
    return msgpack_pack_lstring(msgpack_packer, str, strlen(str));
}

#define msgpack_pack_mstring(MSGPACK_PACKER, STR) \
    msgpack_pack_lstring((MSGPACK_PACKER), (STR), sizeof (STR) - (size_t) 1U)

const msgpack_object * msgpack_get_map_value_for_key(const msgpack_object_map * const map,
                                                     const char * const key)
{
    const size_t key_len = strlen(key);
    assert(key_len <= UINT32_MAX);
    const size_t key_len_32 = (uint32_t) key_len;
    uint32_t kv_size = map->size;
    const struct msgpack_object_kv *kv;
    const msgpack_object_raw *scanned_key;
    while (kv_size > 0U) {
        kv_size--;
        kv = &map->ptr[kv_size];
        assert(kv->key.type == MSGPACK_OBJECT_RAW);
        scanned_key = &kv->key.via.raw;
        if (scanned_key->size == key_len_32 &&
            memcmp(scanned_key->ptr, key, key_len) == 0) {
            return &kv->val;
        }
    }
    return NULL;
}

int msgpack_pack_cstring_or_nil(msgpack_packer * const msgpack_packer,
                                const char * const str)
{    
    if (str == NULL) {
        return msgpack_pack_nil(msgpack_packer);   
    }
    return msgpack_pack_cstring(msgpack_packer, str);
}

const char *find_name_from_id(const IdName *scanned, const int id)
{
    do {
        if (scanned->id == id) {
            break;
        }
        scanned++;
    } while (scanned->name != NULL);
    assert(scanned->name != NULL);

    return scanned->name;
}

int send_message_to_filter(Filter * const filter)
{
    ssize_t written = safe_write(filter->upipe_stdin.fd_write,
                                 filter->msgpack_sbuffer->data,
                                 filter->msgpack_sbuffer->size, -1);
    msgpack_sbuffer_clear(filter->msgpack_sbuffer);    
    if (written != (ssize_t) 0) {
        return -1;
    }    
    return 0;
}

msgpack_unpacked receive_message_from_filter(Filter * const filter)
{
    msgpack_unpacker * const msgpack_unpacker = filter->msgpack_unpacker;
    msgpack_unpacked message;    
    msgpack_unpacked_init(&message);
    msgpack_unpacker_reserve_buffer(msgpack_unpacker,
                                    FILTER_READ_BUFFER_SIZE);
    ssize_t readnb;    
    while (msgpack_unpacker_next(msgpack_unpacker, &message) <= 0) {
        assert(msgpack_unpacker_buffer_capacity(msgpack_unpacker) > 0U);
        readnb = safe_read_partial
            (filter->upipe_stdout.fd_read,
                msgpack_unpacker_buffer(msgpack_unpacker),
                msgpack_unpacker_buffer_capacity(msgpack_unpacker));
        if (readnb <= (ssize_t) 0) {
            assert(0);
            exit(1);
        }
        msgpack_unpacker_buffer_consumed(msgpack_unpacker, readnb);
    }
    msgpack_object_print(stdout, message.data);
    puts("");
    assert(message.data.type == MSGPACK_OBJECT_MAP);
    
    return message;
}

int parse_common_reply_map(const msgpack_object_map * const map,
                           int * const ret, int * const ret_errno,
                           const int fd)
{
    const msgpack_object * const obj_version =
        msgpack_get_map_value_for_key(map, "version");
    assert(obj_version != NULL);
    assert(obj_version->type == MSGPACK_OBJECT_POSITIVE_INTEGER);
    assert(obj_version->via.u64 == VERSION_MAJOR);
    
    const msgpack_object * const obj_return_code =
        msgpack_get_map_value_for_key(map, "return_code");
    if (obj_return_code != NULL) {
        assert(obj_return_code->type == MSGPACK_OBJECT_POSITIVE_INTEGER ||
               obj_return_code->type == MSGPACK_OBJECT_NEGATIVE_INTEGER);
        const int64_t new_return_code = obj_return_code->via.i64;
        assert(new_return_code >= INT_MIN && new_return_code <= INT_MAX);
        *ret = new_return_code;
    }
    
    const msgpack_object * const obj_ret_errno =
        msgpack_get_map_value_for_key(map, "ret_errno");
    if (obj_ret_errno != NULL) {
        assert(obj_ret_errno->type == MSGPACK_OBJECT_POSITIVE_INTEGER ||
               obj_ret_errno->type == MSGPACK_OBJECT_NEGATIVE_INTEGER);
        const int64_t new_ret_errno = obj_ret_errno->via.i64;
        assert(new_ret_errno >= INT_MIN && new_ret_errno <= INT_MAX);
        *ret_errno = new_ret_errno;
    }
    
    const msgpack_object * const obj_force_close =
        msgpack_get_map_value_for_key(map, "force_close");
    if (obj_force_close != NULL) {
        assert(obj_force_close->type == MSGPACK_OBJECT_BOOLEAN);
        const bool force_close = obj_force_close->via.boolean;
        if (force_close != 0) {
            close(fd);
        }
    }
    
    return 0;
}

int socket_parse_filter_reply(Filter * const filter, int * const ret,
                              int * const ret_errno, const int fd)
{
    msgpack_unpacked message = receive_message_from_filter(filter);
    const msgpack_object_map * const map = &message.data.via.map;
    
    parse_common_reply_map(map, ret, ret_errno, fd);
    
    msgpack_unpacked_destroy(&message);

    return 0;
}

int before_apply_filter(const int ret, const int ret_errno, const int fd,
                        const unsigned int nongeneric_items,
                        const char * const function)
{
    Filter * const filter = get_filter();
    assert(filter->msgpack_packer != NULL);
    msgpack_packer * const msgpack_packer = filter->msgpack_packer;
    msgpack_packer_init(msgpack_packer, filter->msgpack_sbuffer,
                        msgpack_sbuffer_write);
    msgpack_pack_map(msgpack_packer, nongeneric_items + 5U);
    
    msgpack_pack_mstring(msgpack_packer, "version");
    msgpack_pack_unsigned_short(msgpack_packer, VERSION_MAJOR);
    
    msgpack_pack_mstring(msgpack_packer, "function");
    msgpack_pack_cstring(msgpack_packer, function);

    msgpack_pack_mstring(msgpack_packer, "return_code");
    msgpack_pack_int(msgpack_packer, ret);
    
    msgpack_pack_mstring(msgpack_packer, "errno");
    msgpack_pack_int(msgpack_packer, ret_errno);

    msgpack_pack_mstring(msgpack_packer, "fd");
    msgpack_pack_int(msgpack_packer, fd);
    
    return 0;
}

int socket_apply_filter(int * const ret, int * const ret_errno,
                        const int domain, const int type,
                        const int protocol)
{
    Filter * const filter = get_filter();
    msgpack_packer * const msgpack_packer = filter->msgpack_packer;    
    const int fd = *ret;    
    before_apply_filter(*ret, *ret_errno, fd, 3U, "socket");    
    
    msgpack_pack_mstring(msgpack_packer, "domain");    
    const char * const domain_name = find_name_from_id(pf_domains, domain);
    msgpack_pack_cstring_or_nil(msgpack_packer, domain_name);
    
    msgpack_pack_mstring(msgpack_packer, "type");
    const char * const type_name = find_name_from_id(sock_types, type);
    msgpack_pack_cstring_or_nil(msgpack_packer, type_name);
    
    msgpack_pack_mstring(msgpack_packer, "protocol");
    const char * const protocol_name = find_name_from_id(ip_protos, protocol);
    msgpack_pack_cstring_or_nil(msgpack_packer, protocol_name);
    
    if (send_message_to_filter(filter) != 0) {
        return -1;
    }    
    return socket_parse_filter_reply(filter, ret, ret_errno, fd);
}

int socket(int domain, int type, int protocol)
{
    static int (* __real_socket)(int domain, int type, int protocol);

    if (__real_socket == NULL) {
        __real_socket = dlsym(RTLD_NEXT, "socket");
        assert(__real_socket != NULL);        
    }
    int ret = __real_socket(domain, type, protocol);
    int ret_errno = errno;
    socket_apply_filter(&ret, &ret_errno, domain, type, protocol);
    errno = ret_errno;
    
    return ret;
}

Context *get_sixjack_context(void)
{
    static Context context;
    if (context.initialized != 0) {
        return &context;
    }
    
    Filter * const filter = &context.filter;
    int ret;
    char *argv[] = { "6jack-filter", NULL };

    upipe_init(&filter->upipe_stdin);
    upipe_init(&filter->upipe_stdout);
    
    posix_spawn_file_actions_t file_actions;
    posix_spawn_file_actions_init(&file_actions);
    posix_spawn_file_actions_adddup2(&file_actions,
                                     filter->upipe_stdin.fd_read, 0);
    posix_spawn_file_actions_adddup2(&file_actions,
                                     filter->upipe_stdout.fd_write, 1);
    posix_spawn_file_actions_addclose(&file_actions,
                                      filter->upipe_stdin.fd_read);
    posix_spawn_file_actions_addclose(&file_actions,
                                      filter->upipe_stdin.fd_write);
    posix_spawn_file_actions_addclose(&file_actions,
                                      filter->upipe_stdout.fd_read);
    posix_spawn_file_actions_addclose(&file_actions,
                                      filter->upipe_stdout.fd_write);
    ret = posix_spawn(&filter->pid, "./a.rb",
                      &file_actions, NULL, argv, environ);
    if (ret != 0) {
        errno = ret;
        perror("posix_spawn");
        return NULL;
    }
    printf("Child = %u\n", (unsigned int) filter->pid);
        
    posix_spawn_file_actions_destroy(&file_actions);    
    filter->msgpack_sbuffer = msgpack_sbuffer_new();
    filter->msgpack_packer = msgpack_packer_new(filter->msgpack_sbuffer,
                                                msgpack_sbuffer_write);
    filter->msgpack_unpacker = msgpack_unpacker_new(FILTER_UNPACK_BUFFER_SIZE);
    context.initialized = 1;
    atexit(free_sixjack_context);
    
    return &context;
}

Filter *get_filter(void)
{
    return &get_sixjack_context()->filter;
}

void free_sixjack_context(void)
{
    Context * const context = get_sixjack_context();
    Filter * const filter = &context->filter;

    msgpack_sbuffer_free(filter->msgpack_sbuffer);
    filter->msgpack_sbuffer = NULL;
    msgpack_packer_free(filter->msgpack_packer);
    filter->msgpack_packer = NULL;
    msgpack_unpacker_free(filter->msgpack_unpacker);
    filter->msgpack_unpacker = NULL;
    
    context->initialized = 0;
}

int main(void)
{
    socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    socket(PF_INET6, SOCK_DGRAM, IPPROTO_UDP);    

    return 0;
}
