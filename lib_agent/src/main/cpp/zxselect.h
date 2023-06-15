
#ifndef __ZXSELECT__
#define __ZXSELECT__

#include "zxlist.h"
#include <time.h>
//#include <sys/types.h>

#define SELECT_MODE_CONNECT	0x1
#define SELECT_MODE_READ	0x2
#define SELECT_MODE_WRITE	0x4
#define SELECT_MODE_ERROR	0x8
//#define SELECT_MODE_WAIT	0x10

typedef void (*p_func_call_back_connected)(void *param, int fd);
typedef void (*p_func_call_back_read)(void *param, int fd, unsigned char *buf, int bufsize);
typedef void (*p_func_call_back_write)(void *param, int fd);
typedef void (*p_func_call_back_timeout)(void *param, int fd);
typedef void (*p_func_call_back_del)(void *param, int fd);
typedef int(*p_func_call_back_get_mode)(void *param, int fd);
typedef void(*p_func_call_back_prepare)(void *param, int fd);
typedef struct _select_node_t
{
	struct list_head node;
	void *param;
	p_func_call_back_connected connected;
	p_func_call_back_read read;
	p_func_call_back_write write;
	p_func_call_back_timeout timeout;
	p_func_call_back_del del;
	p_func_call_back_get_mode get_mode;
	p_func_call_back_prepare prepare;
	int fd;
	int selectMode;
	time_t timeAdd;
	long timeOut;
	int delnode;
}select_node_t;

typedef struct _select_t select_t;

int select_init(select_t **st);
int select_release(select_t *st);
int select_fd_add(select_t *st, select_node_t *node);
select_node_t * select_create_node();
void select_node_release(select_node_t *node);
void select_thread(select_t *st, long long exit_timestamp);
void select_thread_exit(select_t *st);


#endif	//__ZXSELECT__
