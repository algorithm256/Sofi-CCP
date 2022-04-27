
// CCPDemoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CCPDemo.h"
#include "CCPDemoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCCPDemoDlg dialog




CCCPDemoDlg::CCCPDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCCPDemoDlg::IDD, pParent)
	, laSlaveVersion(_T("0.0"))
	, laSlaveInfo(_T("_________"))
	, laSlaveId(_T("_________"))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCCPDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON1, btnConnect);
	DDX_Control(pDX, IDC_BUTTON2, btnDisconnect);
	DDX_Control(pDX, IDC_BUTTON4, btnTest);
	DDX_Control(pDX, IDC_BUTTON3, btnGetVersion);
	DDX_Control(pDX, IDC_BUTTON5, btnExchange);
	DDX_Control(pDX, IDC_BUTTON6, btnGetId);
	DDX_Text(pDX, IDC_EDIT1, laSlaveVersion);
	DDX_Text(pDX, IDC_EDIT2, laSlaveInfo);
	DDX_Text(pDX, IDC_EDIT3, laSlaveId);
}

BEGIN_MESSAGE_MAP(CCCPDemoDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_BTNCONNECT, &CCCPDemoDlg::OnBnClickedBtnconnect)
	ON_BN_CLICKED(IDC_BTNDISCONNECT, &CCCPDemoDlg::OnBnClickedBtndisconnect)
	ON_BN_CLICKED(IDC_BTNTEST, &CCCPDemoDlg::OnBnClickedBtntest)
	ON_BN_CLICKED(IDC_BTNGETVERSION, &CCCPDemoDlg::OnBnClickedBtngetversion)
	ON_BN_CLICKED(IDC_BTNEXCHANGE, &CCCPDemoDlg::OnBnClickedBtnexchange)
	ON_BN_CLICKED(IDC_BTNGETID, &CCCPDemoDlg::OnBnClickedBtngetid)
	ON_EN_CHANGE(IDC_EDIT3, &CCCPDemoDlg::OnEnChangeEdit3)
	ON_BN_CLICKED(IDC_BUTTON7, &CCCPDemoDlg::OnBnClickedButton7)
END_MESSAGE_MAP()


// CCCPDemoDlg message handlers

BOOL CCCPDemoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CCCPDemoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CCCPDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

CString CCCPDemoDlg::GetErrorText(TCCPResult errorCode)
{
	char textBuffer[256] = {0};

	if(CCP_GetErrorText(errorCode, textBuffer) == CCP_ERROR_ACKNOWLEDGE_OK)
		return textBuffer;
	return "";
}

int CCCPDemoDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	m_PccpHandle = 0;

	// PCAN Channel to use
	//
	m_Channel = PCAN_USBBUS1;
	m_Baudrate = PCAN_BAUD_250K;

	// ECU Data
	//
	m_SlaveData.EcuAddress = 0x103;
	m_SlaveData.IdCRO = 0x8CFF50FF;
	m_SlaveData.IdDTO = 0x8CFF5100;
	m_SlaveData.IntelFormat = false;

	return 0;
}

void CCCPDemoDlg::OnClose()
{
	CCP_UninitializeChannel(m_Channel);

	CDialog::OnClose();
}

void CCCPDemoDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	TCCPResult ccpResult;

	CDialog::OnShowWindow(bShow, nStatus);	

	ccpResult = CCP_InitializeChannel(m_Channel, m_Baudrate, 0,0,0);
	if (ccpResult != CCP_ERROR_ACKNOWLEDGE_OK)
	{		
		MessageBox(GetErrorText(ccpResult), "Error");
		btnConnect.EnableWindow(false);
		btnTest.EnableWindow(false);		
	}
}


void CCCPDemoDlg::OnBnClickedBtnconnect()
{
	TCCPResult ccpResult;
	bool bConnected;

	ccpResult = CCP_Connect(m_Channel, &m_SlaveData, &m_PccpHandle, 0);
	bConnected = ccpResult == CCP_ERROR_ACKNOWLEDGE_OK;

	btnConnect.EnableWindow(!bConnected);
	btnDisconnect.EnableWindow(bConnected);
	btnGetVersion.EnableWindow(bConnected);
	btnExchange.EnableWindow(bConnected);
	btnGetId.EnableWindow(false);

	if (!bConnected)
		MessageBox(GetErrorText(ccpResult), "Error");
}

void CCCPDemoDlg::OnBnClickedBtndisconnect()
{
	TCCPResult ccpResult;

	ccpResult = CCP_Disconnect(m_PccpHandle,false, 0);
	m_PccpHandle = 0;

	btnConnect.EnableWindow(true);
	btnDisconnect.EnableWindow(false);
	btnGetVersion.EnableWindow(false);
	btnGetId.EnableWindow(false);
	btnExchange.EnableWindow(false);

	if (ccpResult != CCP_ERROR_ACKNOWLEDGE_OK)
		MessageBox(GetErrorText(ccpResult), "Error");

	laSlaveVersion = "0.0";
	laSlaveId = "_________";
	laSlaveInfo = "_________";
	UpdateData(FALSE);
}

