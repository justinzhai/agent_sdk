#pragma once

#include "zxlist.h"
#include "zxstring.h"
#include "node.h"

typedef struct _fifo_node_t fifo_node_t;

struct _fifo_node_t
{
	// node_t
	struct list_head node;
	int refer;
	vps_mgr_t *mgr;
	p_func_call_back_get_flow_limit get_flow_limit;
	p_func_call_back_transfer_data transfer_data;
	p_func_call_back_delete_data delete_data;
	p_func_call_back_remove_node remove_node;
	p_func_node_release release;
	p_func_node_recv_package recv_package;
	// _fifo_node_t
	int fd[2];
	int id;
	int closeRequest;
	int closeResponse;
	zx_string *zxsTransferData;
	zx_string *zxsResponseData;
	select_node_t *snode;
};

node_t *fifo_node_create(int refer, vps_mgr_t *mgr, p_func_call_back_get_flow_limit get_flow_limit,
	p_func_call_back_transfer_data transfer_data, p_func_call_back_delete_data delete_data, p_func_call_back_remove_node remove_node);
