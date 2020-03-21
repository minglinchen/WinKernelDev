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
#include "sh_ssdt_hook.h"

#if 0

void shDisableWriteProtect(ULONG *old)
{
	wd_ulong cr0_old;
	_asm
	{
		// 取出当前cr0的值放入edi并备份到变量中
		mov edi,cr0 
		mov cr0_old,edi
		// 用各种方法，将edi与上0fffeffffh
		xor edx, edx
		or edx, 0fffe0000h
		push edx
		pop ecx
		or ecx, 00000ffffh
		and edi,ecx;
		cli
		mov cr0,edi;
	};
	*old = cr0_old;
}

void shEnableWriteProtect(ULONG old)
{
	_asm {
		push old
		pop ebx
		mov cr0,ebx
		sti
	};
}

#else 

// 直接了当的版本，可能被杀毒软件直接删除
void shDisableWriteProtect(ULONG *old)
{
	ULONG cr0_old;
	_asm
	{
		cli
		// 取出当前cr0的值放入eax并备份到变量中
		mov eax,cr0 
		mov cr0_old,eax
		// 将eax与上0fffeffffh
		and eax,0fffeffffh
		mov cr0,eax;
	};
	*old = cr0_old;
}

void shEnableWriteProtect(ULONG old)
{
	_asm {
		mov eax,old
		mov cr0,eax
		sti
	};
}

#endif

void* shSsdtHook(
		 void *func_to_hook,
		 void *new_func,
         void **old_func)
{
	ULONG service_id;
	void *function = NULL;
	ULONG cr0_old;
	void* old_function = NULL;

	ASSERT(func_to_hook != NULL);
	ASSERT(new_func != NULL);
	if(func_to_hook == NULL || new_func == NULL)
		return NULL;

	// 获得服务编号
	service_id = *(PULONG)(((PUCHAR)func_to_hook)+1);
	// 获得原有的函数指针。
	old_function = (void *)KeServiceDescriptorTable.ServiceTableBase[service_id];

    // 一定要首先设置原有函数指针，然后再hook。否则可
    // 能出现hook之后原有函数依然是NULL，结果在调用原
    // 有函数的时候crash。
    if(old_func != NULL)
        *old_func = old_function;

	// 最后改写之。在这一句之后，hook就已经起作用了。
	shDisableWriteProtect(&cr0_old);
	KeServiceDescriptorTable.ServiceTableBase[service_id] = (unsigned int)new_func;
	shEnableWriteProtect(cr0_old);

	// 返回旧的。
	return old_function;
}
