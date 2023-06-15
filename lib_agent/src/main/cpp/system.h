#ifndef __SYSTEM_H__
#define __SYSTEM_H__

// 内存泄露检查 配置方法 _CrtDumpMemoryLeaks(); 使用
#ifdef WIN32
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#endif // WIN32

// sleep



#endif // __SYSTEM_H__