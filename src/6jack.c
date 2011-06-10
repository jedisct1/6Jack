
#include "common.h"
#include "utils.h"
#include "msgpack-extensions.h"
#include "id-name.h"
#include "6jack.h"

Filter *get_filter(void)
{
    return &get_sixjack_context()->filter;
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

msgpack_unpacked *receive_message_from_filter(Filter * const filter)
{
    msgpack_unpacker * const msgpack_unpacker = &filter->msgpack_unpacker;    
    msgpack_unpacker_destroy(msgpack_unpacker);
    msgpack_unpacker_init(msgpack_unpacker, FILTER_UNPACK_BUFFER_SIZE);
    msgpack_unpacker_reserve_buffer(msgpack_unpacker,
                                    FILTER_READ_BUFFER_SIZE);
    msgpack_unpacked * const message = &filter->message;
    
    ssize_t readnb;    
    while (msgpack_unpacker_next(msgpack_unpacker, message) <= 0) {
        assert(msgpack_unpacker_buffer_capacity(msgpack_unpacker) > 0U);
        readnb = safe_read_partial
            (filter->upipe_stdout.fd_read,
                msgpack_unpacker_buffer(msgpack_unpacker),
                msgpack_unpacker_buffer_capacity(msgpack_unpacker));
        if (readnb <= (ssize_t) 0) {
            assert(0);
            return NULL;
        }
        msgpack_unpacker_buffer_consumed(msgpack_unpacker, readnb);
    }
    msgpack_object_print(stdout, message->data);
    puts("");
    assert(message->data.type == MSGPACK_OBJECT_MAP);
    
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
        msgpack_get_map_value_for_key(map, "errno");
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
    msgpack_unpacked * const message = receive_message_from_filter(filter);
    const msgpack_object_map * const map = &message->data.via.map;
    parse_common_reply_map(map, ret, ret_errno, fd);

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
    const char * const domain_name =
        find_name_from_id(get_pf_domains(), domain);
    msgpack_pack_cstring_or_nil(msgpack_packer, domain_name);
    
    int type_ = type;
#ifdef SOCK_NONBLOCK
    type_ &= ~SOCK_NONBLOCK;
#endif
#ifdef SOCK_CLOEXEC
    type_ &= ~SOCK_CLOEXEC;
#endif
    msgpack_pack_mstring(msgpack_packer, "type");
    const char * const type_name =
        find_name_from_id(get_sock_types(), type_);
    msgpack_pack_cstring_or_nil(msgpack_packer, type_name);
    
    msgpack_pack_mstring(msgpack_packer, "protocol");
    const char * const protocol_name =
        find_name_from_id(get_ip_protos(), protocol);
    msgpack_pack_cstring_or_nil(msgpack_packer, protocol_name);
    
    if (send_message_to_filter(filter) != 0) {
        return -1;
    }    
    return socket_parse_filter_reply(filter, ret, ret_errno, fd);
}

int INTERPOSE(socket)(int domain, int type, int protocol)
{
    static int (* __real_socket)(int domain, int type, int protocol);

#ifdef USE_INTERPOSERS
    __real_socket = socket;
#else
    if (__real_socket == NULL) {
        __real_socket = dlsym(RTLD_NEXT, "socket");        
        assert(__real_socket != NULL);        
    }
#endif
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
    char *argv[] = { (char *) "example-filter", NULL };

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
    ret = posix_spawn(&filter->pid, "./example-filter.rb",
                      &file_actions, NULL, argv, environ);
    if (ret != 0) {
        errno = ret;
        perror("posix_spawn");
        exit(1);
    }
    printf("Child = %u\n", (unsigned int) filter->pid);
        
    posix_spawn_file_actions_destroy(&file_actions);    
    filter->msgpack_sbuffer = msgpack_sbuffer_new();
    filter->msgpack_packer = msgpack_packer_new(filter->msgpack_sbuffer,
                                                msgpack_sbuffer_write);
    msgpack_unpacker_init(&filter->msgpack_unpacker,
                          FILTER_UNPACK_BUFFER_SIZE);
    msgpack_unpacked_init(&filter->message);
    context.initialized = 1;
    atexit(free_sixjack_context);
    
    return &context;
}

void free_sixjack_context(void)
{
    Context * const context = get_sixjack_context();
    Filter * const filter = &context->filter;

    if (filter->msgpack_sbuffer != NULL) {
        msgpack_sbuffer_free(filter->msgpack_sbuffer);
        filter->msgpack_sbuffer = NULL;
    }
    if (filter->msgpack_packer != NULL) {
        msgpack_packer_free(filter->msgpack_packer);
        filter->msgpack_packer = NULL;
    }
    msgpack_unpacker_destroy(&filter->msgpack_unpacker);   
    msgpack_unpacked_destroy(&filter->message);
    
    context->initialized = 0;
}

#ifdef USE_INTERPOSERS
const struct { void *hook_func; void *real_func; } sixjack_interposers[]
__attribute__ ((section("__DATA, __interpose"))) = {
    { INTERPOSE(socket), socket }
};
#endif

