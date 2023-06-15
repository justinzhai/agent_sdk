#include "VpsMgr.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "zxlog.h"
#include "zxsocket.h"
#include "zxstruct.h"
#include "WorkNode.h"
#include "fifoNode.h"
#include <errno.h>


static void call_back_timeout(void *param, int fd);
static void call_back_write(void *param, int fd);
static void call_back_read(void *param, int fd, unsigned char *buf, int bufsize);
static void call_back_connected(void *param, int fd);
void package_splite(vps_mgr_t *mgr);
int transfer_data(vps_mgr_t *mgr, int type, int refer, unsigned char *data, int datalen);
int delete_data(vps_mgr_t *mgr, int type, int refer);
node_t *get_node(vps_mgr_t *mgr, int refer);
zx_string *proto_package_build(vps_mgr_t *mgr, int type, int refer, unsigned char *data, int datalen);
int flow_limit_calc(vps_mgr_t *mgr, unsigned int flow_last, unsigned int flow_cache);
int flow_limit_calc_not_limit(vps_mgr_t *mgr, unsigned int flow_last, unsigned int flow_cache);

static int VPS_MGR_CONNECT_TIMEOUT	= 15;	//连接超时
//#define VPS_MGR_HEART_ALTERNATION	300		//5分钟间隔心跳包
#define VPS_MGR_HEART_TIMEOUT		60		//60秒心跳超时
#define VPS_MGR_TOTAL_IDLE			7		
static long long g_exit_timestamp = 0;

static vps_mgr_t *g_mgr = NULL;

void vps_mgr_set_config(int timeout_connect)
{
	VPS_MGR_CONNECT_TIMEOUT = timeout_connect;
}

void vps_mgr_set_exit_timestamp(long long exit_timestamp)
{
	g_exit_timestamp = exit_timestamp;
}

void vps_mgr_set_bind_addr(const char *ip)
{
	glob_bind_addr_set(ip);
}

static void call_back_connected(void *param, int fd) 
{
	vps_mgr_t *mgr = (vps_mgr_t*)param;
	mgr->node->selectMode &= ~SELECT_MODE_CONNECT;
	mgr->node->selectMode |= SELECT_MODE_READ;

	mgr->node->timeOut = VPS_MGR_TOTAL_IDLE;
}

static void call_back_read(void *param, int fd, unsigned char *buf, int bufsize)
{
	vps_mgr_t *mgr = (vps_mgr_t*)param;
	int nRead = socket_read(mgr->fd, (char *)buf, bufsize);
	
	add_read_bytes(mgr->flow_statistics, time(NULL), nRead);

	string_add_bin(mgr->zxsRecvData, buf, nRead); 

	package_splite(mgr);
}

static void call_back_write_data_queue(void *param, int fd)
{
	vps_mgr_t *mgr = (vps_mgr_t*)param;
	
	unsigned int nRestSendSize = string_get_len(mgr->zxsSendData) - mgr->offsetSendData;
	if (nRestSendSize > 0) {
		int nWrite = socket_write(mgr->fd, string_get_data(mgr->zxsSendData) + mgr->offsetSendData, nRestSendSize);
		
		mgr->offsetSendData += nWrite;
		
		add_write_bytes(mgr->flow_statistics, g_time, nWrite);

		return;
	}

	do {
		data_queue_node_t *node = data_queue_get_first(mgr->data_queue);
		
		zx_string *tmp = proto_package_build(mgr, node->type, node->refer, node->data, node->len);

		data_queue_node_release(node);

		int nWrite = socket_write(mgr->fd, string_get_data(tmp), string_get_len(tmp));

		add_write_bytes(mgr->flow_statistics, g_time, nWrite);

		string_release(tmp);
		break;
	} while (true);

	if (data_queue_total_size(mgr->data_queue) <= 0 && data_queue_is_empty(mgr->data_queue) == 0 && string_get_len(mgr->zxsSendData) - mgr->offsetSendData <= 0) {
		mgr->node->selectMode &= ~SELECT_MODE_WRITE;
	}
}

