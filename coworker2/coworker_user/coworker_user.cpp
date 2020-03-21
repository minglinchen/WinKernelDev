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
    char tst_msg[1024] = { 0 };

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

    // 这里开始，其实是对驱动的一系列测试。分配3个字符串：
    // 1.长度为0.应该可以正常输入。
    // 2.长度为511字节，应该可以正常输入。
    // 3.长度为512字节，应该返回失败。
    // 4.长度为1024字节的字符串，但声明缓冲区长度为128，应该返回失败。
    // 5.第一次读取，应该读出msg的内容。
    // 5.第一次读取，应该读出长度为511字节的字符串。
    // 6.第二次读取，应该读出长度为0的字符串。
    do {
        memset(tst_msg, '\0', 1);
        if(!DeviceIoControl(device, CWK_DVC_SEND_STR, tst_msg, 1, NULL, 0, &ret_len, 0))
        {
            ret = -3;
            break;
        }
        else
        {
            printf("TEST1 PASS.\r\n");
        }

        memset(tst_msg, '\0', 512);
        memset(tst_msg, 'a', 511);
        if(!DeviceIoControl(device, CWK_DVC_SEND_STR, tst_msg, 512, NULL, 0, &ret_len, 0))
        {
            ret = -5;
            break;
        }
        else
        {
            printf("TEST2 PASS.\r\n");
        }

        memset(tst_msg, '\0', 513);
        memset(tst_msg, 'a', 512);
        if(DeviceIoControl(device, CWK_DVC_SEND_STR, tst_msg, 513, NULL, 0, &ret_len, 0))
        {
            // 这个缓冲区已经过长，理应返回失败。如果成功了则
            // 认为是错误。
            ret = -5;
            break;
        }
        else
        {
            printf("TEST3 PASS.\r\n");
        }

        memset(tst_msg, '\0', 1024);
        memset(tst_msg, 'a', 1023);
        if(DeviceIoControl(device, CWK_DVC_SEND_STR, tst_msg, 128, NULL, 0, &ret_len, 0))
        {
            // 这个缓冲区虽然不过长，但是字符串过长，理应返回失
            // 败。如果成功了则认为是错误。
            ret = -5;
            break;
        }
        else
        {
            printf("TEST4 PASS.\r\n");
        }
        free(tst_msg);

        // 现在开始测试输出。第一个读出的应该是msg.
        if(DeviceIoControl(device, CWK_DVC_RECV_STR, NULL, 0, tst_msg, 1024, &ret_len, 0) == 0 || ret_len != strlen(msg) + 1)
        {
            ret = -6;
            break;
        }
        else 
        {
            printf("TEST5 PASS.\r\n");
        }

        // 第二个读出的应该是长度为0的空字符串。
        if(DeviceIoControl(device, CWK_DVC_RECV_STR, NULL, 0, tst_msg, 1024, &ret_len, 0) == 0 || ret_len != 1)
        {
            ret = -6;
            break;
        }
        else 
        {
            printf("TEST6 PASS.\r\n");
        }

        // 第三个读出的应该是长度为511的全a字符串
        if(DeviceIoControl(device, CWK_DVC_RECV_STR, NULL, 0, tst_msg, 1024, &ret_len, 0) != 0 || ret_len != 511 + 1)
        {
            ret = -6;
            break;
        }
        else 
        {
            printf("TEST7 PASS.\r\n");
        }
    } while(0);
    CloseHandle(device);
	return ret;
}

