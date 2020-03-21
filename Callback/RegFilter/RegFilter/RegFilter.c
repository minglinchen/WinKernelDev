#include "ntddk.h"

#ifndef MAX_PATH
#define MAX_PATH	(260)
#endif

/*函数原型声明*/
void  DriverUnload(__in struct _DRIVER_OBJECT  *DriverObject);

BOOLEAN IsAbsolute(PREG_CREATE_KEY_INFORMATION pCreateInfo);

NTSTATUS  RegistryCallback(__in PVOID  CallbackContext,
						   __in_opt PVOID  Argument1,
						   __in_opt PVOID  Argument2 );

NTSTATUS _stdcall ObQueryNameString( __in PVOID Object,
				  __out_bcount_opt(Length) POBJECT_NAME_INFORMATION ObjectNameInfo,
				  __in ULONG Length,
				  __out PULONG ReturnLength);

UCHAR * _stdcall PsGetProcessImageFileName(__in PEPROCESS Process);

LARGE_INTEGER	g_CmCallbackCookies = {0};
BOOLEAN			g_bSuccRegister = FALSE;

NTSTATUS DriverEntry( __in struct _DRIVER_OBJECT* DriverObject, __in PUNICODE_STRING RegistryPath )
{
	NTSTATUS nStatus = STATUS_UNSUCCESSFUL;
	do 
	{
		DriverObject->DriverUnload = DriverUnload;
		if( STATUS_SUCCESS != CmRegisterCallback(RegistryCallback,NULL,&g_CmCallbackCookies) )
		{
			break;
		}
		g_bSuccRegister = TRUE;
		nStatus = STATUS_SUCCESS;
	} 
	while (FALSE);
	return nStatus;
}


NTSTATUS  RegistryCallback(
				 __in PVOID  CallbackContext,
				 __in_opt PVOID  Argument1,
				 __in_opt PVOID  Argument2 )
{
	switch((REG_NOTIFY_CLASS)Argument1)
	{
	case RegNtPreCreateKey:
		{
			CHAR *pProcessName = PsGetProcessImageFileName(PsGetCurrentProcess());
			PREG_PRE_CREATE_KEY_INFORMATION pCreateInfo = (PREG_PRE_CREATE_KEY_INFORMATION)Argument2;
			DbgPrint("RegFilter ProcessName = %s,CreateKey:%wZ\n",pProcessName,pCreateInfo->CompleteName);
			break;
		}
	case RegNtPreCreateKeyEx:
		{
			CHAR *pProcessName = PsGetProcessImageFileName(PsGetCurrentProcess());
			PREG_CREATE_KEY_INFORMATION pCreateInfo = (PREG_CREATE_KEY_INFORMATION)Argument2;
			/*判断是否绝对路径*/
			if( IsAbsolute( pCreateInfo ) )
			{
				/*绝对路径*/
				DbgPrint("RegFilter ProcessName = %s,CreateKeyEx:%wZ\n",pProcessName,pCreateInfo->CompleteName);
			}
			else
			{
				CHAR strrRootPath[MAX_PATH] = {0};
				ULONG uReturnLen = 0;
				POBJECT_NAME_INFORMATION pNameInfo = (POBJECT_NAME_INFORMATION)strrRootPath;
				if( pCreateInfo->RootObject != NULL )
				{
					ObQueryNameString(pCreateInfo->RootObject,pNameInfo,sizeof(strrRootPath),&uReturnLen);
				}
				DbgPrint("RegFilter ProcessName = %s,CreateKeyEx:%wZ\\%wZ\n",pProcessName,&(pNameInfo->Name),pCreateInfo->CompleteName);

			}
			break;
		} 
	}
	return STATUS_SUCCESS;
}

void  DriverUnload(__in struct _DRIVER_OBJECT  *DriverObject)
{
	if( g_bSuccRegister == TRUE )
	{
		CmUnRegisterCallback(g_CmCallbackCookies);
		g_bSuccRegister = FALSE;
	}
	return;
}

BOOLEAN IsAbsolute(PREG_CREATE_KEY_INFORMATION pCreateInfo)
{
	BOOLEAN bAbsolute = FALSE;
	do 
	{
		if( pCreateInfo == NULL )
		{
			break;
		}
		if( !pCreateInfo->CompleteName || !pCreateInfo->CompleteName->Buffer || !pCreateInfo->CompleteName->Length )
		{
			break;
		}
		if( pCreateInfo->CompleteName->Buffer[0] != L'\\' )
		{
			/*相对路径*/
			break;
		}
		/*绝对路径*/
		bAbsolute = TRUE;
	} 
	while (FALSE);
	return bAbsolute;
}