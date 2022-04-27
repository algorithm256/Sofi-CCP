
// CCPDemoDlg.h : header file
//

#pragma once
#include "afxwin.h"

#include "PCCP.h"

// CCCPDemoDlg dialog
class CCCPDemoDlg : public CDialog
{
// Construction
public:
	CCCPDemoDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_CCPDEMO_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

private:
	TCCPHandle m_PccpHandle;
	TPCANHandle m_Channel;
	TPCANBaudrate m_Baudrate;
	TCCPSlaveData m_SlaveData;
	TCCPExchangeData m_ExchangeData;

	CString GetErrorText(TCCPResult errorCode);

public:
	CButton btnConnect;
	CButton btnDisconnect;
	CButton btnTest;
	CButton btnGetVersion;
	CButton btnExchange;
	CButton btnGetId;
	CString laSlaveVersion;
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	CString laSlaveInfo;
	CString laSlaveId;
	afx_msg void OnClose();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnBnClickedBtnconnect();
	afx_msg void OnBnClickedBtndisconnect();
	afx_msg void OnBnClickedBtntest();
	afx_msg void OnBnClickedBtngetversion();
	afx_msg void OnBnClickedBtnexchange();
	afx_msg void OnBnClickedBtngetid();
	afx_msg void OnEnChangeEdit3();
	afx_msg void OnBnClickedButton7();
};
