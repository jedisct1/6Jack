
#include "common.h"
#include "id-name.h"
#include "id-name_p.h"

const char *find_name_from_id(const IdName *scanned, const int id)
{    
    assert(scanned->name != NULL);
    do {
        if (scanned->id == id) {
            break;
        }
        scanned++;
    } while (scanned->name != NULL);
    assert(scanned->name != NULL);

    return scanned->name;
}

const IdName *get_pf_domains(void)
{
    return pf_domains;
}

const IdName *get_sock_types(void)
{
    return sock_types;
}

const IdName *get_ip_protos(void)
{
    return ip_protos;
}
