#ifndef __ZX_STRUCT__
#define __ZX_STRUCT__

#define RET_OK 0
#define RET_ERROR -1

#define TYPE_CHANNEL_TO_SDK_BEGIN		0x41
#define TYPE_CHANNEL_TO_SDK_ING			0x42
#define TYPE_CHANNEL_TO_SDK_END			0x43
#define TYPE_CHANNEL_TO_SERVER_BEGIN	0x44
#define TYPE_CHANNEL_TO_SERVER_ING		0x45
#define TYPE_CHANNEL_TO_SERVER_END		0x46

#define TYPE_ERROR			0x51
#define TYPE_OK				0x52
#define TYPE_CONNECT		0x53
#define TYPE_TRANS_DATA		0x54
#define TYPE_CONNECT_CLOSE	0x55
#define TYPE_ENCODE_ENABLE	0x56
#define TYPE_SET_NEW_KEY	0x57
#define TYPE_GET_IMSI		0x58
#define TYPE_GET_VERNAME	0x59
#define TYPE_GET_VERCODE	0x5A
#define TYPE_SET_VPS_TOTAL_IDLE		0x5B
#define TYPE_SET_HEARTBEAT_TIMEOUT	0x5C



#define TYPE_RESP_SIZE_SYNC		0x61	//未回传 response的字节数同步到服务，用于服务端统计
#define TYPE_REFER_DEL_SYNC		0x62	

#define TYPE_OPT_NO_RESP		0xC0	//异步response

#define TYPE_ERROR_UNKNOW_TYPE	0xE1
#define TYPE_ERROR_IN_FIFO		0xE2

// 心跳字段
#define TYPE_HEART				0xFF


typedef long long zxint64;
typedef unsigned long long zxuint64;


extern zxuint64 g_time;

#endif //__ZX_STRUCT__
