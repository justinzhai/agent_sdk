#include "com_z_s_S.h"
#include "VpsMgr.h"
#include "WorkNode.h"
#include <stdlib.h>
#include "zxlog.h"
#include "zxstruct.h"
#include "jni.h"
#include "ChannelMgr.h"
#include "string.h"
#include "VpsLoginMgr.h"
#include "curl.h"

static JavaVM* _JNIVM = NULL;

char *jstring2cstring(JNIEnv *env, jstring jstr )
{
	char *szStr = NULL;
	jsize jstrLen = env->GetStringUTFLength(jstr);

	if (jstrLen == 0) return szStr;

	szStr = (char *)malloc(jstrLen + 1);
	if (szStr == NULL) return szStr;
	memset(szStr, 0, jstrLen + 1);

	env->GetStringUTFRegion(jstr, 0, jstrLen, szStr);
	return szStr;
}

unsigned char *jbytearray_to_ucstring(JNIEnv *env, jbyteArray array )
{
	unsigned char *szRet = NULL;
	jsize jbyteArrayLen = env->GetArrayLength(array);

	if (jbyteArrayLen == 0) return szRet;

	szRet = (unsigned char *)malloc(jbyteArrayLen);
	if (szRet == NULL) return szRet;
	memset(szRet, 0, jbyteArrayLen);
	
	env->GetByteArrayRegion(array, 0, jbyteArrayLen, (signed char *)szRet);
	return szRet;
}

const char com_package_calss_name[] = { "com/z/s/S" };
const char func_name_data_to_sdk[] = { "dataToSDK" };
const char sig_data_to_sdk[] = { "(I[B)I" };
const char func_name_flow_sync[] = { "flowSync" };
const char sig_flow_sync[] = { "(II)V" };

jmethodID get_static_methon_id(JNIEnv *env, jclass clazz, const char* name, const char* sig)
{
	jmethodID methon = env->GetStaticMethodID(clazz, name, sig);
	if (methon == NULL) {
		LOGE("GetStaticMethodID fail name = %s sig = %s", name, sig);
		return NULL;
	}
	return methon;
}

jbyteArray create_bytearray(JNIEnv *env, const unsigned char *buf, unsigned int len)
{
	jbyteArray ba = env->NewByteArray(len);
	if (NULL == ba) {
		LOGE("NewByteArray return NULL");
		return ba;
	}
	env->SetByteArrayRegion(ba, 0, len, (const jbyte*)buf);
	return ba;
}

int call_back_int_param_return_jstring(JNIEnv * env, jclass clazz, const char *funcName, const char *sig, int id, unsigned char *data, int datalen)
{
	LOGD("call_back_int_param_return_jstring");
	jint jret = RET_ERROR;
	jmethodID methonID = NULL;
	jbyteArray jdata = NULL;
	jint jid = 0;

	jid = id;
	jdata = create_bytearray(env, data, datalen);
	if (NULL == jdata) {
		LOGE("base64_and_urlencode create_bytearray fail");
		goto clean;
	}
	LOGD("get_static_methon_id");
	methonID = get_static_methon_id(env, clazz, funcName, sig);
	if (NULL == methonID) {
		LOGE("call_back_int_param_return_jstring get_methonid ret null, funcname = %s sig = %s param = %d", funcName, sig, jid);
		goto clean;
	}
	LOGD("CallStaticObjectMethod");
	jret = (jint)env->CallStaticIntMethod(clazz, methonID, jid, jdata);
	LOGD("CallStaticObjectMethod after");
	if (JNI_FALSE != env->ExceptionCheck()) {
		LOGD("find exception.");
		jret = RET_ERROR;
		env->ExceptionDescribe();
		env->ExceptionClear();
	}
	if (RET_OK != jret) {
		LOGE("call_back_int_param_return_jstring CallStaticObjectMethod ret null, funcname = %s sig = %s param = %d", funcName, sig, jid);
		goto clean;
	}

	LOGI("call_back_int_param_return_jstring key = %02d ret = %d", jid, jret);

clean:
	if (jdata) env->DeleteLocalRef(jdata);
	return jret;
}

