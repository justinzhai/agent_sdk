#pragma once
#include "zxlist.h"
#include "semx.h"


typedef struct
{
	struct list_head head;
	semx_t *sem;
	int id_next;
}channel_mgr_t;

typedef struct
{
	struct list_head node;
	int id;
	int fd;
}channel_node_t;

extern channel_mgr_t g_channel_mgr;

int channel_mgr_init(channel_mgr_t *mgr);
int channel_mgr_release(channel_mgr_t *mgr);
int channel_mgr_list_clear(channel_mgr_t *mgr);

int channel_mgr_add_node(channel_mgr_t *mgr, channel_node_t *node);
int channel_mgr_del_node(channel_mgr_t *mgr, int id);
channel_node_t* channel_mgr_get_node(channel_mgr_t *mgr, int id);
channel_node_t* channel_mgr_get_node_and_del_from_list(channel_mgr_t *mgr, int id);

// id: 加入到channel_mgr的id， 用于删除结点
int channel_mgr_send_data_to_sdk(channel_mgr_t *mgr, int fd, unsigned char *data, int datalen, int *id);
int channel_mgr_send_data_to_server(channel_mgr_t *mgr, int id, unsigned char *data, int datalen);
