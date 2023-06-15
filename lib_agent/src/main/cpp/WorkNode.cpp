#include "WorkNode.h"
#include <stdlib.h>
#include <string.h>
#include "zxsocket.h"
#include "zxstruct.h"
#include "zxselect.h"
#include "zxlog.h"
//#include <stdio.h>

extern void callback_to_java_flow_sync(int sendBytes, int recvBytes);

static void call_back_timeout(void *param, int fd);
static void call_back_write(void *param, int fd);
static void call_back_read(void *param, int fd, unsigned char *buf, int bufsize);
static void call_back_connected(void *param, int fd);
static void call_back_del(void *param, int fd);
static int call_bakc_get_mode(void *param, int fd);
int http_get_host_port(const char *szUrl, zx_string *_strHost, int *port);
int work_node_release(node_t *node);
static int work_node_recv_package(node_t *node, int type, int refer, unsigned char *data, int datalen);

static int WORK_NODE_CONNECT_TIMEOUT	= 15;	// 连接超时
static int WORK_NODE_WORK_TIMEOUT		= 0;	// 读写超时, 0为关闭
static int WORK_NODE_WORK_TIMEOUT_FOR_ASYNC_RESP	= 10;	// 读写超时, 0为关闭, 用于response不回传模式

void work_node_set_config(int timeout_connect, int timeout_total)
{
	WORK_NODE_CONNECT_TIMEOUT	= timeout_connect;
	WORK_NODE_WORK_TIMEOUT		= timeout_total;
}

static select_node_t *work_select_node_create(work_node_t *node) 
{
	select_node_t *snode = select_create_node();
	if (snode == NULL) return snode;

	snode->fd			= node->fd;
	snode->connected	= call_back_connected;
	snode->read			= call_back_read;
	snode->write		= call_back_write;
	snode->timeout		= call_back_timeout;
	snode->del			= call_back_del;
	snode->get_mode = call_bakc_get_mode;
	snode->param		= node;
	snode->selectMode	|= SELECT_MODE_CONNECT;
	snode->timeOut		= WORK_NODE_CONNECT_TIMEOUT;
	return snode;
}

node_t *work_node_create(int refer, vps_mgr_t *mgr, p_func_call_back_get_flow_limit get_flow_limit,
	p_func_call_back_transfer_data transfer_data, p_func_call_back_delete_data delete_data, p_func_call_back_remove_node remove_node)
{
	work_node_t *node = (work_node_t *)malloc(sizeof(work_node_t));
	if (node == NULL) return NULL;

	memset(node, 0, sizeof(work_node_t));
	node->refer = refer;
	node->mgr = mgr;
	node->get_flow_limit = get_flow_limit;
	node->transfer_data = transfer_data;
	node->delete_data = delete_data;
	node->remove_node = remove_node;
	node->recv_package = work_node_recv_package;
	node->release = work_node_release;
	node->flow_statistics = create_flow_statistics(1);
	if (NULL == node->flow_statistics) {
		work_node_release((node_t *)node);
		return NULL;
	}
	node->zxsTransferData = string_create();
	node->zxsResponseData = string_create();
	if (node->zxsResponseData == NULL || node->zxsTransferData == NULL) {
		work_node_release((node_t *)node);
		return NULL;
	}
	node->sync_resp = 1;
	node->sendBytys = node->recvBytes = 0;
	node->lastModeWithRead = time(NULL);
	return (node_t *)node;
}

int work_node_release(node_t *_node)
{
	if (_node == NULL) return RET_ERROR;
	work_node_t *node = (work_node_t *)_node;
	if (node->fd != 0) socket_close(node->fd);
	if (node->zxsResponseData != NULL) string_release(node->zxsResponseData);
	if (node->zxsTransferData != NULL) string_release(node->zxsTransferData);
	if (node->snode != NULL) select_node_release(node->snode);
	if (node->flow_statistics != NULL) flow_statistics_release(node->flow_statistics);
	free(node);
	return RET_OK;
}

void work_node_send_close_package(work_node_t *node)
{
	if (node->closeRequest) {
		node->delete_data(node->mgr, TYPE_TRANS_DATA, node->refer);
		return;
	}

	if (node->snode->selectMode & SELECT_MODE_CONNECT) {
		node->transfer_data(node->mgr, TYPE_ERROR, node->refer, NULL, 0, 0);
	} else {
		if (node->sync_resp == 0) {
			node->transfer_data(node->mgr, TYPE_RESP_SIZE_SYNC, node->refer, (unsigned char *)&node->totalResponseSize, sizeof(node->totalResponseSize), 0);
		}
		node->transfer_data(node->mgr, TYPE_CONNECT_CLOSE, node->refer, NULL, 0, 0);
	}
}

void work_node_set_select_node_del(work_node_t *node)
{
	node->snode->delnode = 1;
}

