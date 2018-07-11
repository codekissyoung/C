#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "buffer.h"
#include "protocol.h" 

void dispatch_write(struct io_buff *buff);

extern "C" {
int mod_init(char *conf) {
	return 0;
}

/*
int mod_proc(char *in, char *out, int out_len) {
	int len = *(unsigned int *)in;
	memcpy(out, in, len);
	return 0;
}
*/

int mod_proc(struct io_buff *buff) {
	//int len = *(unsigned int *)in;
    req_rsp_pack *rsp = buff->wbuff;

    memcpy(buff->wbuff, buff->rbuff, sizeof(req_rsp_pack));
	memcpy(rsp->data, "ok", 3);
    rsp->len = sizeof(req_rsp_pack) + 3;

    //dispatch_write(buff);
	return 0;
}

int mod_uninit() {
	return 0;
}
}
