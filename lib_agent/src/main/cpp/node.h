#pragma once
#include "VpsMgr.h"


typedef struct _node_t node_t;

typedef unsigned int(*p_func_call_back_get_flow_limit)(vps_mgr_t *mgr);
typedef int(*p_func_call_back_transfer_data)(vps_mgr_t *mgr, int type, int refer, unsigned char *data, int datalen, int is_cmd);
typedef int(*p_func_call_back_delete_data)(vps_mgr_t *mgr, int type, int refer);
typedef int(*p_func_call_back_remove_node)(vps_mgr_t *mgr, node_t *node);
typedef int(*p_func_node_release)(struct _node_t *node);
typedef int(*p_func_node_recv_package)(struct _node_t *node, int type, int refer, unsigned char *data, int datalen);

struct _node_t
{
	struct list_head node;
	int refer;
	vps_mgr_t *mgr;
	p_func_call_back_get_flow_limit get_flow_limit;
	p_func_call_back_transfer_data transfer_data;
	p_func_call_back_delete_data delete_data;
	p_func_call_back_remove_node remove_node;
	p_func_node_release release;
	p_func_node_recv_package recv_package;
};