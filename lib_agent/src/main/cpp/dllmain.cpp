// dllmain.cpp : ���� DLL Ӧ�ó������ڵ㡣

#include <Windows.h>
#include "zxpipe.h"
#include "ChannelMgr.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		pipe_init();
		channel_mgr_init(&g_channel_mgr);
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}