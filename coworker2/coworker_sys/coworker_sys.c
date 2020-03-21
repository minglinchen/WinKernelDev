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

// 定义一个链表用来保存字符串
#define CWK_STR_LEN_MAX 512
typedef struct {
    LIST_ENTRY list_entry;
    char buf[CWK_STR_LEN_MAX];
} CWK_STR_NODE;

// 还必须有一把自旋锁来保证链表操作的安全性
KSPIN_LOCK g_cwk_lock;
// 一个事件来标识是否有字符串可以取
KEVENT  g_cwk_event;
// 必须有个链表头
LIST_ENTRY g_cwk_str_list;

#define MEM_TAG 'cwkr'

// 分配内存并初始化一个链表节点
CWK_STR_NODE *cwkMallocStrNode()
{
    CWK_STR_NODE *ret = ExAllocatePoolWithTag(
        NonPagedPool, sizeof(CWK_STR_NODE), MEM_TAG);
    if(ret == NULL)
        return NULL;
    return ret;
}

void cwkUnload(PDRIVER_OBJECT driver)
{
	UNICODE_STRING cdo_syb = RTL_CONSTANT_STRING(CWK_CDO_SYB_NAME);
    CWK_STR_NODE *str_node;
    ASSERT(g_cdo != NULL);
   	IoDeleteSymbolicLink(&cdo_syb);
    IoDeleteDevice(g_cdo);

    // 负责的编程态度：释放分配过的所有内核内存。
    while(TRUE)
    {
        str_node = (CWK_STR_NODE *)ExInterlockedRemoveHeadList(
            &g_cwk_str_list, &g_cwk_lock);
        // str_node = RemoveHeadList(&g_cwk_str_list);
        if(str_node != NULL)
            ExFreePool(str_node);
        else
            break;
    };
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
            CWK_STR_NODE *str_node;
		    switch(irpsp->Parameters.DeviceIoControl.IoControlCode)
		    {
            case CWK_DVC_SEND_STR:

                ASSERT(buffer != NULL);
                ASSERT(outlen == 0);

                // 安全的编程态度之一:检查输入缓冲的长度对于长度超出预期的，果
                // 断返回错误。
                if(inlen > CWK_STR_LEN_MAX)
                {
                    status = STATUS_INVALID_PARAMETER;
                    break;                    
                }

                // 安全的编程态度之二：检查字符串的长度，不要使用strlen!如果使
                // 用strlen,一旦攻击者故意输入没有结束符的字符串，会导致内核驱
                // 动访问非法内存空间而崩溃。
                DbgPrint("strnlen = %d\r\n", strnlen((char *)buffer, inlen));
                if(strnlen((char *)buffer, inlen) == inlen)
                {
                    // 字符串占满了缓冲区，且中间没有结束符。立刻返回错误。
                    status = STATUS_INVALID_PARAMETER;
                    break;                    
                }

                // 现在可以认为输入缓冲是安全而且不含恶意的。分配节点。
                str_node = cwkMallocStrNode();
                if(str_node == NULL)
                {
                    // 如果分配不到空间了，返回资源不足的错误
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    break;
                }

                // 前面已经检查了缓冲区中的字符串的确长度合适而且含有结束符
                // ，所以这里用什么函数来拷贝字符串对安全性而言并不非常重要。
                strncpy(str_node->buf,(char *)buffer, CWK_STR_LEN_MAX); 
                // 插入到链表末尾。用锁来保证安全性。
                ExInterlockedInsertTailList(&g_cwk_str_list, (PLIST_ENTRY)str_node, &g_cwk_lock);
                // InsertTailList(&g_cwk_str_list, (PLIST_ENTRY)str_node);
                // 打印
                // DbgPrint((char *)buffer);
                // 那么现在就可以认为这个请求已经成功。因为刚刚已经插入了一
                // 个，那么可以设置事件来表明队列中已经有元素了。
                KeSetEvent(&g_cwk_event, 0, TRUE);
                break;
            case CWK_DVC_RECV_STR:
                ASSERT(buffer != NULL);
                ASSERT(inlen == 0);
                // 应用要求接收字符串。对此，安全上要求是输出缓冲要足够长。
                if(outlen < CWK_STR_LEN_MAX)
                {
                    status = STATUS_INVALID_PARAMETER;
                    break;                    
                }
                while(1) 
                {
                    // 插入到链表末尾。用锁来保证安全性。
                    str_node = (CWK_STR_NODE *)ExInterlockedRemoveHeadList(&g_cwk_str_list, &g_cwk_lock);
                    // str_node = RemoveHeadList(&g_cwk_str_list);
                    if(str_node != NULL)
                    {
                        // 这种情况下，取得了字符串。那就拷贝到输出缓冲中。然后
                        // 整个请求就返回了成功。
                        strncpy((char *)buffer, str_node->buf, CWK_STR_LEN_MAX);
                        ret_len = strnlen(str_node->buf, CWK_STR_LEN_MAX) + 1;
                        ExFreePool(str_node);
                        break;
                    }
                    else
                    {
                        // 对于合法的要求，在缓冲链表为空的情况下，等待事件进行
                        // 阻塞。也就是说，如果缓冲区中没有字符串，就停下来等待
                        // 。这样应用程序也会被阻塞住，DeviceIoControl是不会返回
                        // 的。但是一旦有就会返回。等于驱动“主动”通知了应用。
                        KeWaitForSingleObject(&g_cwk_event,Executive,KernelMode,0,0);
                    }
                }
                break;
            default:
                // 到这里的请求都是不接受的请求。未知的请求一律返回非法参数错误。
                status = STATUS_INVALID_PARAMETER;
                break;
            }
        }
        break;
    }
    // 返回结果
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

    // 初始化事件、锁、链表头。
    KeInitializeEvent(&g_cwk_event,SynchronizationEvent,TRUE); 
    KeInitializeSpinLock(&g_cwk_lock);
    InitializeListHead(&g_cwk_str_list);

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
