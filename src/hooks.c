
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
    __real_recvmsg_init();
    __real_send_init();
    __real_sendmsg_init();    
    __real_sendto_init();
    __real_socket_init();
    __real_write_init();
    __real_writev_init();
    
    return 0;
}