static void call_back_write(void *param, int fd)
{
	vps_mgr_t *mgr = (vps_mgr_t*)param;
	if (string_get_len(mgr->zxsSendData) < mgr->offsetSendData) {
		mgr->node->selectMode &= ~SELECT_MODE_WRITE;
		return;
	}

	unsigned int nRestSendSize = string_get_len(mgr->zxsSendData) - mgr->offsetSendData;
	if (nRestSendSize > 0) {
		int nWrite = socket_write(mgr->fd, string_get_data(mgr->zxsSendData) + mgr->offsetSendData, nRestSendSize);
		
		mgr->offsetSendData += nWrite;
	
		add_write_bytes(mgr->flow_statistics, time(NULL), nWrite);
	}

	if (string_get_len(mgr->zxsSendData) <= mgr->offsetSendData) {
		mgr->node->selectMode &= ~SELECT_MODE_WRITE;
	}
}

static void call_back_timeout(void *param, int fd)
{
	LOGD("vps timeout");
	vps_mgr_t *mgr = (vps_mgr_t*)param;
	if (mgr->node->selectMode & SELECT_MODE_CONNECT) {
		LOGE("mgr connect time out %lds", mgr->node->timeOut);
		select_thread_exit(mgr->st);
		return;
	} else {
		if (mgr->heartbeatTimeout != 0) {
			if ((time(NULL) - mgr->heartbeatLastRecv) > mgr->heartbeatTimeout) {
				LOGE("vps total idle, idle: %lds, timeout: %ds, %ld", mgr->node->timeOut, mgr->heartbeatTimeout, mgr->node->timeAdd);
				select_thread_exit(mgr->st);
			}
		}
	}
}

static int call_back_get_mode(void *param, int fd)
{
	vps_mgr_t *mgr = (vps_mgr_t*)param;
	return mgr->node->selectMode;
}

static void call_back_prepare(void *param, int fd)
{
	//LOGD("vps prepare");
	//LOGI("vps prepare");
	vps_mgr_t *mgr = (vps_mgr_t*)param;
	// TODO: 计算带宽，计算缓冲区流量

	zxuint64 t = g_time;

	// 实时缓存更新
	mgr->flow_cache = data_queue_total_size(mgr->data_queue);

	if (t > mgr->time_last_print_flow) {
		mgr->time_last_print_flow = t;
		LOGE("                flow: %6d B/s, cache: %6d B, limit: %u B", int(get_write_bytes_with_statistical_interval(mgr->flow_statistics, g_time)), int((
			data_queue_total_size(mgr->data_queue))), mgr->flow_limit);
	}

	// 重新计算一次限制
	if (t > mgr->time_last_flow_limit_calc) {
		mgr->time_last_flow_limit_calc = t;
		//LOGE("%llu, %u", get_write_bytes_with_statistical_interval(mgr->flow_statistics) / 10, (data_queue_total_size(mgr->data_queue)));
		flow_limit_calc_not_limit(mgr, get_write_bytes_with_statistical_interval(mgr->flow_statistics, t), (data_queue_total_size(mgr->data_queue)));
	}




	//LOGD("cache: %dKB", );
}

int package_decode(vps_mgr_t *mgr, unsigned char *package, int len)
{
	return RET_OK;
}

int package_encode(vps_mgr_t *mgr, unsigned char *package, int len)
{
	return RET_OK;
}

int transfer_package(vps_mgr_t *mgr, int type, int refer, unsigned char *data, int datalen)
{
	if (datalen > 0 && RET_OK != package_encode(mgr, data, datalen)) {
		return RET_ERROR;
	}
		
	if (data != NULL && datalen != 0) {
		if (STRING_ERROR == string_add_bin(mgr->zxsSendData, data, datalen)) {
			return RET_ERROR;
		}
	}
	if (!(mgr->node->selectMode & SELECT_MODE_WRITE)) {
		mgr->node->selectMode |= SELECT_MODE_WRITE;
	}

	return RET_OK;
}

