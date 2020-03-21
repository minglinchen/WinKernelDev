#include "ntddk.h"


/*函数原型声明*/
void  DriverUnload(__in struct _DRIVER_OBJECT  *DriverObject);


BOOLEAN g_bSuccRegister = FALSE;
VOID LOAD_IMAGE_NOTIFY_ROUTINE(IN PUNICODE_STRING  FullImageName,
							   IN HANDLE  ProcessId, // where image is mapped
							   IN PIMAGE_INFO  ImageInfo
							   );



NTSTATUS DriverEntry( __in struct _DRIVER_OBJECT* DriverObject, __in PUNICODE_STRING RegistryPath )
{
	NTSTATUS nStatus = STATUS_UNSUCCESSFUL;
	
	do 
	{
		DriverObject->DriverUnload = DriverUnload;
		if( STATUS_SUCCESS != PsSetLoadImageNotifyRoutine(LOAD_IMAGE_NOTIFY_ROUTINE) )
		{
			break;
		}
		g_bSuccRegister = TRUE;
		nStatus = STATUS_SUCCESS;
		
	} 
	while (FALSE);
	return nStatus;
}

VOID LOAD_IMAGE_NOTIFY_ROUTINE(IN PUNICODE_STRING  FullImageName,
							   IN HANDLE  ProcessId, // where image is mapped
							   IN PIMAGE_INFO  ImageInfo
							   )
{
	PIMAGE_INFO_EX pInfo = NULL;
	if( !FullImageName || !ImageInfo )
	{
		return;
	}
	if( ImageInfo->ExtendedInfoPresent )
	{
		pInfo = CONTAINING_RECORD(ImageInfo,IMAGE_INFO_EX,ImageInfo);
		DbgPrint("ModLoad Name = %wZ,ProcessID = 0x%x,FileObj = 0x%x,ImageBase = 0x%x,Size = 0x%x\n",
			FullImageName,ProcessId,pInfo->FileObject,pInfo->ImageInfo.ImageBase,pInfo->ImageInfo.ImageSize);
	}
	return;
}

void  DriverUnload(__in struct _DRIVER_OBJECT  *DriverObject)
{
	if( g_bSuccRegister )
	{
		PsRemoveLoadImageNotifyRoutine(LOAD_IMAGE_NOTIFY_ROUTINE);
		g_bSuccRegister = FALSE;
	}
}