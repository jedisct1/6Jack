
#define DEFINE_HOOK_GLOBALS 1
#define DONT_BYPASS_HOOKS   1

#include "common.h"
#include "filter.h"
#include "hook-socket.h"

int (* __real_socket)(int domain, int type, int protocol);

static FilterReplyResult filter_parse_reply(FilterReplyResultBase * const rb,
                                            int * const domain,
                                            int * const type,
                                            int * const protocol)
{
    msgpack_unpacked * const message = filter_receive_message(rb->filter);    
    const msgpack_object_map * const map = &message->data.via.map;
    FilterReplyResult reply_result = filter_parse_common_reply_map(rb, map);
    
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
    return reply_result;
}

static FilterReplyResult filter_apply(FilterReplyResultBase * const rb,
                                      int * const domain, int * const type,
                                      int * const protocol)
{
    msgpack_packer * const msgpack_packer = rb->filter->msgpack_packer;    
    filter_before_apply(rb, 3U, "socket",
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
    
    if (filter_send_message(rb->filter) != 0) {
        return FILTER_REPLY_RESULT_ERROR;
    }
    return filter_parse_reply(rb, domain, type, protocol);
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
    FilterReplyResultBase rb = {
        .pre = true, .ret = &ret, .ret_errno = &ret_errno, .fd = -1
    };
    if (bypass_filter == false && (rb.filter = filter_get()) &&
        filter_apply(&rb, &domain, &type, &protocol)
        == FILTER_REPLY_BYPASS) {
        bypass_call = true;
    }
    if (bypass_call == false) {
        ret = __real_socket(domain, type, protocol);
        ret_errno = errno;
    }
    if (bypass_filter == false) {
        rb.fd = ret;
        rb.pre = false;
        filter_apply(&rb, &domain, &type, &protocol);
    }
    errno = ret_errno;
    
    return ret;
}
