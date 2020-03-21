#pragma once

#define WFP_TAG	'wfpT'

#pragma pack(push)
#pragma pack(1)
typedef struct _tagWfp_NetInfo
{
	USHORT      m_uSrcPort;	//源端口
	USHORT      m_uRemotePort;	//目标端口
	ULONG       m_ulSrcIPAddr; //源地址
	ULONG       m_ulRemoteIPAddr; //目标地址
	ULONG       m_ulNetWorkType; //协议
	USHORT		m_uDirection;//数据包的方向，0表示发送，1表示接收

} ST_WFP_NETINFO, *PST_WFP_NETINFO;

typedef struct _tagWfp_NetInfoList
{
	LIST_ENTRY		m_linkPointer;
	ST_WFP_NETINFO	m_stWfpNetInfo;

}ST_WFP_NETINFOLIST,*PST_WFP_NETINFOLIST;

#pragma pack(pop)


BOOLEAN InitRuleInfo();

BOOLEAN UninitRuleInfo();

BOOLEAN AddNetRuleInfo(PVOID pRuleInfo,ULONG uLen);

BOOLEAN IsHitRule(USHORT uRemotePort);