#include "ChannelMgr.h"
#include "zxstruct.h"
#include <stdlib.h>
#include "zxlog.h"
#include "zxpipe.h"
#include <unistd.h>


extern int callback_to_java_channel(int id, unsigned char *data, int datalen);

channel_mgr_t g_channel_mgr = { 0 };
zxuint64 g_time = 0;
int channel_mgr_init(channel_mgr_t * mgr)
{
	init_list_head(&mgr->head);
	mgr->id_next = 1;

	mgr->sem = semaphore_create();

	semaphore_post(mgr->sem);

	return RET_OK;
}

int channel_mgr_release(channel_mgr_t * mgr)
{
	int ret = RET_OK;

	channel_mgr_list_clear(mgr);

	ret = semphore_close(mgr->sem);

	return ret;
}

int channel_mgr_list_clear(channel_mgr_t * mgr)
{
	struct list_head *pos = NULL;
	semaphore_wait(mgr->sem);
	list_for_each(pos, &mgr->head) {
		channel_node_t *node = list_entry(pos, channel_node_t, node);
		pos = pos->prev;
		free(node);
	}
	semaphore_post(mgr->sem);
	return RET_OK;
}

int channel_mgr_list_count(channel_mgr_t * mgr)
{
	int count = 0;
	struct list_head *pos = NULL;
	semaphore_wait(mgr->sem);
	list_for_each(pos, &mgr->head) {
		count++;
	}
	semaphore_post(mgr->sem);
	return count;
}

int channel_mgr_add_node(channel_mgr_t *mgr, channel_node_t * node)
{
	semaphore_wait(mgr->sem);
	list_add(&node->node, &mgr->head);
	semaphore_post(mgr->sem);

	return RET_OK;
}

int channel_mgr_del_node(channel_mgr_t *mgr, int id)
{
	channel_node_t *node = channel_mgr_get_node_and_del_from_list(mgr, id);
	if (node != NULL) {
		free(node);
	}
	return RET_OK;
}

channel_node_t* channel_mgr_get_node(channel_mgr_t *mgr, int id)
{
	channel_node_t *ret_node = NULL;
	if (mgr == NULL || mgr->sem == NULL) return NULL;

	semaphore_wait(mgr->sem);
	struct list_head *pos = NULL;
	list_for_each(pos, &mgr->head) {
		channel_node_t *node = list_entry(pos, channel_node_t, node);
		if (node->id == id) {
			ret_node = node;
			break;
		}
	}
	semaphore_post(mgr->sem);

	return ret_node;
}

channel_node_t* channel_mgr_get_node_and_del_from_list(channel_mgr_t *mgr, int id)
{
	channel_node_t *ret_node = NULL;
	if (mgr == NULL || mgr->sem == NULL) return NULL;

	semaphore_wait(mgr->sem);
	struct list_head *pos = NULL;
	list_for_each(pos, &mgr->head) {
		channel_node_t *node = list_entry(pos, channel_node_t, node);
		if (node->id == id) {
			list_del(pos);
			ret_node = node;
			break;
		}
	}
	semaphore_post(mgr->sem);

	return ret_node;
}

int channel_mgr_get_id(channel_mgr_t *mgr)
{
	int id = 0;
	if (mgr == NULL || mgr->sem == NULL) return id;

	semaphore_wait(mgr->sem);
	id = mgr->id_next++;
	if (mgr->id_next < 0) mgr->id_next = 1;
	semaphore_post(mgr->sem);

	return id;
}

int channel_mgr_send_data_to_sdk(channel_mgr_t * mgr, int fd, unsigned char * data, int datalen, int *id)
{

	int ret = RET_ERROR;
	channel_node_t *node = (channel_node_t *)malloc(sizeof(channel_node_t));

	node->id = channel_mgr_get_id(mgr);
	node->fd = fd;
	channel_mgr_add_node(mgr, node);
	
	*id = node->id;
	return callback_to_java_channel(node->id, data, datalen);
}

int channel_mgr_pipe_wirte(int fd, unsigned char *data, int datalen)
{
	 pipe_write(fd, data, datalen);


	return RET_OK;
}

int channel_mgr_send_data_to_server(channel_mgr_t * mgr, int id, unsigned char * data, int datalen)
{
	int ret = RET_ERROR;

	channel_node_t *node = channel_mgr_get_node_and_del_from_list(mgr, id);
	if (node == NULL) return RET_ERROR;

	if (RET_OK == channel_mgr_pipe_wirte(node->fd, data, datalen)) {
		ret = RET_OK;
	}

	free(node);

	return ret;
}

