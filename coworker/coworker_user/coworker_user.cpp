// coworker_user.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#define CWK_DEV_SYM L"\\\\.\\slbkcdo_3948d33e"

// 从应用层给驱动发送一个字符串。
#define  CWK_DVC_SEND_STR \
	(ULONG)CTL_CODE( \
	FILE_DEVICE_UNKNOWN, \
	0x911,METHOD_BUFFERED, \
	FILE_WRITE_DATA)

// 从驱动读取一个字符串
#define  CWK_DVC_RECV_STR \
	(ULONG)CTL_CODE( \
	FILE_DEVICE_UNKNOWN, \
	0x912,METHOD_BUFFERED, \
	FILE_READ_DATA)

int _tmain(int argc, _TCHAR* argv[])
{
	HANDLE device = NULL;
	ULONG ret_len;
    int ret = 0;
    char *msg = {"Hello driver, this is a message from app.\r\n"};

	// 打开设备.每次要操作驱动的时候，先以此为例子打开设备
	device=CreateFile(CWK_DEV_SYM,GENERIC_READ|GENERIC_WRITE,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_SYSTEM,0);
	if (device == INVALID_HANDLE_VALUE)
	{
		printf("coworker demo: Open device failed.\r\n");
		return -1;
	}
	else
		printf("coworker demo: Open device successfully.\r\n");

    if(!DeviceIoControl(device, CWK_DVC_SEND_STR, msg, strlen(msg) + 1, NULL, 0, &ret_len, 0))
    {
        printf("coworker demo: Send message failed.\r\n");
        ret = -2;
    }
    else
        printf("coworker demo: Send message successfully.\r\n");

    CloseHandle(device);
	return ret;
}