int transfer_data(vps_mgr_t *mgr, int type, int refer, unsigned char *data, int datalen)
{
	int nRestData = datalen;
	do {
		int nSendLen = nRestData > 0xFFFF ? 0xFFFF : nRestData;
		if (RET_OK != transfer_package(mgr, type, refer, data + datalen - nRestData, nSendLen)) {
			return RET_ERROR;
		}
		nRestData -= nSendLen;
	} while (nRestData);
	return RET_OK;
}

int transfer_data_from_node(vps_mgr_t *mgr, int type, int refer, unsigned char *data, int datalen, int is_cmd = 0)
{
	int ret = RET_OK;
	int nRestData = datalen;

	if (is_cmd) {
		return transfer_data(mgr, type, refer, data, datalen);
	}

	do {
		int nSendLen = nRestData > 0xFFFF ? 0xFFFF : nRestData;
		if (0 == nSendLen) {
			if (RET_OK != data_queue_add_data(mgr->data_queue, type, refer, NULL, nSendLen)) {
				ret = RET_ERROR;
				break;
			}
		}
		else {
			unsigned char *buf = (unsigned char *)malloc(sizeof(char) * nSendLen);
			if (NULL == buf) {
				ret = RET_ERROR;
				break;
			}
			memcpy(buf, data + datalen - nRestData, nSendLen);
			if (RET_OK != data_queue_add_data(mgr->data_queue, type, refer, buf, nSendLen)) {
				ret = RET_ERROR;
				break;
			}
		}

		nRestData -= nSendLen;
	} while (nRestData);

	if (!(mgr->node->selectMode & SELECT_MODE_WRITE)) {
		mgr->node->selectMode |= SELECT_MODE_WRITE;
	}

	return ret;
}

int delete_data(vps_mgr_t *mgr, int type, int refer)
{
	data_queue_del_data(mgr->data_queue, refer, type);
	return RET_OK;
}

unsigned int get_flow_limit(vps_mgr_t *mgr)
{
	// TODO: 暂时改成获取缓存大小的功能
	return mgr->flow_cache;
	//return mgr->flow_limit;
}

zx_string *proto_package_build(vps_mgr_t *mgr, int type, int refer, unsigned char *data, int datalen)
{
	zx_string *ret = NULL;

	if (datalen > 0 && RET_OK != package_encode(mgr, data, datalen)) {
		return NULL;
	}

	if (NULL == (ret = string_create())) {
		return NULL;
	}

	if (data != NULL && datalen != 0) {
		if (STRING_ERROR == string_add_bin(ret, data, datalen)) {
			string_release(ret);
			return NULL;
		}
	}

	return ret;
}

int remove_node(vps_mgr_t *mgr, node_t *wnode) 
{
	int refer = wnode->refer;
	list_del(&wnode->node);
	wnode->release(wnode);
	
	transfer_data_from_node(mgr, TYPE_REFER_DEL_SYNC, refer, NULL, 0);
	return RET_OK;
}

bool has_a_full_package(zx_string *zxsData, unsigned int offset, int *type, int *refer, unsigned char **package, int *pakcagelen) 
{
	return true;
}

void package_dispatch_heart(vps_mgr_t *mgr, int type, int refer, unsigned char *data, int datalen)
{
	LOGD("recv heart package");
	transfer_data(mgr, TYPE_HEART, 0, NULL, 0);
	mgr->heartbeatLastRecv = time(NULL);
}

void package_dispatch_ok(vps_mgr_t *mgr, int type, int refer, unsigned char *data, int datalen)
{

}

