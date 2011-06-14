
#define DEFINE_HOOK_GLOBALS 1
#define DONT_BYPASS_HOOKS   1

#include "common.h"
#include "filter.h"
#include "hook-socket.h"

int (* __real_socket)(int domain, int type, int protocol);

static FilterReplyResult filter_parse_reply(Filter * const filter,
                                            int * const ret,
                                            int * const ret_errno,
                                            const int fd, int * const domain,
                                            int * const type,
                                            int * const protocol)
{
    msgpack_unpacked * const message = filter_receive_message(filter);
    const msgpack_object_map * const map = &message->data.via.map;
    filter_parse_common_reply_map(map, ret, ret_errno, fd);
    
    const msgpack_object * const obj_domain =
        msgpack_get_map_value_for_key(map, "domain");    
    int new_domain;    
    if (obj_domain != NULL && obj_domain->type == MSGPACK_OBJECT_RAW &&
        idn_find_id_from_name(idn_get_pf_domains(), &new_domain,
                              obj_domain->via.raw.ptr,
                              (size_t) obj_domain->via.raw.size) == 0) {
        *domain = new_domain;
    }

    const msgpack_object * const obj_type =
        msgpack_get_map_value_for_key(map, "type");
    int new_type;    
    if (obj_type != NULL && obj_type->type == MSGPACK_OBJECT_RAW &&
        idn_find_id_from_name(idn_get_sock_types(), &new_type,
                              obj_type->via.raw.ptr,
                              (size_t) obj_type->via.raw.size) == 0) {
        *type = new_type;
    }

    const msgpack_object * const obj_protocol =
        msgpack_get_map_value_for_key(map, "protocol");
    int new_protocol;
    if (obj_protocol != NULL && obj_protocol->type == MSGPACK_OBJECT_RAW &&
        idn_find_id_from_name(idn_get_ip_protos(), &new_protocol,
                              obj_protocol->via.raw.ptr,
                              (size_t) obj_protocol->via.raw.size) == 0) {
        *protocol = new_protocol;
    }    
    return 0;
}

static FilterReplyResult filter_apply(const bool pre, int * const ret,
                                      int * const ret_errno,
                                      int * const domain, int * const type,
                                      int * const protocol)
{
    Filter * const filter = filter_get();
    msgpack_packer * const msgpack_packer = filter->msgpack_packer;    
    const int fd = *ret;    
    filter_before_apply(pre, *ret, *ret_errno, fd, 3U, "socket",
                        NULL, (socklen_t) 0U, NULL, (socklen_t) 0U);
    
    msgpack_pack_mstring(msgpack_packer, "domain");    
    const char * const domain_name =
        idn_find_name_from_id(idn_get_pf_domains(), *domain);
    msgpack_pack_cstring_or_nil(msgpack_packer, domain_name);
    
    int type_ = *type;
#ifdef SOCK_NONBLOCK
    type_ &= ~SOCK_NONBLOCK;
#endif
#ifdef SOCK_CLOEXEC
    type_ &= ~SOCK_CLOEXEC;
#endif
    msgpack_pack_mstring(msgpack_packer, "type");
    const char * const type_name =
        idn_find_name_from_id(idn_get_sock_types(), type_);
    msgpack_pack_cstring_or_nil(msgpack_packer, type_name);
    
    msgpack_pack_mstring(msgpack_packer, "protocol");
    const char * const protocol_name =
        idn_find_name_from_id(idn_get_ip_protos(), *protocol);
    msgpack_pack_cstring_or_nil(msgpack_packer, protocol_name);
    
    if (filter_send_message(filter) != 0) {
        return -1;
    }
    return filter_parse_reply(filter, ret, ret_errno, fd,
                              domain, type, protocol);
}

int __real_socket_init(void)
{
#ifdef USE_INTERPOSERS
    __real_socket = socket;
#else
    if (__real_socket == NULL) {
        __real_socket = dlsym(RTLD_NEXT, "socket");        
        assert(__real_socket != NULL);        
    }
#endif
    return 0;
}

int INTERPOSE(socket)(int domain, int type, int protocol)
{
    __real_socket_init();
    const bool bypass_filter = getenv("SIXJACK_BYPASS") != NULL;
    int ret = 0;
    int ret_errno = 0;    
    bool bypass_call = false;
    if (bypass_filter == false &&
        filter_apply(true, &ret, &ret_errno, &domain, &type, &protocol)
        == FILTER_REPLY_BYPASS) {
        bypass_call = true;
    }
    if (bypass_call == false) {
        ret = __real_socket(domain, type, protocol);
        ret_errno = errno;
    }
    if (bypass_filter == false) {
        filter_apply(false, &ret, &ret_errno, &domain, &type, &protocol);
    }
    errno = ret_errno;
    
    return ret;
}
