///
/// @file sh_ssdt_hook.h
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
///

#ifndef _SH_SSDTHOOK_HEADER_
#define _SH_SSDTHOOK_HEADER_

#ifdef __cplusplus
extern "C" {
#endif

// 提供给用户使用的函数。输入一个new_function的指针，将会替代原来的函数的指针。
// 但是用户必须自己去找到要hook的函数的地址。
void* shSsdtHook(
		 void *func_to_hook,
		 void *new_func,
         void **old_func);

// 这个结构实际未公开。在这里自己定义。
typedef struct ServiceDescriptorEntry {
	unsigned int *ServiceTableBase;
	unsigned int *ServiceCounterTableBase;
	unsigned int NumberOfServices;
	unsigned char *ParamTableBase;
} ServiceDescriptorTableEntry_t, *PServiceDescriptorTableEntry_t;

// 导入SSDT的符号
__declspec(dllimport)  ServiceDescriptorTableEntry_t KeServiceDescriptorTable;

// 几个函数的定义。如果要做Hook的话，这些函数的类型定义是能用上的。
typedef NTSTATUS (*SH_ZW_CREATE_FILE_F)(
	OUT PHANDLE  FileHandle,
    IN ACCESS_MASK  DesiredAccess,
    IN POBJECT_ATTRIBUTES  ObjectAttributes,
    OUT PIO_STATUS_BLOCK  IoStatusBlock,
    IN PLARGE_INTEGER  AllocationSize  OPTIONAL,
    IN ULONG  FileAttributes,
    IN ULONG  ShareAccess,
    IN ULONG  CreateDisposition,
    IN ULONG  CreateOptions,
    IN PVOID  EaBuffer  OPTIONAL,
    IN ULONG  EaLength
);

typedef NTSTATUS (*SH_ZW_CREATE_DIRECTORY_OBJECT_F)(
    OUT PHANDLE  DirectoryHandle,
    IN ACCESS_MASK  DesiredAccess,
    IN POBJECT_ATTRIBUTES  ObjectAttributes
);

typedef NTSTATUS (*SH_ZW_CLOSE_F)(
    IN HANDLE  Handle
);

typedef NTSTATUS (*SH_ZW_READ_FILE_F)(
    IN HANDLE  FileHandle,
    IN HANDLE  Event  OPTIONAL,
    IN PIO_APC_ROUTINE  ApcRoutine  OPTIONAL,
    IN PVOID  ApcContext  OPTIONAL,
    OUT PIO_STATUS_BLOCK  IoStatusBlock,
    OUT PVOID  Buffer,
    IN ULONG  Length,
    IN PLARGE_INTEGER  ByteOffset  OPTIONAL,
    IN PULONG  Key  OPTIONAL
    );

typedef NTSTATUS (*SH_ZW_WRITE_FILE_F)(
    IN HANDLE  FileHandle,
    IN HANDLE  Event  OPTIONAL,
    IN PIO_APC_ROUTINE  ApcRoutine  OPTIONAL,
    IN PVOID  ApcContext  OPTIONAL,
    OUT PIO_STATUS_BLOCK  IoStatusBlock,
    IN PVOID  Buffer,
    IN ULONG  Length,
    IN PLARGE_INTEGER  ByteOffset  OPTIONAL,
    IN PULONG  Key  OPTIONAL
);

typedef NTSTATUS (*SH_ZW_SET_INFORMATION_FILE_F)(
    IN HANDLE  FileHandle,
    OUT PIO_STATUS_BLOCK  IoStatusBlock,
    IN PVOID  FileInformation,
    IN ULONG  Length,
    IN FILE_INFORMATION_CLASS  FileInformationClass
);

typedef NTSTATUS (*SH_ZW_DELETE_FILE_F)(
    IN POBJECT_ATTRIBUTES  ObjectAttributes
);

typedef NTSTATUS (*SH_ZW_CREATE_PROCESS_EX_F)(
    OUT PHANDLE ProcessHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN HANDLE ParentProcess,
    IN BOOLEAN InheritObjectTable,
    IN HANDLE SectionHandle OPTIONAL,
    IN HANDLE DebugPort OPTIONAL,
    IN HANDLE ExceptionPort OPTIONAL,
    IN HANDLE Unknown 
);  

typedef NTSTATUS
	(*SH_ZW_OPEN_PROCESS)(
		IN PHANDLE  ProcessHandle,
		IN ACCESS_MASK  DesiredAccess,
		IN POBJECT_ATTRIBUTES  ObjectAttributes,
		IN PCLIENT_ID  ClientId);

#ifdef __cplusplus
}
#endif

#endif 