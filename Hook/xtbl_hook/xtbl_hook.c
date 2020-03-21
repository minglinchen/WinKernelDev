///
/// @file xtbl_hook.c
/// @author tanwen
/// @date 2012-7-4
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
#include "xtbl_hook.h"

#ifdef _WIN64
#error "Only support 32 bit!"
#endif

// 提供给用户使用的函数。输入一个new_function的指针，将会替代原来的函数的指针。
// 但是用户必须自己去找到要hook的函数的地址。
void* xtblHook(void *func_to_hook,void* new_func, void **old_func)
{
	PBYTE func_body = (PBYTE)func_to_hook;
    PVOID *target_addr = *(PVOID *)(func_body + 2);
    PVOID ret = NULL;

    // 这个函数仅仅支持32位
    ASSERT(sizeof(PVOID) == 4);

    // 能这样做的函数是有特点的，必须要满足形似下面的函数：
    // nt!IofCallDriver:
    // 804e47c5 ff2500375580    jmp     dword ptr [nt!pIofCallDriver (80553700)]
    if(func_body[0] != 0xff || func_body[1] != 0x25)
    {
        KdPrint(("xtbl: Not an export function.\r\n"));
        return NULL;
    }

    // 接下来，现有的函数应该是一个内核地址。这是另一重检查。
    // 本来，这应该是一个内核函数的地址。
    if((ULONG)*target_addr < 0x80000000)
    {
        KdPrint(("xtbl: Not an export function.\r\n"));
        return NULL;
    }

    if((ULONG)new_func <= 0x80000000)
    {
        KdPrint(("xtbl: Not an good new function.\r\n"));
        return NULL;
    }

    // 现在确定是了（不完全确定），先取得
    ret = *target_addr;

    if(old_func != NULL)
        *old_func = ret;

    // 交换之后就hook上了。所以在这之前一定要设置好旧的。
	InterlockedExchange((PLONG)target_addr,
		(LONG)new_func);
	return ret;
}

void* xtblHookWithName(UNICODE_STRING *func_name,void* new_func, void **old_func)
{
    void *func_pt = MmGetSystemRoutineAddress(func_name);
    if(func_pt == NULL) 
    {
        KdPrint(("xtblHookWithName: failed to get the function address from %wZ\r\n", func_name));
        return NULL;
    }
    return xtblHook(func_pt, new_func, old_func);
}
