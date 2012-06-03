
#define DONT_BYPASS_HOOKS   1

#include "common.h"

#ifdef USE_INTERPOSERS
DLL_PUBLIC const struct { void *hook_func; void *real_func; }
sixjack_interposers[] __attribute__ ((section("__DATA, __interpose"))) = {
    { INTERPOSE(bind), bind },
    { INTERPOSE(close), close },
    { INTERPOSE(connect), connect },
    { INTERPOSE(read), read },
    { INTERPOSE(recv), recv },
    { INTERPOSE(recvfrom), recvfrom },
    { INTERPOSE(recvmsg), recvmsg },
    { INTERPOSE(send), send },
    { INTERPOSE(sendmsg), sendmsg },
    { INTERPOSE(sendto), sendto },
    { INTERPOSE(socket), socket },
    { INTERPOSE(write), write },
    { INTERPOSE(writev), writev },
};
#endif
