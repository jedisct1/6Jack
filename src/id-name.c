
#include "common.h"
#include "id-name.h"
#include "id-name_p.h"

const char *idn_find_name_from_id(const IdName *scanned, const int id)
{    
    assert(scanned->name != NULL);
    do {
        if (scanned->id == id) {
            return scanned->name;            
        }
        scanned++;
    } while (scanned->name != NULL);
    
    return NULL;
}

int idn_find_id_from_name(const IdName *scanned, int * const id,
                          const char * const name, const size_t name_len)
{
    assert(scanned->name != NULL);
    do {
        if (strncasecmp(scanned->name, name, name_len) == 0) {
            *id = scanned->id;
            return 0;
        }
        scanned++;
    } while (scanned->name != NULL);
    
    return -1;
}

const IdName *idn_get_pf_domains(void)
{
    return pf_domains;
}

const IdName *idn_get_sock_types(void)
{
    return sock_types;
}

const IdName *idn_get_ip_protos(void)
{
    return ip_protos;
}
