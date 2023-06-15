//
// Created by wyf on 2023/5/23.
//

#include "VpsLoginMgr.h"
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <pthread.h>
#include <string>
#include "httpUtil.h"
#include "cmath"
#include "zxlog.h"
#include "json.h"
#include "VpsMgr.h"
#include "WorkNode.h"
#include <sys/time.h>
#include "TerminalUtil.h"
#include "HttpPostModule.h"
#include "md5.h"
#include "curl_http.h"

using namespace std;

struct VpsInfo {
    int connTimeout = 15;
    int retry = 0;
    int retryInterval = 180;
    string host;
    int port = 0;
};

struct SocketInfo {
    /**
     * 连接超时时间，单位秒，默认15
     */
    int connTimeout = 15;
    /**
     * 总超时时间，单位秒，默认60
     */
    int totalTimeout = 60;
};

struct ConfigInfo {
    struct VpsInfo vps;
    struct SocketInfo socket;
    int reloadInterval = 300;
};

struct LoginResponse {
    int result = -1;
    string message;
    struct ConfigInfo config;
};


LoginResponse sendLoginRequest(char *url, char *t, char *sign, char *channel, char *pkg,
                               char *u, char *e, char *mac, int net, char *ver);

static long long currentTimeInMilliseconds()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

unsigned char ToHex(unsigned char x)
{
    return  x > 9 ? x + 55 : x + 48;
}



std::string UrlEncode(const std::string& str)
{
    std::string strTemp = "";
    size_t length = str.length();
    for (size_t i = 0; i < length; i++)
    {
        if (isalnum((unsigned char)str[i]) ||
            (str[i] == '-') ||
            (str[i] == '_') ||
            (str[i] == '.') ||
            (str[i] == '~'))
            strTemp += str[i];
        else if (str[i] == ' ')
            strTemp += "+";
        else
        {
            strTemp += '%';
            strTemp += ToHex((unsigned char)str[i] >> 4);
            strTemp += ToHex((unsigned char)str[i] % 16);
        }
    }
    return strTemp;
}

/**
 *
 * @param url
 * @param t
 * @param sign
 * @param channel
 * @param pkg
 * @param u
 * @param e
 * @param mac
 * @param net
 * @param ver
 * @return
 */
int vpsWorkerWithLogin(char *url, char *t, char *sign, char *channel, char *pkg,
                       char *u, char *e, char *mac, int net, char *ver) {

    LoginResponse loginResponse = sendLoginRequest(
            url, t, sign, channel, pkg, u, e, mac,
            net, ver);

    if (0 != loginResponse.result || loginResponse.config.vps.retry < 1) {
        sleep(loginResponse.config.reloadInterval);
        return RET_ERROR;
    }

    // 关机数据不全，退出
    if (loginResponse.config.vps.host.empty() || loginResponse.config.vps.port == 0) {
        sleep(loginResponse.config.reloadInterval);
        return RET_ERROR;
    }

    long long t1 = currentTimeInMilliseconds();
    for(int i = 0; i < loginResponse.config.vps.retry; i++) {
        vps_mgr_set_config(loginResponse.config.vps.connTimeout);
        work_node_set_config(loginResponse.config.socket.connTimeout, loginResponse.config.socket.totalTimeout);
        int ret = start(loginResponse.config.vps.host.data(), loginResponse.config.vps.port, u);
        if (ret == RET_ERROR && loginResponse.config.vps.retryInterval > 0) {
            sleep(loginResponse.config.vps.retryInterval);
        }
    }

    int dt = (currentTimeInMilliseconds() - t1) / 1000;
    if (loginResponse.config.reloadInterval > dt) {
        LOGI("sleep %d s", loginResponse.config.reloadInterval - dt);
        sleep(loginResponse.config.reloadInterval - dt);
    }

    return 0;
}

string buildPostData(char *url, char *t, char *sign, char *channel, char *pkg,
                     char *u, char *e, char *mac, int net, char *ver)
{
    char * json = getJsonSystemProperties();

    Json::Value root;
    Json::Reader().parse(json, root);
    root["e"] = e;
    root["u"] = u;
    root["m"] = mac;
    root["n"] = net;  //networktype
    root["a"] = url;
    root["t"] = t;
    root["s"] = sign;
    root["svc"] = VPS_SO_VERSION_CODE;
    root["svn"] = VPS_SO_VERSION_NAME;
    root["jvc"] = 0;
    root["jvn"] = ver;
    root["c"] = channel;
    root["p"] = pkg;
    root["sb"] = 0;
    root["lex"] = 0;
    root["lms"] = "";
    string postData = Json::FastWriter().write(root);

    free(json);
    return postData;
}