int recv_request_connect(work_node_t *node, unsigned char *data, int datalen)
{
	if (RET_OK != socket_create_nonblock(&node->fd)) {
		node->transfer_data(node->mgr, TYPE_ERROR, node->refer, NULL, 0, 0);
		return RET_ERROR;
	}
	select_node_t *snode = work_select_node_create(node);
	if (snode == NULL) {
		node->transfer_data(node->mgr, TYPE_ERROR, node->refer, NULL, 0, 0);
		return RET_ERROR;
	}
	node->snode = snode;
	zx_string *zxHost = string_create();
	zx_string *zxRecvData = string_create_bin(data, datalen);
	int port = 0;
	if (zxHost == NULL || zxRecvData == NULL) {
		node->transfer_data(node->mgr, TYPE_ERROR, node->refer, NULL, 0, 0);
		return RET_ERROR;
	}

	if (RET_OK != http_get_host_port(string_get_data(zxRecvData), zxHost, &port)) {
		node->transfer_data(node->mgr, TYPE_ERROR, node->refer, NULL, 0, 0);
		string_release(zxHost);
		string_release(zxRecvData);
		return RET_ERROR;
	}

	if (RET_OK != socket_connect(node->refer, node->fd, string_get_data(zxHost), port)) {
		node->transfer_data(node->mgr, TYPE_ERROR, node->refer, NULL, 0, 0);
		string_release(zxHost);
		string_release(zxRecvData);
		return RET_ERROR;
	}
	
	select_fd_add(node->mgr->st, node->snode);
	string_release(zxHost);
	string_release(zxRecvData);
	return RET_OK;
}

int recv_request_transfer(work_node_t *node, unsigned char *data, int datalen)
{
	if (datalen == 0 || data == NULL) return RET_OK;

	add_write_bytes(node->flow_statistics, time(NULL), datalen);

	if (STRING_ERROR == string_add_bin(node->zxsTransferData, data, datalen)) {
		work_node_set_select_node_del(node);
		return RET_OK;
	}

	node->snode->selectMode |= SELECT_MODE_WRITE;
	
	return RET_OK;
}

int recv_request_close(work_node_t *node, unsigned char *data, int datalen)
{
	node->snode->delnode = 1;
	node->closeRequest = 1;
	return RET_OK;
}

int recv_request_async_response_opt_set(work_node_t *node)
{
	node->sync_resp = 0;
	if (node->snode->selectMode & SELECT_MODE_READ)
		node->snode->timeOut = WORK_NODE_WORK_TIMEOUT_FOR_ASYNC_RESP;
	return RET_OK;
}

static int work_node_recv_package(node_t *_node, int type, int refer, unsigned char *data, int datalen)
{
	if (_node == NULL) return RET_ERROR;
	work_node_t *node = (work_node_t *)_node;

	if (type == TYPE_CONNECT) {
		return recv_request_connect(node, data, datalen);
	} else if (type == TYPE_TRANS_DATA) {
		return recv_request_transfer(node, data, datalen);
	} else if (type == TYPE_CONNECT_CLOSE) {
		return recv_request_close(node, data, datalen);
	} else if (type == TYPE_OPT_NO_RESP) {
		return recv_request_async_response_opt_set(node);
	} else {
		//TODO LOG warning
	}
	return RET_OK;
}

static void call_back_connected(void *param, int fd)
{
	work_node_t *node = (work_node_t*)param;
	node->snode->selectMode &= ~SELECT_MODE_CONNECT;
	node->snode->selectMode |= SELECT_MODE_READ;
	if (node->sync_resp == 1)
		node->snode->timeOut = WORK_NODE_WORK_TIMEOUT;
	else
		node->snode->timeOut = WORK_NODE_WORK_TIMEOUT_FOR_ASYNC_RESP;

	if (RET_OK != node->transfer_data(node->mgr, TYPE_OK, node->refer, NULL, 0, 1)) {
		work_node_set_select_node_del(node);
	}
}

static void call_back_read(void *param, int fd, unsigned char *buf, int bufsize)
{
	work_node_t *node = (work_node_t*)param;
	
	const unsigned int static READ_SIZE_OF_ONE_TIME = 3900;
	unsigned int nReadSize = bufsize - 1000 > READ_SIZE_OF_ONE_TIME ? READ_SIZE_OF_ONE_TIME : bufsize - 1000;

	int nRead = socket_read(node->fd, (char *)buf, nReadSize);

	node->recvBytes += nRead;
	add_read_bytes(node->flow_statistics, g_time, nRead);

	if (node->sync_resp == 1) {
		if (RET_OK != node->transfer_data(node->mgr, TYPE_TRANS_DATA, node->refer, buf, nRead, 0)) {
			work_node_set_select_node_del(node);
			return;
		}
	} else {
		node->totalResponseSize += nRead;
	}
}

static void call_back_write(void *param, int fd)
{
	work_node_t *node = (work_node_t*)param;

	unsigned int nRestSendSize = string_get_len(node->zxsTransferData) - node->offsetTransferData;
	if (nRestSendSize > 0) {
		int nWrite = socket_write(node->fd, string_get_data(node->zxsTransferData) + node->offsetTransferData, nRestSendSize);
		
		node->offsetTransferData += nWrite;
		node->sendBytys += nWrite;
	}
}