void package_dispatch_error(vps_mgr_t *mgr, int type, int refer, unsigned char *data, int datalen)
{

}

void package_dispatch_encode_enable(vps_mgr_t *mgr, int type, int refer, unsigned char *data, int datalen)
{
	mgr->encode = VPS_ENCODE_ENABLE;
	if (RET_OK != transfer_data(mgr, TYPE_OK, 0, NULL, 0)) {
		LOGW("package_dispatch_encode_enable ERROR");
		select_thread_exit(mgr->st);
	}
}

void package_dispatch_get_imsi(vps_mgr_t *mgr, int type, int refer)
{
	char *data = NULL;
	int datalen = 0;
	if (mgr->zxsIMSI != NULL) {
		data = string_get_data(mgr->zxsIMSI);
		datalen = string_get_len(mgr->zxsIMSI);
	}
	if (RET_OK != transfer_data(mgr, TYPE_OK, 0, (unsigned char *)data, datalen)) {
		LOGW("package_dispatch_get_imsi ERROR");
		select_thread_exit(mgr->st);
	}
}

void package_dispatch_get_version_name(vps_mgr_t *mgr, int type, int refer, unsigned char *data, int datalen)
{
	char ver_name[] = { VPS_SO_VERSION_NAME };
	if (RET_OK != transfer_data(mgr, TYPE_OK, 0, (unsigned char *)ver_name, strlen(ver_name))) {
		LOGW("package_dispatch_get_version_name ERROR");
		select_thread_exit(mgr->st);
	}
}

void package_dispatch_get_version_code(vps_mgr_t *mgr, int type, int refer, unsigned char *data, int datalen)
{
	char sz_ver_code[12] = { 0 };
	sprintf(sz_ver_code, "%d", VPS_SO_VERSION_CODE);
	if (RET_OK != transfer_data(mgr, TYPE_OK, 0, (unsigned char *)sz_ver_code, strlen(sz_ver_code))) {
		LOGW("package_dispatch_get_version_code ERROR");
		select_thread_exit(mgr->st);
	}
}

void package_dispatch_set_vps_total_idle(vps_mgr_t *mgr, int type, int refer, unsigned char *data, int datalen)
{
	LOGD("recv type TYPE_SET_VPS_TOTAL_IDLE");
	unsigned int totalIdle = 0;
	zx_string *zxsTotalIdle = string_create_bin(data, datalen);
	if (NULL == zxsTotalIdle || STRING_OK != string_to_uint(zxsTotalIdle, &totalIdle)) {
		transfer_data(mgr, TYPE_ERROR, 0, NULL, 0);
	}
	else {
		LOGI("set total idle %ds.", totalIdle);
		mgr->node->timeOut = totalIdle;
		transfer_data(mgr, TYPE_OK, 0, NULL, 0);
	}
}

void package_dispatch_set_heartbeat_timeout(vps_mgr_t *mgr, int type, int refer, unsigned char *data, int datalen)
{
	LOGD("recv type TYPE_SET_HEARTBEAT_TIMEOUT");
	unsigned int heartbeatTimeOut = 0;
	zx_string *zxsHeartbeatTimeOut = string_create_bin(data, datalen);
	if (NULL == zxsHeartbeatTimeOut || STRING_OK != string_to_uint(zxsHeartbeatTimeOut, &heartbeatTimeOut)) {
		transfer_data(mgr, TYPE_ERROR, 0, NULL, 0);
	}
	else {
		LOGI("set heartbeat timeout %ds.", heartbeatTimeOut);
		mgr->heartbeatTimeout = heartbeatTimeOut;
		transfer_data(mgr, TYPE_OK, 0, NULL, 0);
	}
}

