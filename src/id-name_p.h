
#ifndef __ID_NAME_P_H__
#define __ID_NAME_P_H__ 1

static IdName pf_domains[] = {
    { PF_LOCAL,  "PF_LOCAL" },
    { PF_UNIX,   "PF_UNIX" },
    { PF_INET,   "PF_INET" },
    { PF_ROUTE,  "PF_ROUTE" },
    { PF_KEY,    "PF_KEY" },
    { PF_INET6,  "PF_INET6" },
#ifdef PF_SYSTEM
    { PF_SYSTEM, "PF_SYSTEM" },
#endif
#ifdef PF_NDRV
    { PF_NDRV,   "PF_NDRV" },
#endif
#ifdef PF_NETLINK
    { PF_NETLINK,"PF_NETLINK" },
#endif
#ifdef PF_FILE
    { PF_FILE,   "PF_FILE" },
#endif
    { -1,        NULL }
};

static IdName sock_types[] = {
    { SOCK_STREAM,    "SOCK_STREAM" },
    { SOCK_DGRAM,     "SOCK_DGRAM" },
    { SOCK_RAW,       "SOCK_RAW" },
    { SOCK_SEQPACKET, "SOCK_SEQPACKET" },
    { SOCK_RDM,       "SOCK_RDM" },
    { -1,             NULL }
};

static IdName ip_protos[] = {
    { IPPROTO_IP,      "IPPROTO_IP" },
    { IPPROTO_ICMP,    "IPPROTO_ICMP" },
    { IPPROTO_IGMP,    "IPPROTO_IGMP" },
#ifdef IPPROTO_IPV4    
    { IPPROTO_IPV4,    "IPPROTO_IPV4" },
#endif
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
#ifdef IPPROTO_IPCOMP    
    { IPPROTO_IPCOMP,  "IPPROTO_IPCOMP" },
#endif    
    { IPPROTO_PIM,     "IPPROTO_PIM" },
#ifdef IPPROTO_PGM
    { IPPROTO_PGM,     "IPPROTO_PGM" },
#endif
    { -1,              NULL }
};

#endif