LoginResponse respParseToLoginStruct(string resp)
{
    LOGI("respParseToLoginStruct resp: %s", resp.data());

    LoginResponse loginResponse;

    // 初始化一个默认数据
    loginResponse.result = 1000;
    loginResponse.message = "";
    loginResponse.config.reloadInterval = 5 * 60;
    loginResponse.config.vps.retry = 0;
    loginResponse.config.vps.retryInterval = 0;
    loginResponse.config.vps.connTimeout = 15;
    loginResponse.config.vps.host = "";
    loginResponse.config.vps.port = 0;
    loginResponse.config.socket.connTimeout = 15;
    loginResponse.config.socket.totalTimeout = 60;

    Json::Value root;
    if (Json::Reader().parse(resp, root)) {
        if (!root["result"].isNull() && root["result"].isInt()) {
            loginResponse.result = root["result"].asInt();
        }
        if (!root["message"].isNull() && root["message"].isString()) {
            loginResponse.message = root["message"].isNull() ? "" : root["message"].asString();
        }

        if (!root["config"].isNull() && root["config"].isObject()) {
            if (!root["config"]["reloadInterval"].isNull() && root["config"]["reloadInterval"].isInt()) {
                loginResponse.config.reloadInterval = root["config"]["reloadInterval"].asInt();
            }

            if (!root["config"]["socket"].isNull() && root["config"]["socket"].isObject()) {
                Json::Value socket = root["config"]["socket"];
                if (!socket["connTimeout"].isNull() && socket["connTimeout"].isInt()) {
                    loginResponse.config.socket.connTimeout = socket["connTimeout"].asInt();
                }
                if (!socket["totalTimeout"].isNull() && socket["totalTimeout"].isInt()) {
                    loginResponse.config.socket.totalTimeout = socket["totalTimeout"].asInt();
                }
            }

            if (!root["config"]["vps"].isNull() && root["config"]["vps"].isObject()) {
                Json::Value vps = root["config"]["vps"];
                if (!vps["retry"].isNull() && vps["retry"].isInt()) {
                    loginResponse.config.vps.retry = vps["retry"].asInt();
                }
                if (!vps["retryInterval"].isNull() && vps["retryInterval"].isInt()) {
                    loginResponse.config.vps.retryInterval = vps["retryInterval"].asInt();
                }
                if (!vps["connTimeout"].isNull() && vps["connTimeout"].isInt()) {
                    loginResponse.config.vps.connTimeout = vps["connTimeout"].asInt();
                }
                if (!vps["host"].isNull() && vps["host"].isString()) {
                    loginResponse.config.vps.host = vps["host"].asString();
                }
                if (!vps["port"].isNull() && vps["port"].isInt()) {
                    loginResponse.config.vps.port = vps["port"].asInt();
                }
            }

        }
    } else {
        // 解析失败
        loginResponse.result = 1000;
        loginResponse.message = "json解析失败";
    }
    return loginResponse;
}

LoginResponse sendLoginRequest(char *url, char *t, char *sign, char *channel, char *pkg,
                               char *u, char *e, char *mac, int net, char *ver) {

    LOGI("sendLogin request");

    string postData = "login=" + UrlEncode(buildPostData(url, t, sign, channel, pkg, u, e, mac, net, ver));
    LOGI("login post data is %s", postData.data());

    LOGI("vps login mgr init send login request");
//    string urlStr = url; // getConfigUrl(mac);


    string resPost0;

    int ret = curl_post_req(url, postData, resPost0);


//    int ret= http_post(url, (unsigned char *)postData.data(), postData.length(), &buffer);

    LOGI("sdk_login resped" );

    LOGI("sdk_login resp ret: %d", ret);

    LOGI("sdk_login resp: %s", resPost0.data());

//    std::string response(buffer.buf, buffer.buf_used);
//    string response =(char *) buffer.buf;
    LOGI("response resp: %s", resPost0.data());

    LoginResponse loginResponse = respParseToLoginStruct(resPost0);


    return loginResponse;
}
