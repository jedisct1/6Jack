
#include "common.h"
#include "msgpack-extensions.h"

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