static void call_back_timeout(void *param, int fd)
{
	work_node_t *node = (work_node_t*)param;
	work_node_set_select_node_del(node);
}

static void call_back_del(void *param, int fd)
{
	work_node_t *node = (work_node_t*)param;
	work_node_send_close_package(node);
#ifdef ANDROID
	//callback_to_java_flow_sync(node->sendBytys, node->recvBytes);
#endif // DEBUG
	node->remove_node(node->mgr, (node_t *)node);
}

static int call_bakc_get_mode(void *param, int fd)
{
	int mode;
	work_node_t *node = (work_node_t*)param;
	
	const unsigned int static CACHE_LIMIT = 1 * 256 * 1024;

	long long flow_cache = node->get_flow_limit(node->mgr);
	if (flow_cache <= CACHE_LIMIT) {
		node->lastModeWithRead = time(NULL);
		return node->snode->selectMode;
	}

	zxint64 now = time(NULL);
	if (now > node->lastModeWithRead + 2) {
		node->lastModeWithRead = time(NULL);
		node->read2k = 1;
		return node->snode->selectMode;
	}

	return node->snode->selectMode & ~SELECT_MODE_READ;
}

int http_get_host_port(const char *szUrl, zx_string *_strHost, int *port)
{
	int ret = RET_ERROR;
	unsigned int pos = 0, pos1 = 0;
	zx_string *zxsUrl = NULL, *strHostAndPort = NULL, *zxsHost = NULL, *zxsPort = NULL, *zxsProtocol = NULL;
	if (NULL == szUrl || NULL == _strHost || NULL == port) {
		return RET_ERROR;
	}

	*port = 80; //设置http默认端口号

	if (NULL == (zxsUrl = string_create_chars(szUrl))) {
		//LOGE("http_get_host_port  string_create_chars %s", szUrl);
		goto clean;
	}

	if ((ret = string_tolower(zxsUrl)) != STRING_OK) {
		//LOGE("http_get_host_port  string_tolower %s", szUrl);
		ret = RET_ERROR;
		goto clean;
	}

	pos = string_find_chars(zxsUrl, "://");
	if (pos != STRING_NPOS) {
		pos1 = string_find_chars(zxsUrl, "/", pos + 3);
		if (pos1 == STRING_NPOS) {
			pos1 = string_get_len(zxsUrl);
		}
	} else {
		pos1 = string_find_chars(zxsUrl, "/");
		if (pos1 == STRING_NPOS) {
			pos1 = string_get_len(zxsUrl);
		}
	}
	if (pos != STRING_NPOS) {
		if (NULL == (zxsProtocol = string_substr_begin_cnt(zxsUrl, 0, pos))) {
			//LOGE("http_get_host_port string_substr_befor %s %s", string_get_data(zxsUrl), "://");
			goto clean;
		}
		if (string_find_chars(zxsProtocol, "https") != STRING_NPOS) {
			*port = 443;
			//LOGE("http_get_host_port string_find_chars %s %s", string_get_data(zxsProtocol), URL_PROTOCOL_STR_HTTP);
		}
	}

	if (NULL == (strHostAndPort = string_substr_begin_cnt(zxsUrl, pos + 3, pos1 - pos - 3))) {
		goto clean;
	}
	if (string_get_len(strHostAndPort) == 0) {
		goto clean;
	}
	//取端口号
	if ((pos = string_find_chars(strHostAndPort, ":")) != STRING_NPOS) {
		zxsHost = string_substr_begin_cnt(strHostAndPort, 0, pos);
		zxsPort = string_substr_begin(strHostAndPort, pos + 1);
		if (NULL == zxsHost || NULL == zxsPort) {
		//	LOGE("http_get_host_port  string_tolower %s", szUrl);
			goto clean;
		}

		if (RET_OK != string_to_uint(zxsPort, (unsigned int*)port)) {
		//	LOGE("http_get_host_port string_to_uint error str = %s", string_get_data(zxsPort));
			goto clean;
		}

		string_swap(_strHost, zxsHost);
	} else {
		if (STRING_OK != string_add_chars(_strHost, string_get_data(strHostAndPort))) {
		//	LOGE("http_get_host_port string_add_chars %s", string_get_data(strHostAndPort));
			goto clean;
		}
	}

	if (0 == string_get_len(_strHost)) {
		goto clean;
	}

	//LOGD("http_get_host_port host = %s port = %d", string_get_data(_strHost), *port);
	ret = RET_OK;

clean:
	if (zxsUrl) string_release(zxsUrl);
	if (strHostAndPort) string_release(strHostAndPort);
	if (zxsHost) string_release(zxsHost);
	if (zxsPort) string_release(zxsPort);
	if (zxsProtocol) string_release(zxsProtocol);
	return ret;
}