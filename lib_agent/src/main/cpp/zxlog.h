#ifndef __ZX_LOG_H__
#define __ZX_LOG_H__

#include <stdio.h>
#include <stdlib.h>
#include "system.h"
#include "string.h"

#ifdef ANDROID
	#include <android/log.h>

	#ifdef __cplusplus
	extern "C" {
	#endif

//		#ifdef debug
//		#define YWT_DEBUG_MODE //disable it @release
//		#endif

		#ifdef YWT_DEBUG_MODE
			#define LOG_TAG "ZxVps"
			#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
			#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
			#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
			#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
			static void printBuf(unsigned char* buf, int len) {
				char b[1024] = { 0 };
				int i = 0;
				for (i = 0; i < len; i++) {
					sprintf(b + (i % 100) * 3, "%02X ", buf[i]);
					if (((i + 1) % 100) == 0) {
						LOGD("%s", b);
						memset(b, 0, sizeof(b));
					}
				}
				if ((i % 100) != 0) {
					LOGD("%s", b);
				}
				LOGD("\n");
			}
		#else
			#define LOGD(...)
			#define LOGI(...)
			#define LOGE(...)
			#define LOGW(...)
		#endif

	#ifdef __cplusplus
	}
	#endif

#elif defined(linux)

	#include <string.h>
	#include <time.h>
	#define LOGFORMAT(x, format, ...) { \
		time_t now = time(NULL); \
		struct tm *t = localtime(&now); \
		char *buf = (char *)malloc(strlen(format) + 1024); \
		sprintf(buf, "%02d/%02d/%02d %02d:%02d:%02d [%-5s] %s\n", (t->tm_year + 1900)%100, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, x, format); \
		printf(buf, ##__VA_ARGS__); \
		free(buf); \
		fflush(stdout); \
	}

	#define LOGD(format, ...) 
	#define LOGI(format, ...) LOGFORMAT("INFO", format, ##__VA_ARGS__)
	#define LOGE(format, ...) LOGFORMAT("WARN", format, ##__VA_ARGS__)
	#define LOGW(format, ...) LOGFORMAT("ERROR", format, ##__VA_ARGS__)

#else // ANDROID

	#include <string.h>
	#include <time.h>
	#define LOGFORMAT(x, format, ...) { \
		struct tm t; \
		time_t now = time(NULL); \
		localtime_s(&t, &now); \
		char *buf = (char *)malloc(strlen(format) + 1024); \
		sprintf_s(buf, strlen(format) + 1024, "%02d/%02d/%02d %02d:%02d:%02d [%-5s] %s\n", (t.tm_year + 1900)%100, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, x, format); \
		printf(buf, ##__VA_ARGS__); \
		free(buf); \
	}

	#ifdef _DEBUG
		#define LOGD(format, ...) LOGFORMAT("DEBUG", format, ##__VA_ARGS__)
		#define LOGI(format, ...) LOGFORMAT("INFO", format, ##__VA_ARGS__)
		#define LOGE(format, ...) LOGFORMAT("WARN", format, ##__VA_ARGS__) 
		#define LOGW(format, ...) LOGFORMAT("ERROR", format, ##__VA_ARGS__) 
	#else
		#define LOGD(format, ...) 
		#define LOGI(format, ...) LOGFORMAT("INFO", format, ##__VA_ARGS__)
		#define LOGE(format, ...) LOGFORMAT("WARN", format, ##__VA_ARGS__) 
		#define LOGW(format, ...) LOGFORMAT("ERROR", format, ##__VA_ARGS__) 
	#endif // _DEBUG

#endif // ANDROID


#endif //__ZX_LOG_H__
