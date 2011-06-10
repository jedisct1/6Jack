
#define DONT_BYPASS_HOOKS 1

#include "common.h"
#include "hooks.h"

int hooks_init(void)
{
    __real_socket_init();
    
    return 0;
}
