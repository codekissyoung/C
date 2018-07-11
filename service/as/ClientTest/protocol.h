#ifndef _PROTOCOL_H
#define _PROTOCOL_H

/* error number */
enum server_error_num {
	ERR_MAGIC_CODE = -1001,
	ERR_CMD_CODE   = -1002,
};

#pragma pack(1)

typedef struct {
	unsigned int len;		/* 包长度，包括包头 */
	unsigned int magic;		/* magic code */
	unsigned int cmd;		/* command number */
	unsigned int sequence;	/* 序列号 */
	int state;				/* 记录错误代码等信息 */
    char data[0]; /* 包体 */
} pack_header;

#pragma pack()

typedef pack_header req_rsp_pack;

#endif
