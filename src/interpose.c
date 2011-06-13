
#define DONT_BYPASS_HOOKS   1

#include "common.h"

#ifdef USE_INTERPOSERS
DLL_PUBLIC const struct { void *hook_func; void *real_func; }
sixjack_interposers[] __attribute__ ((section("__DATA, __interpose"))) = {
    { INTERPOSE(close), close },
    { INTERPOSE(connect), connect },
    { INTERPOSE(socket), socket },
};
#endif
