
// ControllerDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "controller.h"
#include "ControllerDlg.h"
#include "afxdialogex.h"
#include <sstream>
#include <vector>
#include "ImCharset.h"
#include "AddFanDlg.h"
#include "PushLogDlg.h"
#include "SettingManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_TCPCLIENT_DATA_ARRIVE WM_USER+1

#define TIMERID_KEEPALIVE  1000

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CControllerDlg 对话框



CControllerDlg::CControllerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CONTROLLER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	srand((unsigned)time(0));
	m_id = GenerateString(10);
}

void CControllerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_ACCOUNT_COUNT, m_accountCountEdit);
	DDX_Control(pDX, IDC_EDIT_FANMIN, m_fanMinEdit);
	DDX_Control(pDX, IDC_EDIT_FANMAX, m_fanMaxEdit);
	DDX_Control(pDX, IDC_EDIT_MACHINE_CODE, m_machineCodeEdit);
}

BEGIN_MESSAGE_MAP(CControllerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()	
	ON_BN_CLICKED(IDC_BUTTON_ACCOUNT_ERROR, &CControllerDlg::OnBnClickedButtonAccountError)
	ON_BN_CLICKED(IDC_BUTTON_ADD_FAN, &CControllerDlg::OnBnClickedButtonAddFan)	
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CControllerDlg 消息处理程序

BOOL CControllerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标	

	m_tcpClient.SetHost(CImCharset::UnicodeToUTF8(CSettingManager::Get()->m_serverAddr.c_str()).c_str());
	m_tcpClient.SetPort(51234);
	m_tcpClient.SetCallback(this);
	m_tcpClient.Start();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CControllerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

void CControllerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

HCURSOR CControllerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CControllerDlg::OnClose()
{
	m_tcpClient.Stop();

	__super::OnClose();
}

void CControllerDlg::OnConnected()
{
	SendIdentifier();

	SetTimer(TIMERID_KEEPALIVE, 5000, nullptr);
}

void CControllerDlg::OnDataArrive(const std::string& data)
{
	std::string* lparam = new std::string(data);
	PostMessage(WM_TCPCLIENT_DATA_ARRIVE, 0, (LPARAM)lparam);
}

BOOL CControllerDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_TCPCLIENT_DATA_ARRIVE)
	{
		std::string* data = (std::string*)pMsg->lParam;
		DataArrive(*data);
		delete data;		
		return TRUE;
	}

	return __super::PreTranslateMessage(pMsg);
}

void CControllerDlg::DataArrive(const std::string& data)
{	
}

void CControllerDlg::SendIdentifier()
{
	if (!m_tcpClient.IsConnected())
	{
		return;
	}

	std::wstringstream formattedTime;
	formattedTime << L"cmd=identify,ctype=controller,id=" << m_id;
	std::wstring data = formattedTime.str();
	m_tcpClient.SendData(CImCharset::UnicodeToUTF8(data.c_str()));
}

void CControllerDlg::SendKeepAlive()
{
	if (!m_tcpClient.IsConnected())
	{
		return;
	}

	std::wstringstream formattedTime;
	formattedTime << L"cmd=keep_alive,id=" << m_id;
	std::wstring data = formattedTime.str();
	m_tcpClient.SendData(CImCharset::UnicodeToUTF8(data.c_str()));
}

std::wstring CControllerDlg::GenerateString(int length)
{
	std::wstring randomString;
	for (int i = 0; i < 10; i++) {
		wchar_t randomChar = rand() % 36; // Generate a random number between 0 and 35
		if (randomChar < 10) {
			randomChar += L'0'; // Convert the number to a digit
		}
		else {
			randomChar = L'a' + (randomChar - 10); // Convert the number to a letter
		}
		randomString += randomChar;
	}
	return randomString;
}

void CControllerDlg::OnBnClickedButtonAccountError()
{
	CString countString;
	m_accountCountEdit.GetWindowText(countString);
	if (countString.IsEmpty())
	{
		MessageBox(L"请先输入异常账号数", L"提示", MB_OK);
		return;
	}

	if (!m_tcpClient.IsConnected())
	{
		MessageBox(L"控制端未连接", L"提示", MB_OK);
		return;
	}

	CString machineCode = GetMachineCode();

	CString data;
	data.Format(L"cmd=push,id=%s,type=changestatus,count=%s", machineCode, countString);
	m_tcpClient.SendData(CImCharset::UnicodeToUTF8(data));

	MessageBox(L"账号状态控制成功", L"提示", MB_OK);
}

void CControllerDlg::OnBnClickedButtonAddFan()
{
	CString fanMin;
	CString fanMax;
	m_fanMinEdit.GetWindowText(fanMin);
	m_fanMaxEdit.GetWindowText(fanMax);
	if (fanMin.IsEmpty() || fanMax.IsEmpty() || _wtoi(fanMin) > _wtoi(fanMax))
	{
		MessageBox(L"请填写正确加粉数", L"提示", MB_OK);
		return;
	}

	if (!m_tcpClient.IsConnected())
	{
		MessageBox(L"控制端未连接", L"提示", MB_OK);
		return;
	}

	CString machineCode = GetMachineCode();

	CString data;
	data.Format(L"cmd=push,id=%s,type=addfan,min=%s,max=%s", machineCode, fanMin, fanMax);
	m_tcpClient.SendData(CImCharset::UnicodeToUTF8(data));

	MessageBox(L"加粉控制成功", L"提示", MB_OK);
}

CString CControllerDlg::GetMachineCode()
{
	CString machineCode;
	m_machineCodeEdit.GetWindowText(machineCode);
	return machineCode.MakeLower();
}

void CControllerDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == TIMERID_KEEPALIVE)
	{
		SendKeepAlive();
		return;
	}

	__super::OnTimer(nIDEvent);
}
