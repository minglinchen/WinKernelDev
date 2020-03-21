#pragma once


#ifndef MAX_PATH
#define MAX_PATH	(260)
#endif

#define WFP_DEVICE_NAME				L"\\Device\\wfp_sample_device"

#define WFP_SYM_LINK_NAME			L"\\DosDevices\\wfp_sample_device"

#define WFP_SAMPLE_ESTABLISHED_CALLOUT_DISPLAY_NAME	L"WfpSampleEstablishedCalloutName"

#define WFP_SAMPLE_SUB_LAYER_DISPLAY_NAME	L"WfpSampleSubLayerName"

#define WFP_SAMPLE_FILTER_ESTABLISH_DISPLAY_NAME	L"WfpSampleFilterEstablishName"

#define HTTP_DEFAULT_PORT	80

#define IOCTL_WFP_SAMPLE_ADD_RULE CTL_CODE(FILE_DEVICE_UNKNOWN,0x801,METHOD_BUFFERED,FILE_READ_ACCESS | FILE_WRITE_ACCESS)

// {D969FC67-6FB2-4504-91CE-A97C3C32AD36}
DEFINE_GUID(WFP_SAMPLE_ESTABLISHED_CALLOUT_V4_GUID, 0xd969fc67, 0x6fb2, 0x4504, 0x91, 0xce, 0xa9, 0x7c, 0x3c, 0x32, 0xad, 0x36);

// {ED6A516A-36D1-4881-BCF0-ACEB4C04C21C}
DEFINE_GUID(WFP_SAMPLE_SUBLAYER_GUID, 
			0xed6a516a, 0x36d1, 0x4881, 0xbc, 0xf0, 0xac, 0xeb, 0x4c, 0x4, 0xc2, 0x1c);



/*函数原型声明*/
void  DriverUnload(__in struct _DRIVER_OBJECT  *DriverObject);

PDEVICE_OBJECT	CreateDevice( __in struct _DRIVER_OBJECT* DriverObject );

NTSTATUS WfpSampleIRPDispatch(IN PDEVICE_OBJECT DeviceObject,IN PIRP Irp);

NTSTATUS WfpRegisterCalloutImple(
								  IN OUT void* deviceObject,
								  IN  FWPS_CALLOUT_CLASSIFY_FN ClassifyFunction,
								  IN  FWPS_CALLOUT_NOTIFY_FN NotifyFunction,
								  IN  FWPS_CALLOUT_FLOW_DELETE_NOTIFY_FN FlowDeleteFunction,
								  IN  GUID const* calloutKey,
								  IN  UINT32 flags,
								  OUT UINT32* calloutId
								  );
NTSTATUS WfpRegisterCallouts( IN OUT void* deviceObject );

VOID NTAPI Wfp_Sample_Established_ClassifyFn_V4(
	IN const FWPS_INCOMING_VALUES0  *inFixedValues,
	IN const FWPS_INCOMING_METADATA_VALUES0  *inMetaValues,
	IN OUT VOID  *layerData,
	IN OPTIONAL const void  *classifyContext,
	IN const FWPS_FILTER1  *filter,
	IN UINT64  flowContext,
	OUT FWPS_CLASSIFY_OUT0  *classifyOut
	);


NTSTATUS NTAPI Wfp_Sample_Established_NotifyFn_V4( IN FWPS_CALLOUT_NOTIFY_TYPE notifyType, IN const GUID* filterKey, IN const FWPS_FILTER* filter);

VOID NTAPI Wfp_Sample_Established_FlowDeleteFn_V4( IN UINT16 layerId, IN UINT32 calloutId, IN UINT64 flowContext );


NTSTATUS WfpAddCallouts();

NTSTATUS WfpRegisterCallouts(IN OUT void* deviceObject);

NTSTATUS WfpRegisterCalloutImple(
								 IN OUT void* deviceObject,
								 IN  FWPS_CALLOUT_CLASSIFY_FN ClassifyFunction,
								 IN  FWPS_CALLOUT_NOTIFY_FN NotifyFunction,
								 IN  FWPS_CALLOUT_FLOW_DELETE_NOTIFY_FN FlowDeleteFunction,
								 IN  GUID const* calloutKey,
								 IN  UINT32 flags,
								 OUT UINT32* calloutId
								 );

NTSTATUS WfpAddSubLayer();

NTSTATUS WfpAddFilters();


VOID WfpUnRegisterCallouts();

VOID WfpRemoveCallouts();

VOID WfpRemoveSubLayer();

VOID WfpRemoveFilters();

HANDLE OpenEngine();

void CloseEngine();

NTSTATUS InitWfp();

VOID UninitWfp();

VOID DeleteDevice();








