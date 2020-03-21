///
/// @file xtbl_hook.h
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
///

#ifndef _XTBL_HOOK_HEADER_
#define _XTBL_HOOK_HEADER_

#ifdef __cplusplus
extern "C" {
#endif

// 提供给用户使用的函数。输入一个new_function的指针，将会替代原来的函数的指针。
// 但是用户必须自己去找到要hook的函数的地址。
void* xtblHook(void *function,void* new_function, void **old_func);

// 和上边的函数一样，只是用的是函数名。
void* xtblHookWithName(UNICODE_STRING *func_name,void* new_func, void **old_func);


#ifdef __cplusplus
}
#endif

#endif // _XTBL_HOOK_HEADER_