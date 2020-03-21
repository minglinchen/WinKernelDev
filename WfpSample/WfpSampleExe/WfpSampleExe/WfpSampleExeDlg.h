// WfpSampleExeDlg.h : header file
//

#pragma once
#include "winioctl.h"

// CWfpSampleExeDlg dialog
class CWfpSampleExeDlg : public CDialog
{
// Construction
public:
	CWfpSampleExeDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_WFPSAMPLEEXE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
};

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
#pragma pack(pop)

#define IOCTL_WFP_SAMPLE_ADD_RULE CTL_CODE(FILE_DEVICE_UNKNOWN,0x801,METHOD_BUFFERED,FILE_READ_ACCESS | FILE_WRITE_ACCESS)