void package_dispatch_connect(vps_mgr_t *mgr, int type, int refer, unsigned char *data, int datalen)
{
	node_t *wnode = work_node_create(refer, mgr, get_flow_limit, transfer_data_from_node, delete_data, remove_node);
	
	if (wnode == NULL) {
		transfer_data(mgr, TYPE_ERROR, refer, NULL, 0);
		return;
	}
	if (RET_OK != wnode->recv_package(wnode, type, refer, data, datalen)) {
		wnode->release(wnode);
		return;
	}
	list_add_tail(&wnode->node, &mgr->workNodeHeader);
}

void package_dispatch_transfer_data(vps_mgr_t *mgr, int type, int refer, unsigned char *data, int datalen)
{
	node_t *node = get_node(mgr, refer);
	if (node != NULL) {
		node->recv_package(node, type, refer, data, datalen);
	}
}

void package_dispatch_channel_to_sdk_begin(vps_mgr_t *mgr, int type, int refer, unsigned char *data, int datalen)
{
	LOGD("recv type TYPE_CHANNEL_TO_SDK_BEGIN");
	node_t *node = fifo_node_create(refer, mgr, get_flow_limit, transfer_data_from_node, delete_data, remove_node);
	//node_t *node = fifo_node_create(refer, mgr, transfer_data, remove_node);
	if (node == NULL) {
		transfer_data(mgr, TYPE_ERROR, refer, NULL, 0);
		return;
	}
	LOGD("add fifo node to list");
	list_add_tail(&node->node, &mgr->workNodeHeader);
	node->recv_package(node, type, refer, data, datalen);
}

void package_dispatch_channel_to_sdk_ing(vps_mgr_t *mgr, int type, int refer, unsigned char *data, int datalen)
{
	LOGD("recv type TYPE_CHANNEL_TO_SDK_ING");
	node_t *node = get_node(mgr, refer);
	if (node != NULL) {
		node->recv_package(node, type, refer, data, datalen);
	}
	if (node == NULL) { LOGE("not find fifo node"); }
}

void package_dispatch_channel_to_sdk_end(vps_mgr_t *mgr, int type, int refer, unsigned char *data, int datalen)
{
	LOGD("recv type TYPE_CHANNEL_TO_SDK_END");
	node_t *node = get_node(mgr, refer);
	if (node != NULL) {
		node->recv_package(node, type, refer, data, datalen);
	}
}

void package_dispatch_default(vps_mgr_t *mgr, int type, int refer, unsigned char *data, int datalen)
{
	if (RET_OK != transfer_data(mgr, TYPE_ERROR_UNKNOW_TYPE, 0, NULL, 0)) {

	}
}

void package_dispatch(vps_mgr_t *mgr, int type, int refer, unsigned char *data, int datalen)
{
	if (datalen > 0 && (RET_OK != package_decode(mgr, data, datalen))) {
		return;
	}

	switch (type)
	{
	case TYPE_HEART:				package_dispatch_heart(mgr, type, refer, data, datalen); break;
	case TYPE_OK:					package_dispatch_ok(mgr, type, refer, data, datalen); break;
	case TYPE_ERROR:				package_dispatch_error(mgr, type, refer, data, datalen); break;
	case TYPE_ENCODE_ENABLE:		package_dispatch_encode_enable(mgr, type, refer, data, datalen); break;
	case TYPE_GET_IMSI:				package_dispatch_get_imsi(mgr, type, refer); break;
	case TYPE_GET_VERNAME:			package_dispatch_get_version_name(mgr, type, refer, data, datalen); break;
	case TYPE_GET_VERCODE:			package_dispatch_get_version_code(mgr, type, refer, data, datalen); break;
	case TYPE_SET_VPS_TOTAL_IDLE:	package_dispatch_set_vps_total_idle(mgr, type, refer, data, datalen); break;
	case TYPE_SET_HEARTBEAT_TIMEOUT:package_dispatch_set_heartbeat_timeout(mgr, type, refer, data, datalen); break;
	case TYPE_CONNECT:				package_dispatch_connect(mgr, type, refer, data, datalen); break;
	case TYPE_OPT_NO_RESP:
	case TYPE_TRANS_DATA:
	case TYPE_CONNECT_CLOSE:		package_dispatch_transfer_data(mgr, type, refer, data, datalen); break;
	default:						package_dispatch_default(mgr, type, refer, data, datalen); break;
	}
}

