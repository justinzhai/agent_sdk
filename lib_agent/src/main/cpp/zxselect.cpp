#include "zxselect.h"
#include "zxlog.h"
#include "semx.h"
#include "zxstruct.h"
#include <sys/select.h>
#include <errno.h>


#define SELECT_TIME_OUT_USEC 100000		//select 一次等待的微秒时间，100000μs = 100ms = 0.1秒
#define SELECT_TIME_OUT_SEC 0			//select 一次等待的秒时间
#define SELECT_TIME_OUT_TOTAL_SEC 45	//一个socket select 超时时间

struct _select_t
{
	int isThreadExit;
	struct list_head listHeadFd;
	unsigned char *buf;
	unsigned int nBufSize;
	semx_t *sem;
};

select_node_t * select_create_node()
{
	select_node_t *node = (select_node_t*)malloc(sizeof(select_node_t));
	memset(node, 0, sizeof(select_node_t));
	return node;
}

void select_node_release(select_node_t *node)
{
	free(node);
}

int select_fd_add(select_t *st, select_node_t *node)
{
	//LOGI("select_fd_add");
	node->timeAdd = time(NULL);
	list_add_tail(&node->node, &st->listHeadFd);
	return RET_OK;
}

void select_fill_fd(select_t *st, int &fd_max, fd_set &fdSetR, int &fdRead, fd_set &fdSetW, int &fdWrite)
{
	fdRead = fdWrite = 0;
	struct list_head *pos = NULL;
	
	list_for_each(pos, &st->listHeadFd) {
		select_node_t *snode = list_entry(pos, select_node_t, node);
		int mode = snode->get_mode(snode->param, snode->fd);
		bool isSet = false;
		if (mode & SELECT_MODE_READ) {
			FD_SET(snode->fd, &fdSetR);
			fdRead++;
			isSet = true;
		} 
		if (mode & SELECT_MODE_WRITE || mode & SELECT_MODE_CONNECT) {
			FD_SET(snode->fd, &fdSetW);
			fdWrite++;
			isSet = true;
		}
		if (isSet) 
			fd_max = (fd_max < snode->fd) ? snode->fd : fd_max;
		//LOGI("thread = %d FD_SET fd = %d mode = %s", gettid(), node->fd, mode == SELECT_MODE_READ ? "SELECT_MODE_READ" : "SELECT_MODE_WRITE");
	}
}

void select_callback(select_t *st, fd_set *fdSetR, fd_set *fdSetW)
{
	struct list_head *pos = NULL;
	list_for_each(pos, &st->listHeadFd) {
		// 第一个结点是sdk长连接结点，先跳过
		if (st->listHeadFd.next == pos) {
			continue;
		}
		select_node_t *snode = list_entry(pos, select_node_t, node);
		if (FD_ISSET(snode->fd, fdSetW)) {
			if (snode->selectMode & SELECT_MODE_CONNECT) {
				if (snode->connected != NULL) {
					snode->timeAdd = time(NULL);
					snode->connected(snode->param, snode->fd);
				}
			} else if (snode->selectMode & SELECT_MODE_WRITE){
				if (snode->write != NULL) {
					snode->timeAdd = time(NULL);
					snode->write(snode->param, snode->fd);
				}
			}
		}
		if (FD_ISSET(snode->fd, fdSetR)) {
			if (snode->selectMode & SELECT_MODE_READ) {
				if (snode->read != NULL) {
					snode->timeAdd = time(NULL);
					snode->read(snode->param, snode->fd, st->buf, st->nBufSize);
				}
			}
		}
	}

	// 单独处理第一个结点，最后处理
	select_node_t *snode = list_first_entry(&st->listHeadFd, select_node_t, node);
	if (FD_ISSET(snode->fd, fdSetW)) {
		if (snode->selectMode & SELECT_MODE_CONNECT) {
			if (snode->connected != NULL) {
				snode->timeAdd = time(NULL);
				snode->connected(snode->param, snode->fd);
			}
		}
		else if (snode->selectMode & SELECT_MODE_WRITE) {
			if (snode->write != NULL) {
				snode->timeAdd = time(NULL);
				snode->write(snode->param, snode->fd);
			}
		}
	}
	if (FD_ISSET(snode->fd, fdSetR)) {
		if (snode->selectMode & SELECT_MODE_READ) {
			if (snode->read != NULL) {
				snode->timeAdd = time(NULL);
				snode->read(snode->param, snode->fd, st->buf, st->nBufSize);
			}
		}
	}
}

