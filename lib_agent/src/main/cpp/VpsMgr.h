#pragma once

#include "zxlist.h"
#include "zxselect.h"
#include "zxstring.h"
#include "ChannelMgr.h"
#include "flowStatistics.h"
#include "DataQueue.h"

#define VPS_SO_VERSION_CODE 19
#define VPS_SO_VERSION_NAME "1.2.7"

#define VPS_KEY ("vps_test_key")
#define VPS_ENCODE_ENABLE	1
#define VPS_ENCODE_DISABLE	0

typedef struct _vps_mgr_t
{
	struct list_head workNodeHeader;	//vps work node
	int fd;			//³¤Á¬½Ó¾ä±ú
	int waitHeartResponse;
	int encode;
	select_t *st;
	select_node_t *node;
	zx_string *zxsRecvData;
	zx_string *zxsSendData;
	zx_string *zxsKey;
	zx_string *zxsIMSI;
	unsigned int offsetRecvData;
	unsigned int offsetSendData;
	int heartbeatTimeout;
	unsigned int heartbeatLastRecv;
	channel_mgr_t *channel_mgr;
	flow_statistics_t *flow_statistics;
	zxuint64 time_last_print_flow;
	data_queue_t *data_queue;
	unsigned int flow_limit;
	unsigned int flow_limit_earlier;
	unsigned int flow_cache;
	zxuint64 time_last_flow_limit_calc;
}vps_mgr_t;

void vps_mgr_set_config(int timeout_connect);
void vps_mgr_set_exit_timestamp(long long exit_timestamp);
void vps_mgr_set_bind_addr(const char *ip);
int start(const char *szHost, int nPort, const char *szIMSI);
int close();
int vps_mgr_set_channel_response(int id, unsigned char *data, int datalen);