void CCCPDemoDlg::OnBnClickedBtntest()
{
	TCCPResult ccpResult;

	ccpResult = CCP_Test(m_Channel, &m_SlaveData, 0);
	if(ccpResult == CCP_ERROR_ACKNOWLEDGE_OK)
		MessageBox("ECU is available for connect");
	else if(ccpResult == CCP_ERROR_INTERNAL_TIMEOUT)
		MessageBox("ECU NOT available");
	else
		MessageBox(GetErrorText(ccpResult), "Error");
}

void CCCPDemoDlg::OnBnClickedBtngetversion()
{
	TCCPResult ccpResult;
	BYTE mainByte, releaseByte;

	ccpResult = CCP_GetCcpVersion(m_PccpHandle, &mainByte, &releaseByte, 0);

	if (ccpResult != CCP_ERROR_ACKNOWLEDGE_OK)
		MessageBox(GetErrorText(ccpResult), "Error");
	else
	{
		laSlaveVersion.Format("%d.%d", mainByte,releaseByte);
		UpdateData(FALSE);
	}
}

void CCCPDemoDlg::OnBnClickedBtnexchange()
{
	TCCPResult ccpResult;
	BYTE MasterData[4];

	MasterData[0] = 0x78;
	MasterData[1] = 0x56;
	MasterData[2] = 0x34;
	MasterData[3] = 0x12;


	ccpResult = CCP_ExchangeId(m_PccpHandle, &m_ExchangeData, MasterData, 4, 0);

	if (ccpResult != CCP_ERROR_ACKNOWLEDGE_OK)
		MessageBox(GetErrorText(ccpResult), "Error");
	else
	{
		laSlaveInfo.Format("ID Length %d. Data Type %d. Res. Mask %d. Protec. Mask %d. ",
							m_ExchangeData.IdLength,m_ExchangeData.DataType,m_ExchangeData.AvailabilityMask, m_ExchangeData.ProtectionMask);
		btnGetId.EnableWindow(true);
		UpdateData(FALSE);
	}
}

void CCCPDemoDlg::OnBnClickedBtngetid()
{
	TCCPResult ccpResult;
	BYTE IdArray[8];
	CString strTemp;

	ccpResult =  CCP_Upload(m_PccpHandle,m_ExchangeData.IdLength, IdArray, 0);

	if (ccpResult != CCP_ERROR_ACKNOWLEDGE_OK)
		MessageBox(GetErrorText(ccpResult), "Error");
	else
	{
		switch(m_ExchangeData.IdLength)
		{
			case 1:
				laSlaveId.Format("%.2X",IdArray[0]);
				break;
			case 2:
				laSlaveId.Format("%.4X",(int)*PWORD(IdArray));
				break;
			case 4:
				laSlaveId.Format("%.8X", (int)*LPDWORD(IdArray));
				break;
			default:
				laSlaveId = "";
				for(int iCount = 0; iCount < m_ExchangeData.IdLength; iCount++)
				{
					strTemp =  laSlaveId;
					laSlaveId.Format("%.2X%s", IdArray[iCount], strTemp);
				}
		}
		UpdateData(FALSE);
	}
}


void CCCPDemoDlg::OnEnChangeEdit3()
{
	// TODO:  Je¿eli to jest kontrolka RICHEDIT, to kontrolka nie bêdzie
	// wyœlij to powiadomienie, chyba ¿e przes³onisz element CDialog::OnInitDialog()
	// funkcja i wywo³anie CRichEditCtrl().SetEventMask()
	// z flag¹ ENM_CHANGE zsumowan¹ logicznie z mask¹.

	// TODO:  Dodaj tutaj swój kod procedury obs³ugi powiadamiania kontrolki
}

void CCCPDemoDlg::OnBnClickedButton7()
{
	TCCPResult ccpResult;
	bool CurrentStatus = true;
	unsigned int buflen = 4;
	BYTE *Seed;

	Seed = (BYTE*)malloc(buflen * sizeof(BYTE));

	ccpResult = CCP_GetSeed(m_PccpHandle, CCP_RSM_DATA_ADQUISITION, false, Seed, 0);
	free(Seed);
	//Seed = (char*)malloc(buflen * sizeof(char));
	//if (ccpResult != CCP_ERROR_ACKNOWLEDGE_OK)
	//	MessageBox(GetErrorText(ccpResult), "Error");
	//else
	//{
	// 
		//laSlaveVersion.Format(ptrstr);
//		UpdateData(FALSE);
	//}
	//free(ptrstr);
	//return 0;

}
