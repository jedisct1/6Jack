
#ifndef __MSGPACK_EXTENSIONS__
#define __MSGPACK_EXTENSIONS__ 1

#include <msgpack.h>

int msgpack_pack_lstring(msgpack_packer * const msgpack_packer,
                         const char * const str, const size_t str_len);

int msgpack_pack_cstring(msgpack_packer * const msgpack_packer,
                         const char * const str);

#define msgpack_pack_mstring(MSGPACK_PACKER, STR) \
    msgpack_pack_lstring((MSGPACK_PACKER), (STR), sizeof (STR) - (size_t) 1U)

const msgpack_object * msgpack_get_map_value_for_key(const msgpack_object_map * const map,
                                                     const char * const key);

int msgpack_pack_cstring_or_nil(msgpack_packer * const msgpack_packer,
                                const char * const str);

#endif
