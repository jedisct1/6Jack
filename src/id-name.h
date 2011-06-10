
#ifndef __ID_NAME_H__
#define __ID_NAME_H__ 1

typedef struct IdName_ {
    int id;
    const char *name;
} IdName;

const char *idn_find_name_from_id(const IdName *scanned, const int id);
const IdName *idn_get_pf_domains(void);
const IdName *idn_get_sock_types(void);
const IdName *idn_get_ip_protos(void);

#endif