void package_splite(vps_mgr_t *mgr) 
{
	do {
		int type = -1, refer = 0, packagelen = 0;
		unsigned char *package = NULL;
		if (!has_a_full_package(mgr->zxsRecvData, mgr->offsetRecvData, &type, &refer, &package, &packagelen)) {
			break;
		}
		package_dispatch(mgr, type, refer, package, packagelen > 3 ? packagelen - 4 : 0);
		mgr->offsetRecvData += packagelen;
	} while (true);
}

node_t *get_node(vps_mgr_t *mgr, int refer)
{
	struct list_head *pos = NULL;
	list_for_each(pos, &mgr->workNodeHeader) {
		node_t *node = list_entry(pos, node_t, node);
		if (node->refer == refer) {
			return node;
		}
	}
	return NULL;
}

int vps_mgr_init(vps_mgr_t *mgr) 
{
	mgr->heartbeatTimeout = VPS_MGR_HEART_TIMEOUT;
	mgr->heartbeatLastRecv = time(NULL);
	init_list_head(&mgr->workNodeHeader);

	mgr->flow_statistics = create_flow_statistics(2);
	if (NULL == mgr->flow_statistics) {
		return RET_ERROR;
	}

	mgr->data_queue = data_queue_create();
	if (NULL == mgr->data_queue) {
		return RET_ERROR;
	}

	mgr->zxsRecvData = string_create();
	mgr->zxsSendData = string_create();
	if (mgr->zxsRecvData == NULL || mgr->zxsSendData == NULL) {
		if (mgr->zxsRecvData != NULL) {
			string_release(mgr->zxsRecvData);
			mgr->zxsRecvData = NULL;
		}

		if (mgr->zxsSendData != NULL) {
			string_release(mgr->zxsSendData);
			mgr->zxsSendData = NULL;
		}
		return RET_ERROR;
	}

	mgr->channel_mgr = &g_channel_mgr;
	return RET_OK;
}

void vps_mgr_release(vps_mgr_t *mgr)
{
	if (mgr->zxsRecvData != NULL) {
		string_release(mgr->zxsRecvData);
	}
	if (mgr->zxsSendData != NULL) {
		string_release(mgr->zxsSendData);
	}
	if (mgr->zxsKey != NULL) {
		string_release(mgr->zxsKey);
	}
	if (mgr->zxsIMSI != NULL) {
		string_release(mgr->zxsIMSI);
	}
	if (mgr->fd !=0) {
		socket_close(mgr->fd);
	}
	if (mgr->node != NULL) {
		select_node_release(mgr->node);
	}
	if (NULL != mgr->flow_statistics) {
		flow_statistics_release(mgr->flow_statistics);
	}
	if (NULL != mgr->data_queue) {
		data_queue_release(mgr->data_queue);
	}
}

void vps_mgr_list_clear(vps_mgr_t *mgr) 
{
	struct list_head *pos = NULL;
	list_for_each(pos, &mgr->workNodeHeader) {
		node_t *wnode = list_entry(pos, node_t, node);
		list_del(pos);
		pos = pos->prev;
		wnode->release(wnode);
	}
}

int vps_mgr_select_node_init(vps_mgr_t *mgr)
{
	if (RET_OK != socket_create_nonblock(&mgr->fd)) {
		return RET_ERROR;
	}

	mgr->node = select_create_node();
	if (mgr->node == NULL) {
		return RET_ERROR;
	}
	mgr->node->fd = mgr->fd;
	mgr->node->connected = call_back_connected;
	mgr->node->read = call_back_read;
	//mgr->node->write = call_back_write;
	mgr->node->write = call_back_write_data_queue;
	mgr->node->timeout = call_back_timeout;
	mgr->node->get_mode = call_back_get_mode;
	mgr->node->prepare = call_back_prepare;
	mgr->node->param = mgr;
	mgr->node->selectMode |= SELECT_MODE_CONNECT;
	mgr->node->timeOut = VPS_MGR_CONNECT_TIMEOUT;
	return RET_OK;
}

