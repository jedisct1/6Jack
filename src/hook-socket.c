
#include "common.h"
#include "filter.h"
#include "hook-socket.h"

static int socket_parse_filter_reply(Filter * const filter, int * const ret,
                                     int * const ret_errno, const int fd)
{
    msgpack_unpacked * const message = receive_message_from_filter(filter);
    const msgpack_object_map * const map = &message->data.via.map;
    parse_common_reply_map(map, ret, ret_errno, fd);

    return 0;
}

static int socket_apply_filter(int * const ret, int * const ret_errno,
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
