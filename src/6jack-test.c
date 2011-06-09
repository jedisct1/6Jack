#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(void)
{
    socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    socket(PF_INET6, SOCK_DGRAM, IPPROTO_UDP);    

    return 0;
}
