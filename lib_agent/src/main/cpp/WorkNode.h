#pragma once

#include "zxlist.h"
#include "zxstring.h"
#include "node.h"

typedef struct _work_node_t work_node_t;

struct _work_node_t
{
	// node_t
	//node_t node;
	struct list_head node;
	int refer;
	vps_mgr_t *mgr;
	p_func_call_back_get_flow_limit get_flow_limit;
	p_func_call_back_transfer_data transfer_data;
	p_func_call_back_delete_data delete_data;
	p_func_call_back_remove_node remove_node;
	p_func_node_release release;
	p_func_node_recv_package recv_package;
	// _work_node_t
	int fd;
	int closeRequest;
	//int closeResponse;
	zx_string *zxsTransferData;
	zx_string *zxsResponseData;
	unsigned int offsetTransferData;
	unsigned int offsetResponseData;
	select_node_t *snode;
	int sync_resp;	// 1: response send back; 0: no response
	unsigned int totalResponseSize;
	int sendBytys;
	int recvBytes;
	flow_statistics_t *flow_statistics;
	zxint64 lastModeWithRead;
	int read2k;
};

node_t *work_node_create(int refer, vps_mgr_t *mgr, p_func_call_back_get_flow_limit get_flow_limit,
	p_func_call_back_transfer_data transfer_data, p_func_call_back_delete_data delete_data, p_func_call_back_remove_node remove_node);

void work_node_set_config(int timeout_connect, int timeout_total);