int start(const char *szHost, int nPort, const char *szIMSI)
{
	LOGI("vps start, addr: %s:%d", szHost, nPort);
	vps_mgr_t mgr = { 0 };
	g_mgr = &mgr;
	mgr.encode = VPS_ENCODE_DISABLE;
	if (RET_OK != select_init(&mgr.st)) {
		return RET_ERROR;
	}
	if (RET_OK != vps_mgr_init(&mgr)) {
		goto clean;
	}
	if (NULL == (mgr.zxsKey = string_create_chars(VPS_KEY))) {
		goto clean;
	}
	if (szIMSI != NULL && (NULL == (mgr.zxsIMSI = string_create_chars(szIMSI)))) {
		goto clean;
	}
	if (RET_OK != vps_mgr_select_node_init(&mgr)) {
		goto clean;
	}
	if (RET_OK != socket_connect(0, mgr.fd, szHost, nPort)) {
		goto clean;
	}
	if (RET_OK != select_fd_add(mgr.st, mgr.node)) {
		goto clean;
	}
	
	LOGI("select_thread.");
	select_thread(mgr.st, g_exit_timestamp);
	LOGI("select_thread end. %lld", g_exit_timestamp == 0? g_exit_timestamp : g_exit_timestamp - time(NULL));
	
	g_mgr = NULL;
	select_release(mgr.st);
	vps_mgr_list_clear(&mgr);
	vps_mgr_release(&mgr);

	LOGI("vps exit");
	
	return RET_OK;

clean:
	select_release(mgr.st);
	vps_mgr_release(&mgr);
	LOGI("vps error exit");
	return RET_ERROR;
}

int close()
{
	if (g_mgr != NULL) {
		LOGW("close");
		select_thread_exit(g_mgr->st);
	}
	return RET_OK;
}

int vps_mgr_set_channel_response(int id, unsigned char *data, int datalen)
{
	LOGD("vps_mgr_set_channel_response");
	return channel_mgr_send_data_to_server(&g_channel_mgr, id, data, datalen);
	return RET_OK;
}

int flow_limit_calc1(vps_mgr_t *mgr, unsigned int flow_last, unsigned int flow_cache)
{
	const static int FLOW_LIMIT_MAX = 5 * 1024 * 1024;
	const static int FLOW_LIMIT_MIN = 1024;

	unsigned int limit_new = 0;
	unsigned int limit_last = mgr->flow_limit;
	unsigned int cache_last = mgr->flow_cache;
	unsigned int limit_last1 = mgr->flow_limit_earlier;

	if (flow_cache == 0) {
		limit_new = FLOW_LIMIT_MAX;
		if (limit_last > FLOW_LIMIT_MIN) {
			limit_new = (unsigned int)(limit_last * 1.5);
			if (limit_new > FLOW_LIMIT_MAX) {
				limit_new = FLOW_LIMIT_MAX;
			}
		}
		else if (limit_last <= FLOW_LIMIT_MIN && limit_last > 0) {
			limit_new = (unsigned int)((limit_last + limit_last1 + (cache_last > flow_cache ? (cache_last - flow_cache) / 2 : 0)) / 2);
		}
	}
	else if (flow_cache > 2.0 * flow_last && flow_cache > 1024) {
		limit_new = FLOW_LIMIT_MIN;
	}
	else if (flow_cache > 1.3 * flow_last && flow_cache > 1024) {
		limit_new = flow_last / 20 > FLOW_LIMIT_MIN ? flow_last / 20 : FLOW_LIMIT_MIN;
	}
	else if (flow_cache > 0.7 * flow_last && flow_cache > 1024) {
		limit_new = flow_last / 10 > FLOW_LIMIT_MIN ? flow_last / 10 : FLOW_LIMIT_MIN;
	}
	else if (flow_cache > cache_last) {
		limit_new = (unsigned int)((limit_last + limit_last1 + (flow_cache > cache_last ? (flow_cache - cache_last) / 2 : 0)) / 2 * (1 + ((flow_cache > 0.5 * flow_last) ? -0.05 : 0.05)));
	}
	else {
		limit_new = (unsigned int)((limit_last + limit_last1 + ((cache_last > flow_cache) ? (cache_last - flow_cache) / 2 : 0)) / 2 * (1 + ((flow_cache > 0.5 * flow_last) ? -0.05 : 0.05)));
	}

	mgr->flow_cache = flow_cache;
	mgr->flow_limit_earlier = limit_last;
	mgr->flow_limit = limit_new;

	return limit_new;
}

