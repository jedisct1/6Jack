
#define DONT_BYPASS_HOOKS 1

#include "common.h"
#include "hooks.h"

int hooks_init(void)
{
    __real_bind_init();
    __real_close_init();
    __real_connect_init();
    __real_read_init();
    __real_recv_init();
    __real_recvfrom_init();
    __real_socket_init();
    __real_write_init();    
    
    return 0;
}
