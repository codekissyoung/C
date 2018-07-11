#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "buffer.h"
#include "protocol.h"

extern "C" int mod_init(char *conf)
{
    return 0;
}

extern "C" int mod_proc(struct io_buff *buff)
{
    req_rsp_pack *rsp = buff->wbuff;
    memcpy(buff->wbuff, buff->rbuff, sizeof(req_rsp_pack));
    memcpy(rsp->data, "ok", 3);
    rsp->len = sizeof(req_rsp_pack) + 3;
    printf("%s\n" ,"echo ok");
    return 0;
}

extern "C" int mod_uninit()
{
    return 0;
}
