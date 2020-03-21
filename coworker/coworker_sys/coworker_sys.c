///
/// @file   coworker_sys.c
/// @author tanwen
/// @date   2012-5-28
///

#include <ntifs.h>
#include <wdmsec.h>

PDEVICE_OBJECT g_cdo = NULL;

const GUID  CWK_GUID_CLASS_MYCDO =
{0x17a0d1e0L, 0x3249, 0x12e1, {0x92,0x16, 0x45, 0x1a, 0x21, 0x30, 0x29, 0x06}};

#define CWK_CDO_SYB_NAME    L"\\??\\slbkcdo_3948d33e"

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

void cwkUnload(PDRIVER_OBJECT driver)
{
	UNICODE_STRING cdo_syb = RTL_CONSTANT_STRING(CWK_CDO_SYB_NAME);
    ASSERT(g_cdo != NULL);
   	IoDeleteSymbolicLink(&cdo_syb);
    IoDeleteDevice(g_cdo);
}

NTSTATUS cwkDispatch(		
			      IN PDEVICE_OBJECT dev,
			      IN PIRP irp)
{
    PIO_STACK_LOCATION  irpsp = IoGetCurrentIrpStackLocation(irp);
    NTSTATUS status = STATUS_SUCCESS;
    ULONG ret_len = 0;
    while(dev == g_cdo) 
    {
        // 如果这个请求不是发给g_cdo的，那就非常奇怪了。
        // 因为这个驱动只生成过这一个设备。所以可以直接
        // 返回失败。
	    if(irpsp->MajorFunction == IRP_MJ_CREATE || irpsp->MajorFunction == IRP_MJ_CLOSE)
	    {
            // 生成和关闭请求，这个一律简单地返回成功就可以
            // 了。就是无论何时打开和关闭都可以成功。
            break;
	    }
    	
        if(irpsp->MajorFunction == IRP_MJ_DEVICE_CONTROL)
	    {
		    // 处理DeviceIoControl。
            PVOID buffer = irp->AssociatedIrp.SystemBuffer;  
            ULONG inlen = irpsp->Parameters.DeviceIoControl.InputBufferLength;
            ULONG outlen = irpsp->Parameters.DeviceIoControl.OutputBufferLength;
		    ULONG len;
		    switch(irpsp->Parameters.DeviceIoControl.IoControlCode)
		    {
            case CWK_DVC_SEND_STR:
                ASSERT(buffer != NULL);
                ASSERT(inlen > 0);
                ASSERT(outlen == 0);
                DbgPrint((char *)buffer);
                // 已经打印过了，那么现在就可以认为这个请求已经成功。
                break;
            case CWK_DVC_RECV_STR:
            default:
                // 到这里的请求都是不接受的请求。未知的请求一律返回非法参数错误。
                status = STATUS_INVALID_PARAMETER;
                break;
            }
        }
        break;
    }
    // 到这里的请求都是不接受的请求。未知的请求一律返回非法参数错误。
	irp->IoStatus.Information = ret_len;
	irp->IoStatus.Status = status;
	IoCompleteRequest(irp,IO_NO_INCREMENT);
	return status;
}


NTSTATUS DriverEntry(PDRIVER_OBJECT driver,PUNICODE_STRING reg_path)
{
	NTSTATUS status;
	ULONG i;
    UCHAR mem[256] = { 0 };

	// 生成一个控制设备。然后生成符号链接。
	UNICODE_STRING sddl = RTL_CONSTANT_STRING(L"D:P(A;;GA;;;WD)");
	UNICODE_STRING cdo_name = RTL_CONSTANT_STRING(L"\\Device\\cwk_3948d33e");
	UNICODE_STRING cdo_syb = RTL_CONSTANT_STRING(CWK_CDO_SYB_NAME);

    KdBreakPoint();

	// 生成一个控制设备对象。
	status = IoCreateDeviceSecure(
		driver,
		0,&cdo_name,
		FILE_DEVICE_UNKNOWN,
		FILE_DEVICE_SECURE_OPEN,
		FALSE,&sddl,
		(LPCGUID)&CWK_GUID_CLASS_MYCDO,
		&g_cdo);
	if(!NT_SUCCESS(status))
		return status;

	// 生成符号链接.
	IoDeleteSymbolicLink(&cdo_syb);
	status = IoCreateSymbolicLink(&cdo_syb,&cdo_name);
	if(!NT_SUCCESS(status))
	{
		IoDeleteDevice(g_cdo);
		return status;
	}
	
	// 所有的分发函数都设置成一样的。
	for(i=0;i<IRP_MJ_MAXIMUM_FUNCTION;i++)
	{
		driver->MajorFunction[i] = cwkDispatch;
	}

	// 支持动态卸载。
    driver->DriverUnload = cwkUnload;
	// 清除控制设备的初始化标记。
	g_cdo->Flags &= ~DO_DEVICE_INITIALIZING;
	return STATUS_SUCCESS;
}
