#include "fifoNode.h"
#include <stdlib.h>
#include <string.h>
#include "zxsocket.h"
#include "zxstruct.h"
#include "zxselect.h"
#include "zxlog.h"
#include "zxpipe.h"
#include <unistd.h>


static int fifo_node_set_del(fifo_node_t *node, int close_from_request, int close_from_response)
{
	node->snode->delnode = 1;
	node->closeRequest = close_from_request;
	node->closeResponse = close_from_response;
	return RET_OK;
}

void fifo_node_error_send(fifo_node_t *node, int type)
{
	if (node->closeRequest || node->closeResponse) return;

	node->transfer_data(node->mgr, type, node->refer, NULL, 0, 0);

	fifo_node_set_del(node, 0, 1);
}

static void fifo_call_back_read(void *param, int fd, unsigned char *buf, int bufsize)
{
	fifo_node_t *node = (fifo_node_t*)param;
	unsigned int nReadSize = bufsize - 8;
	int nRead = socket_read(node->fd[0], (char *)buf, nReadSize);

	string_add_bin(node->zxsResponseData, buf, nRead);
	
	if (RET_OK != node->transfer_data(node->mgr, TYPE_CHANNEL_TO_SERVER_BEGIN, node->refer, NULL, 0, 0)) {
		fifo_node_error_send(node, TYPE_ERROR);
		return;
	}
	if (RET_OK != node->transfer_data(node->mgr, TYPE_CHANNEL_TO_SERVER_ING, node->refer, (unsigned char *)string_get_data(node->zxsResponseData), *(int*)string_get_data(node->zxsResponseData), 0)) {
		fifo_node_error_send(node, TYPE_ERROR);
		return;
	}
	if (RET_OK != node->transfer_data(node->mgr, TYPE_CHANNEL_TO_SERVER_END, node->refer, NULL, 0, 0)) {
		fifo_node_error_send(node, TYPE_ERROR);
		return;
	}			
}

static void fifo_call_back_del(void *param, int fd)
{
	LOGD("fifo_call_back_del");
	fifo_node_t *node = (fifo_node_t*)param;
	node->remove_node(node->mgr, (node_t *)node);
}

static int call_bakc_get_mode(void *param, int fd)
{
	fifo_node_t *node = (fifo_node_t*)param;
	return node->snode->selectMode;
}

static select_node_t *fifo_select_node_create(fifo_node_t *node)
{
	select_node_t *snode = select_create_node();
	if (snode == NULL) return snode;

	snode->fd = node->fd[0];
	snode->connected = NULL;
	snode->read = fifo_call_back_read;
	snode->write = NULL;
	snode->timeout = NULL;
	snode->del = fifo_call_back_del;
	snode->get_mode = call_bakc_get_mode;
	snode->param = node;
	snode->selectMode |= SELECT_MODE_READ;
	snode->timeOut = 0;
	return snode;
}

static int recv_channel_to_sdk_begin(fifo_node_t *node, int type)
{
	select_node_t *snode = fifo_select_node_create(node);
	if (snode == NULL) {
		LOGE("fifo_select_node_create return null.");
		fifo_node_error_send(node, TYPE_ERROR);
		return RET_ERROR;
	} 
	node->snode = snode;
	select_fd_add(node->mgr->st, node->snode);
	return RET_OK;
}

static int recv_channel_to_sdk_ing(fifo_node_t *node, unsigned char *data, int datalen)
{
	if (STRING_OK != string_add_bin(node->zxsTransferData, data, datalen)) {
		LOGE("recv_channel_to_sdk_ing string_add_bin error");
		fifo_node_error_send(node, TYPE_ERROR);
		return RET_ERROR;
	}
	return RET_OK;
}

static int recv_channel_to_sdk_end(fifo_node_t *node)
{
	LOGD("recv_channel_to_sdk_end");
	int ret = RET_ERROR, id = 0;
	ret = channel_mgr_send_data_to_sdk(
			node->mgr->channel_mgr,
			node->fd[1],
			(unsigned char *)string_get_data(node->zxsTransferData),
			string_get_len(node->zxsTransferData), &id);
	if (RET_OK != ret) {
		fifo_node_error_send(node, TYPE_ERROR);
		return ret;
	}
	node->id = id;
	return ret;
}

static int recv_request_close(fifo_node_t *node)
{
	return fifo_node_set_del(node, 1, 0);
}

static int fifo_node_recv_package(node_t *_node, int type, int refer, unsigned char *data, int datalen)
{
	if (_node == NULL) return RET_ERROR;
	fifo_node_t *node = (fifo_node_t *)_node;

	switch (type)
	{
	case TYPE_CHANNEL_TO_SDK_BEGIN:	recv_channel_to_sdk_begin(node, type); break;
	case TYPE_CHANNEL_TO_SDK_ING:	recv_channel_to_sdk_ing(node, data, datalen); break;
	case TYPE_CHANNEL_TO_SDK_END:	recv_channel_to_sdk_end(node); break;
	case TYPE_CONNECT_CLOSE:		recv_request_close(node); break;
	default:						fifo_node_error_send(node, TYPE_ERROR_IN_FIFO); break;
	}

	return RET_OK;
}

int fifo_node_release(node_t *_node)
{
	if (_node == NULL) return RET_ERROR;
	fifo_node_t *node = (fifo_node_t *)_node;

	if (node->id != 0) {
		channel_mgr_del_node(node->mgr->channel_mgr, node->id);
	}

	pipe_close(node->fd);
	if (node->zxsResponseData != NULL) string_release(node->zxsResponseData);
	if (node->zxsTransferData != NULL) string_release(node->zxsTransferData);
	if (node->snode != NULL) select_node_release(node->snode);
	free(node);
	return RET_OK;
}

node_t *fifo_node_create(int refer, vps_mgr_t *mgr, p_func_call_back_get_flow_limit get_flow_limit,
	p_func_call_back_transfer_data transfer_data, p_func_call_back_delete_data delete_data, p_func_call_back_remove_node remove_node)
{
	fifo_node_t *node = (fifo_node_t *)malloc(sizeof(fifo_node_t));
	if (node == NULL) return NULL;

	memset(node, 0, sizeof(fifo_node_t));
	node->refer = refer;
	node->mgr = mgr;
	node->get_flow_limit = get_flow_limit;
	node->transfer_data = transfer_data;
	node->delete_data = delete_data;
	node->remove_node = remove_node;
	node->recv_package = fifo_node_recv_package;
	node->release = fifo_node_release;
	node->zxsTransferData = string_create();
	node->zxsResponseData = string_create();
	if (node->zxsResponseData == NULL || node->zxsTransferData == NULL) {
		fifo_node_release((node_t *)node);
		return NULL;
	}
	if (RET_OK != pipe_create(node->fd)) {
		fifo_node_release((node_t *)node);
		return NULL;
	}
	return (node_t *)node;
}