int flow_limit_calc(vps_mgr_t *mgr, unsigned int flow_last, unsigned int flow_cache) {
	long a = 5 * 1024;
	long b = 50 * 1024;

	if (mgr->flow_limit == 0) {
		mgr->flow_limit = 1000 * 1024;
	}

	unsigned int limit_new = 0;
	unsigned int limit_last = mgr->flow_limit;
	unsigned int cache_last = mgr->flow_cache;
	unsigned int limit_last1 = mgr->flow_limit_earlier;



	if (flow_last * 0.5 < b) {
		b = (long)(flow_last * 0.5);
		if (b<a + 1024) b = a + 1024;
	}

	static int k = 0;
	int k1 = k;
	if (flow_cache == 0) {
		limit_new = (long)((limit_last + limit_last1) / 2 * (1 + 0.1*k1++)); // 0.1每次+0.05系统
		if (limit_new < flow_last * 0.3) {
			limit_new = (long)(flow_last * 0.3);
		}
		if (limit_new < limit_last) {
			limit_new = limit_last;
		}
	}
	else if (flow_cache < a) {
		long cache_d = (flow_cache - cache_last) / 2;
		limit_new = (long)((limit_last + limit_last1) / 2 * (1 + 0.03));
	}
	else if (flow_cache >= a && flow_cache <= b) {
		limit_new = (long)((limit_last + limit_last1) / 2);
	}
	else if (flow_cache > 2.5 * flow_last) {
		limit_new = 1024;
	}
	else if (flow_cache > 2.0 * flow_last) {
		limit_new = flow_last / 20; // 最少1k
	}
	else if (flow_cache > 1.5 * flow_last) {
		limit_new = flow_last / 10;// 最少1k else {
	}
	else {

		if (flow_cache > cache_last) {
			limit_new = (long)(limit_last * (1 - 0.03));
		}
		else {
			long cache_d = (flow_cache - cache_last) / 2;
			limit_new = (long)((limit_last + limit_last1 + (cache_d < 0 ? -cache_d / 2 : 0)) / 2 * (1 - 0.03));
		}
	}

	if (limit_new > 5000 * 1024) {
		limit_new = 5000 * 1024;
	}
	if (k1 == k) { k = 0; }
	else { k = k1; }
	mgr->flow_cache = flow_cache;
	mgr->flow_limit_earlier = limit_last;
	mgr->flow_limit = limit_new;

	return limit_new;
}

int flow_limit_calc_not_limit(vps_mgr_t *mgr, unsigned int flow_last, unsigned int flow_cache) {

	unsigned int limit_new = 1000 * 1024 * 5;

	mgr->flow_cache = flow_cache;
	mgr->flow_limit_earlier = mgr->flow_limit;
	mgr->flow_limit = limit_new;

	return limit_new;
}