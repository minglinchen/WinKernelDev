///
/// @file sh_ssdt_hook.c
/// @author tanwen
/// @date 2012-6-19
/// 
/// 版权声明：
/// 
/// 1.此份代码为谭文为《天书夜读：无文档Windows内核安全编程》所编写的代码
/// 范例。为随书附赠，从未出售，因此作者本人并不以此谋取任何商业利益，同时
/// 不为本代码提供任何担保。
///
/// 2.使用此代码用作辅助学习之外的其他用途必须要接受以下协议：此代码作者不
/// 为此代码担任何责任。包括但不限于由于使用此代码引起的任何直接或者间接的
/// 损失，以及知识产权争议导致的纠纷，本人也不会承担任何举证的责任。使用者
/// 必须自己承担一切责任。
///
/// 3.使用此代码即为接受上述协议。不接受上述协议者，不得使用此代码。

#include <ntifs.h>
#include <windef.h>
#include "../ssdt_hook/sh_ssdt_hook.h"
#include "../xtbl_hook/xtbl_hook.h"

static SH_ZW_CREATE_FILE_F hsOriginalZwCreateFile = NULL;
static SH_ZW_CLOSE_F hsOriginalZwClose = NULL;

// 这个例子中，hook ZwCreateFile和ZwClose,用来进程访问文件的情况。
NTSTATUS hsZwCreateFile(
	OUT PHANDLE  file_handle,
    IN ACCESS_MASK  desired_access,
    IN POBJECT_ATTRIBUTES  obj_attr,
    OUT PIO_STATUS_BLOCK  io_status,
    IN PLARGE_INTEGER  alloc_size  OPTIONAL,
    IN ULONG  file_attr,
    IN ULONG  shared_access,
    IN ULONG  create_dispos,
    IN ULONG  create_options,
    IN PVOID  ea_buffer  OPTIONAL,
    IN ULONG  ea_length)
{
	NTSTATUS status;
	// 这个函数最后还是调用旧函数。
	status = hsOriginalZwCreateFile(
		file_handle, 
		desired_access, 
		obj_attr, 
		io_status, 
		alloc_size, 
		file_attr, 
		shared_access, 
		create_dispos,
		create_options,
		ea_buffer,
		ea_length);
	// 打印一条信息，了解谁（哪个进程）想打开什么文件，不做别的更多的
	// 的事情。
	DbgPrint("ZwCreateFile: %wZ, desired_access = %x, returned status = %x, handle = %x\r\n", 
		obj_attr->ObjectName, desired_access, status, *file_handle);
	return status;
};

NTSTATUS hsZwClose(
    IN HANDLE  handle
)
{
	DbgPrint("ZwClose: handle %x was closed.\r\n", handle);
	return hsOriginalZwClose(handle);	
};

// 首先定义一个IofCallDriver的函数类型
typedef NTSTATUS (__fastcall *HS_IOF_CALLDIVER)(PDEVICE_OBJECT dev,PIRP irp);
HS_IOF_CALLDIVER hsOriginalIofCallDriver = NULL;

NTSTATUS __fastcall hsIofCallDriver(PDEVICE_OBJECT dev,PIRP irp)
{
    // 这个函数的调用次数太多了。如果打印东西的话，会铺天盖地。所以
    // 这里仅仅每256次打印一个。
    static ULONG count = 0;
    count ++;
    if(count % 256 == 0)
    {
        DbgPrint("IofCallDriver: dev = %x, irp = %x\r\n", dev, irp);
    }
    return hsOriginalIofCallDriver(dev, irp);
}


static void hsSleep(int msec)
{
    LARGE_INTEGER interval;
#define DELAY_ONE_MICROSECOND   (-10)
#define DELAY_ONE_MILLISECOND   (DELAY_ONE_MICROSECOND*1000)
#define DELAY_ONE_SECOND        (DELAY_ONE_MILLISECOND*1000)
    interval.QuadPart = (msec * DELAY_ONE_MILLISECOND);
    KeDelayExecutionThread(KernelMode, FALSE, &interval);
}