void select_list_clear(select_t *st)
{
	struct list_head *pos = NULL, *tmp = NULL;
	
	list_for_each(pos, &st->listHeadFd) {
		select_node_t *node = list_entry(pos, select_node_t, node);
		if (node->delnode == 1) {
			list_del(pos);
			pos = pos->prev;	//不使用前指针，del后pos->next为野指针
			if (node->del) {
				node->del(node->param, node->fd);
			}
		}
	}
}

int _select_count(select_t *st)
{
	int count = 0;
	struct list_head *pos = NULL;
	list_for_each(pos, &st->listHeadFd) {
		count++;
	}
	return count;
}
//
//int select_list_size()
//{
//	int count = 0;
//	struct list_head *pos = NULL;
//	sem_wait(&gSelect.semFd);
//	count = _select_count();
//	sem_post(&gSelect.semFd);
//	return count;
//}

int select_is_empty(select_t *st)
{
	int count = 0;
	struct list_head *pos = NULL;
	count = _select_count(st);
	return (0 == count) ? 1 : 0;
}

void select_run_once(select_t *st)
{
	//LOGD("select_run_once");
	int fdMax = -1, fdWrite = 0, fdRead = 0;
	fd_set fdSetR, fdSetW;

	FD_ZERO(&fdSetR);
	FD_ZERO(&fdSetW);
	select_fill_fd(st, fdMax, fdSetR, fdRead, fdSetW, fdWrite);
	//LOGD("select_run_once fdRead = %d fdWrite = %d", fdRead, fdWrite );
	
	timeval tv;
	tv.tv_sec = SELECT_TIME_OUT_SEC;
	tv.tv_usec = SELECT_TIME_OUT_USEC;
	if (fdMax != -1) {
		int ret = select(fdMax + 1, (fdRead == 0) ? NULL : &fdSetR, (fdWrite == 0) ? NULL : &fdSetW, NULL, &tv);
		if (ret > 0) {
			select_callback(st, &fdSetR, &fdSetW);
		} else if (0 == ret) {
			//LOGI("select timeout");
		} else {
			LOGE("select error %d", errno);
		}
	}
}

void select_check_timeout(select_t *st)
{
	struct list_head *pos = NULL, *tmp = NULL;
	time_t now = time(NULL);

	list_for_each(pos, &st->listHeadFd) {
		select_node_t *snode = list_entry(pos, select_node_t, node);
		if (snode->timeOut > 0 && now - snode->timeAdd > snode->timeOut) {
			snode->timeAdd = now;	//用于下次触发超时
			if (snode->timeout != NULL) snode->timeout(snode->param, snode->fd);
		}
	}
}

void select_prepare(select_t *st)
{
	struct list_head *pos = NULL;
	list_for_each(pos, &st->listHeadFd) {
		select_node_t *snode = list_entry(pos, select_node_t, node);
		if (snode->prepare) {
			snode->prepare(snode->param, snode->fd);
		}
	}
}

void select_thread(select_t *st, long long exit_timestamp){
	while (!st->isThreadExit) {
		if (select_is_empty(st)) {
			break;
		}
		zxuint64 t = time(NULL);
		if (t > g_time) {
			g_time = t;
		}
		select_prepare(st);
		select_run_once(st);
		select_check_timeout(st);
		select_list_clear(st);
		if (exit_timestamp != 0 && time(NULL) >= exit_timestamp) {
			break;
		}
	}
}

void select_thread_exit(select_t *st)
{
	st->isThreadExit = 1;
}

int select_init(select_t **_st)
{
	select_t *st = (select_t *)malloc(sizeof(select_t));
	if (st == NULL) 
		return RET_ERROR;
	memset(st, 0, sizeof(select_t));

	st->isThreadExit = 0;
	init_list_head(&st->listHeadFd);
	st->nBufSize = 64 * 1024;

	st->buf = (unsigned char *)malloc(st->nBufSize);
	if (st->buf == NULL) {
		goto clean;
	}

	st->sem = semaphore_create();
	if (st->sem == NULL) {
		goto clean;
	}

	*_st = st;
	return RET_OK;

clean:
	select_release(st);
	return RET_ERROR;
}

int select_release(select_t *st)
{
	if (st != NULL) {
		if (st->sem) semphore_close(st->sem);
		if (st->buf) free(st->buf);
		free(st);
	}
	
	return RET_OK;
}