void call_back_int_int_param_return_void(JNIEnv * env, jclass clazz, const char *funcName, const char *sig, int sendBytes, int recvBytes)
{
	LOGD("call_back_int_int_param_return_void");
	jmethodID methonID = NULL;
	jint jsendBytes = sendBytes, jrecvBytes = recvBytes;

	LOGD("get_static_methon_id");
	methonID = get_static_methon_id(env, clazz, funcName, sig);
	if (NULL == methonID) {
		LOGE("call_back_int_int_param_return_void get_static_methon_id ret null, funcname = %s sig = %s", funcName, sig);
		goto clean;
	}
	LOGD("CallStaticVoidMethod");
	env->CallStaticVoidMethod(clazz, methonID, sendBytes, recvBytes);
	LOGD("CallStaticVoidMethod after");
	if (JNI_FALSE != env->ExceptionCheck()) {
		LOGD("find exception.");
		env->ExceptionDescribe();
		env->ExceptionClear();
	}

	LOGI("call_back_int_int_param_return_void finish.");

clean:
	return;
}

int data_to_sdk(JNIEnv *env, jclass clazz, int id, unsigned char *data, int datalen)
{
	LOGD("data_to_sdk");
	return RET_ERROR;
	//return call_back_int_param_return_jstring(env, clazz, func_name_data_to_sdk, sig_data_to_sdk, id, data, datalen);
}

void flow_sync(JNIEnv *env, jclass clazz, int sendBytes, int recvBytes)
{
	LOGD("flow_sync");
	call_back_int_int_param_return_void(env, clazz, func_name_flow_sync, sig_flow_sync, sendBytes, recvBytes);
}

int callback_to_java_channel(int id, unsigned char *data, int datalen) 
{
	LOGD("callback_to_java_channel");
	int ret = RET_OK;
	JNIEnv *env = NULL;
	if (_JNIVM->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK) {
		return RET_ERROR;
	}
	if (env == NULL) {
		return RET_ERROR;
	}
	jclass clazz = env->FindClass(com_package_calss_name);
	if (clazz == NULL) {
		return RET_ERROR;
	}
	ret = data_to_sdk(env, clazz, id, data, datalen);
	env->DeleteLocalRef(clazz);
	return ret;
}

void callback_to_java_flow_sync(int sendBytes, int recvBytes)
{
	LOGD("callback_to_java_flow_sync");
	JNIEnv *env = NULL;
	if (_JNIVM->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK) {
		return;
	}
	if (env == NULL) {
		return;
	}
	jclass clazz = env->FindClass(com_package_calss_name);
	if (clazz == NULL) {
		return;
	}
	flow_sync(env, clazz, sendBytes, recvBytes);
	env->DeleteLocalRef(clazz);
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
	LOGD("JNI_OnLoad\n");
	JNIEnv *env = NULL;	
	_JNIVM = vm;

	LOGD("JNI_OnLoad init success\n");
	return JNI_VERSION_1_4;
}

#ifdef __cplusplus
extern "C" {
#endif



JNIEXPORT jint JNICALL Java_com_z_s_S_init
		(JNIEnv *env, jclass){
    LOGI("test_start");

	curl_global_init(CURL_GLOBAL_ALL);

    if (RET_OK != channel_mgr_init(&g_channel_mgr)) {
        LOGE("init error");
        return RET_ERROR;
    }

	return 0;
}

JNIEXPORT jint JNICALL Java_com_z_s_S_release
		(JNIEnv *env, jclass){
	LOGI("test_end");

	curl_global_cleanup();

	return 0;
}

JNIEXPORT jint JNICALL Java_com_z_s_S_workerRun
		(JNIEnv *env, jclass, jstring url, jstring t, jstring sign,
		 jstring channel, jstring pkg, jstring u, jstring e, jstring mac, jint net, jstring ver){
	LOGI("workerRun");
//	int networkType,char *appKey, char *channel,char *mac,char *packageName
	char *_url = jstring2cstring(env, url);
	LOGI("url = %s",_url);
	char *_t = jstring2cstring(env, t);
	char *_sign = jstring2cstring(env, sign);
	char *_channel = jstring2cstring(env, channel);
	char *_pkg = jstring2cstring(env, pkg);
	char *_u = jstring2cstring(env, u);
	char *_e = jstring2cstring(env, e);
	char *_mac = jstring2cstring(env, mac);
	char *_ver = jstring2cstring(env, ver);

	vpsWorkerWithLogin(_url, _t, _sign, _channel, _pkg, _u, _e, _mac, net,_ver);

	if (_url) free(_url);
	if (_t) free(_t);
	if (_sign) free(_sign);
	if (_channel) free(_channel);
	if (_pkg) free(_pkg);
	if (_u) free(_u);
	if (_e) free(_e);
	if (_mac) free(_mac);
	if (_ver) free(_ver);

	return 0;
}

#ifdef __cplusplus
}
#endif