VOID hsDriverUnload(IN PDRIVER_OBJECT DriverObject) 
{
	UNICODE_STRING zw_create_file = RTL_CONSTANT_STRING(L"ZwCreateFile");
	UNICODE_STRING zw_close_file = RTL_CONSTANT_STRING(L"ZwClose");
    UNICODE_STRING iof_call_driver = RTL_CONSTANT_STRING(L"IofCallDriver");
	void *pt_create = NULL, *pt_close = NULL, *pt_iof_call_driver = NULL;
	if(hsOriginalZwCreateFile != NULL) 
	{
		pt_create = MmGetSystemRoutineAddress(&zw_create_file);
		shSsdtHook(pt_create, hsOriginalZwCreateFile, NULL);
	}
	if(hsOriginalZwClose != NULL)
	{
		pt_close = MmGetSystemRoutineAddress(&zw_close_file);
		shSsdtHook(pt_close, hsOriginalZwClose, NULL);
	}
    if(hsIofCallDriver != NULL)
    {
        pt_iof_call_driver = MmGetSystemRoutineAddress(&iof_call_driver);
        xtblHook(pt_iof_call_driver, hsOriginalIofCallDriver, NULL);
    }

	// 即便是卸载了hook,也不排除一些对这两个函数的调用还在进行中。所以
	// 这个时候立刻卸载驱动，可能导致正在执行的代码被卸载掉了，结果会导致
	// 蓝屏。这种问题没有什么特别好的解决方案，一般而言就是睡眠一段时间
	// 来让那些函数都执行完毕。这里的5000意味着5秒。
	hsSleep(5000);
}

NTSTATUS
DriverEntry(
             IN PDRIVER_OBJECT   driver,
             IN PUNICODE_STRING  reg_path)
{
	UNICODE_STRING zw_create_file = RTL_CONSTANT_STRING(L"ZwCreateFile");
	UNICODE_STRING zw_close_file = RTL_CONSTANT_STRING(L"ZwClose");
    UNICODE_STRING iof_call_driver = RTL_CONSTANT_STRING(L"IofCallDriver");
	void *pt_create = NULL, *pt_close = NULL, *pt_iof_call_driver = NULL;
	void *pt = NULL;
	NTSTATUS status = STATUS_UNSUCCESSFUL;

	KdBreakPoint();

	pt_create = MmGetSystemRoutineAddress(&zw_create_file);
	pt_close = MmGetSystemRoutineAddress(&zw_close_file);
    pt_iof_call_driver = MmGetSystemRoutineAddress(&iof_call_driver);

	ASSERT(pt_create != NULL);
	ASSERT(pt_close != NULL);

	do {
		// 如果这两个函数的地址都拿不到，那就直接返回失败即可
		if(pt_create == NULL ||
			pt_close == NULL)
		{
			break;
		}
		
		// 地址拿到之后就开始做hook。
		if(shSsdtHook(pt_create, hsZwCreateFile, (void **)&hsOriginalZwCreateFile) == NULL)
		{
			// 如果hook失败的话，返回失败。
			break;
		}

		// 到这里第一个hook成功，还有第二个
		// 地址拿到之后就开始做hook。
		if(shSsdtHook(pt_close, hsZwClose, (void **)&hsOriginalZwClose) == NULL)
		{
			// 如果hook失败的话，返回失败。返回之前恢复第一个hook.
			pt_create = MmGetSystemRoutineAddress(&zw_create_file);
			shSsdtHook(pt_create, hsOriginalZwCreateFile, NULL);
			break;
		}
		status = STATUS_SUCCESS;
        if(xtblHook(pt_iof_call_driver, hsIofCallDriver, (void **)&hsOriginalIofCallDriver) == NULL)
        {
			// 如果hook失败的话，返回失败。返回之前恢复第一,二个hook.
			pt_create = MmGetSystemRoutineAddress(&zw_create_file);
			shSsdtHook(pt_create, hsOriginalZwCreateFile, NULL);
			pt_close = MmGetSystemRoutineAddress(&zw_close_file);
			shSsdtHook(pt_close, hsOriginalZwClose, NULL);
            break;
        }
	} while(0);
	driver->DriverUnload = hsDriverUnload;
	return status;
}