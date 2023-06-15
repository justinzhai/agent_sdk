#include "DataQueue.h"
#include <stdlib.h>
#include "system.h"

struct _data_queue_t
{
	struct list_head head;
	zxuint64 totalSize;
};


data_queue_node_t * data_queue_node_create(unsigned char type, unsigned char refer, unsigned char * data, unsigned int len)
{
	data_queue_node_t *node = (data_queue_node_t *)malloc(sizeof(data_queue_node_t));
	if (NULL == node) {
		return NULL;
	}

	node->type = type;
	node->refer = refer;
	node->data = data;
	node->len = len;
	
	return node;
}

void data_queue_node_release(data_queue_node_t * node)
{
	if (node->data) {
		free(node->data);
	}
	free(node);
}

data_queue_t * data_queue_create()
{
	data_queue_t *data_queue = (data_queue_t *)malloc(sizeof(data_queue_t));
	if (NULL == data_queue) {
		return NULL;
	}

	init_list_head(&data_queue->head);
	data_queue->totalSize = 0;

	return data_queue;
}

int data_queue_add_data(data_queue_t * data_queue, unsigned char type, unsigned char refer, unsigned char * data, unsigned int len)
{
	data_queue_node_t *node = data_queue_node_create(type, refer, data, len);
	if (NULL == node) {
		return RET_ERROR;
	}
	
	data_queue_add_data(data_queue, node);

	return RET_OK;
}

void data_queue_add_data(data_queue_t * data_queue, data_queue_node_t * node)
{
	list_add_tail(&node->node, &data_queue->head);
	data_queue->totalSize += node->len;
}

void data_queue_del_data(data_queue_t * data_queue, unsigned char refer, unsigned char type)
{
	list_head *pos = NULL;
	data_queue_node_t *tmp = NULL;

	list_for_each(pos, &data_queue->head) {
		tmp = list_entry(pos, data_queue_node_t, node);
		if (tmp->refer == refer && tmp->type == type) {
			list_del(pos);
			pos = pos->prev;
			data_queue->totalSize -= tmp->len;
			data_queue_node_release(tmp);
		}
	}
}

data_queue_node_t * data_queue_get_first(data_queue_t * data_queue)
{
	if (list_is_empty(&data_queue->head)) {
		return NULL;
	}

	data_queue_node_t *node = list_first_entry(&data_queue->head, data_queue_node_t, node);
	list_del(&node->node);
	data_queue->totalSize -= node->len;

	return node;
}

zxuint64 data_queue_total_size(data_queue_t * data_queue)
{
	return data_queue->totalSize;
}

int data_queue_is_empty(data_queue_t *data_queue)
{
	if (list_is_empty(&data_queue->head)) {
		return 0;
	}
	return 1;
}

void data_queue_release(data_queue_t * data_queue)
{
	list_head *pos = NULL;
	data_queue_node_t *tmp = NULL;

	list_for_each(pos, &data_queue->head) {
		 tmp = list_entry(pos, data_queue_node_t, node);
		 list_del(pos);
		 pos = pos->prev;
		 data_queue_node_release(tmp);
	}
	
	free(data_queue);
}
