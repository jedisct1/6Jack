
#ifndef __HOOKS_H__
#define __HOOKS_H__ 1

#include "hook-bind.h"
#include "hook-close.h"
#include "hook-connect.h"
#include "hook-read.h"
#include "hook-recv.h"
#include "hook-recvfrom.h"
#include "hook-recvmsg.h"
#include "hook-send.h"
#include "hook-sendmsg.h"
#include "hook-sendto.h"
#include "hook-socket.h"
#include "hook-write.h"

int hooks_init(void);

#endif
