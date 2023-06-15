#pragma once

#include "zxlist.h"
#include "zxstruct.h"
#include "zxstring.h"

typedef struct _data_queue_node_t data_queue_node_t;
typedef struct _data_queue_t data_queue_t;


struct _data_queue_node_t
{
	struct list_head node;
	unsigned short len;
	unsigned char type;
	unsigned char refer;
	unsigned char *data;
};

data_queue_node_t *data_queue_node_create(unsigned char type, unsigned char refer, unsigned char * data, unsigned int len);
void data_queue_node_release(data_queue_node_t *node);

data_queue_t *data_queue_create();
int data_queue_add_data(data_queue_t *data_queue, unsigned char type, unsigned char refer, unsigned char *data, unsigned int len);
void data_queue_add_data(data_queue_t *data_queue, data_queue_node_t *node);
void data_queue_del_data(data_queue_t * data_queue, unsigned char refer, unsigned char type);
data_queue_node_t *data_queue_get_first(data_queue_t *data_queue);
zxuint64 data_queue_total_size(data_queue_t *data_queue);
int data_queue_is_empty(data_queue_t *data_queue); // 0: empty, not 0: not empty
void data_queue_release(data_queue_t *data_queue);


