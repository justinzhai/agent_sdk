//
// Created by zhl on 2023/5/11.
//

#ifndef VPS_SDK_WITH_CONFIG_HTTPUTIL_H
#define VPS_SDK_WITH_CONFIG_HTTPUTIL_H

char* http_get(const char* url);
char* http_post(const char* url, const char* post_str);
#endif //VPS_SDK_WITH_CONFIG_HTTPUTIL_H