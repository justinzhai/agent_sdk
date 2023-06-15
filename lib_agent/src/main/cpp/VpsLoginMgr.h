//
// Created by wyf on 2023/5/23.
//
//

/**
 * 本类用于处理login_sdk请求
 */
#ifndef VPS_SDK_WITH_CONFIG_VPSLOGINMGR_H
#define VPS_SDK_WITH_CONFIG_VPSLOGINMGR_H

int vpsWorkerWithLogin(char *url, char *t, char *sign, char *channel, char *pkg,
                       char *u, char *e, char *mac, int net, char *ver);

#endif //VPS_SDK_WITH_CONFIG_VPSLOGINMGR_H
