//
// Created by zhl on 2023/5/11.
//

#include "TerminalUtil.h"
#include <sys/system_properties.h>
#include <cstdio>
#include <filesystem>

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <linux/if.h>
#include <linux/in.h>
#include <sys/ioctl.h>
#include "json.h"



#include "android/log.h"
static const char *TAG = "jniLog";

char *generateUuid();

#define LOGI(fmt, args...) __android_log_print(ANDROID_LOG_INFO, TAG, fmt, ##args)
#define LOGD(fmt, args...) __android_log_print(ANDROID_LOG_DEBUG, TAG, fmt, ##args)
#define LOGE(fmt, args...) __android_log_print(ANDROID_LOG_ERROR, TAG, fmt, ##args)

int printProperty(const char *key, const char *value, void *cookie)
{
    printf("key=%s,value=%s\n", key, value);
    return 0;
}


char * getHsman(char *json){
    char value[PROP_VALUE_MAX];
    __system_property_get("ro.product.manufacturer", value);
    sprintf(json, "%s\"hm\":\"%s\"", json, value);
    return value;
}

char * getHstype(char *json){
    char value[PROP_VALUE_MAX];
    __system_property_get("ro.product.model", value);
    sprintf(json, "%s,\"ht\":\"%s\"", json, value);
    return value;
}

char * getOsVer(char *json){
    char value[PROP_VALUE_MAX];
    __system_property_get("ro.build.version.release", value);
    sprintf(json, "%s,\"o\":\"%s\"", json, value);
    return value;
}

/**
 * 读取一个本地的uuid,如果没有则生成一个
 * @return
 */
char *getUuidFromLocalStorage(){
    char *uuid = (char *) malloc(37);
    FILE *fp = fopen("/data/local/tmp/uuid", "r");
    if (fp == NULL) {
        LOGI("uuid file not exist");
        uuid = generateUuid();
        LOGI("generate uuid %s", uuid);
        fp = fopen("/data/local/tmp/uuid", "w");
        if (fp == NULL) {
            LOGI("uuid file create fail");
            return uuid;
        }
        fwrite(uuid, 1, 37, fp);
        fclose(fp);
        return uuid;
    }
    fread(uuid, 1, 37, fp);
    fclose(fp);
    return uuid;
}



char *generateUuid() {

}

char* printTerminalInfo()
{
    char value[PROP_VALUE_MAX];
    __system_property_get("ro.product.model", value);
    LOGI("ro.product.model=%s", value);
    __system_property_get("ro.product.brand", value);
    LOGI("ro.product.brand=%s", value);
    __system_property_get("ro.product.name", value);
    LOGI("ro.product.name=%s", value);
    __system_property_get("ro.product.device", value);
    LOGI("ro.product.device=%s", value);
    __system_property_get("ro.product.board", value);
    LOGI("ro.product.board=%s", value);
    __system_property_get("ro.product.manufacturer", value);
    LOGI("ro.product.manufacturer=%s", value);
    __system_property_get("ro.build.version.release", value);
    LOGI("ro.build.version.release=%s", value);
    __system_property_get("ro.build.version.sdk", value);
    LOGI("ro.build.version.sdk=%s", value);
    __system_property_get("ro.build.version.incremental", value);
    LOGI("ro.build.version.incremental=%s", value);
    __system_property_get("ro.build.version.codename", value);
    LOGI("ro.build.version.codename=%s", value);
    __system_property_get("ro.build.version.all_codenames", value);
    LOGI("ro.build.version.all_codenames=%s", value);
    __system_property_get("ro.build.version.security_patch", value);
    LOGI("ro.build.version.security_patch=%s", value);
    __system_property_get("ro.build.version.base_os", value);
    LOGI("ro.build.version.base_os=%s", value);
    __system_property_get("ro.build.id", value);
    LOGI("ro.build.id=%s", value);
    __system_property_get("ro.build.display.id", value);
    LOGI("ro.build.display.id=%s", value);
    __system_property_get("ro.build.version.build_id", value);
    LOGI("ro.build.version.build_id=%s", value);
    __system_property_get("ro.build.version.build_tags", value);
    LOGI("ro.build.version.build_tags=%s", value);
    __system_property_get("ro.build.type", value);
    LOGI("ro.build.type=%s", value);
    __system_property_get("ro.build.user", value);
    LOGI("ro.build.user=%s", value);
    __system_property_get("ro.build.host", value);
    LOGI("ro.build.host=%s", value);
    __system_property_get("ro.build.tags", value);
    LOGI("ro.build.tags=%s", value);
    __system_property_get("ro.build.flavor", value);
    LOGI("ro.build.flavor=%s", value);
    __system_property_get("ro.build.selinux", value);
    LOGI("ro.build.selinux=%s", value);
    __system_property_get("ro.product.cpu.abi", value);
    LOGI("ro.product.cpu.abi=%s", value);
    __system_property_get("ro.product.cpu.abi2", value);
    LOGI("ro.product.cpu.abi2=%s", value);
    __system_property_get("ro.product.cpu.abilist", value);
    LOGI("ro.product.cpu.abilist=%s", value);
    __system_property_get("ro.product.cpu.abilist32", value);
    LOGI("ro.product.cpu.abilist32=%s", value);
    __system_property_get("ro.product.cpu.abilist64", value);
    LOGI("ro.product.cpu.abilist64=%s", value);
    __system_property_get("ro.product.first_api_level", value);
    LOGI("ro.product.first_api_level=%s", value);
    __system_property_get("ro.product.cpu.abilist", value);
    LOGI("ro.product.cpu.abilist=%s", value);
    __system_property_get("ro.product.first_api_level", value);
    LOGI("ro.product.first_api_level=%s", value);

    char mac_addr[80] = {0};
    printf( "MAC = %d, %s\n", get_mac_addr(mac_addr), mac_addr);
    return value;
}



int get_mac_addr(char *mac_addr)
{
    int sockfd;
    struct ifreq ifr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) >= 0) {
        //strncpy(ifr.ifr_name, "eth0", IFNAMESIZE);
        strncpy(ifr.ifr_name, "wlan0", IFNAMSIZ);

        ifr.ifr_addr.sa_family = AF_INET;

        if (ioctl(sockfd, SIOCGIFHWADDR, (char*) &ifr) == 0) {
            sprintf(mac_addr, "%02X:%02X:%02X:%02X:%02X:%02X",
                    (unsigned char) ifr.ifr_ifru.ifru_hwaddr.sa_data[0], (unsigned char) ifr.ifr_ifru.ifru_hwaddr.sa_data[1],
                    (unsigned char) ifr.ifr_ifru.ifru_hwaddr.sa_data[2], (unsigned char) ifr.ifr_ifru.ifru_hwaddr.sa_data[3],
                    (unsigned char) ifr.ifr_ifru.ifru_hwaddr.sa_data[4], (unsigned char) ifr.ifr_ifru.ifru_hwaddr.sa_data[5]);

            LOGI("mac addresss = %s", mac_addr);
            return 0;
        }
    }
    /* error */
    return -1;
}

char * getJsonSystemProperties(){
    char * json = (char *)malloc(1024);
    sprintf(json, "{");
    char *hsman = getHsman(json);
    char *hsType = getHstype(json);
    char *o = getOsVer(json);
    sprintf(json, "%s}", json);
    return json;

}



int main(int argc, char *argv[], char *envp[])
{
    char mac_addr[80] = {0};
    printf( "MAC = %d, %s\n", get_mac_addr(mac_addr), mac_addr );

    return 0;
}