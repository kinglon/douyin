
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

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_TCPCLIENT_DATA_ARRIVE WM_USER+1

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
	m_id = GenerateString(10);
}

void CControllerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_CONTROL_ALL, m_controlAllCheckBox);
	DDX_Control(pDX, IDC_LIST_CLIENT, m_clientList);
}

BEGIN_MESSAGE_MAP(CControllerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_NOTIFY(NM_RCLICK, IDC_LIST_CLIENT, &CControllerDlg::OnNMRClickListClient)
	ON_COMMAND(ID_ACCOUNT_OK, &CControllerDlg::OnAccountOk)
	ON_COMMAND(ID_ACCOUNT_ERROR, &CControllerDlg::OnAccountError)
	ON_COMMAND(ID_ADD_FAN, &CControllerDlg::OnAddFan)
	ON_COMMAND(ID_FOLLOW, &CControllerDlg::OnFollow)
	ON_COMMAND(ID_REPLY, &CControllerDlg::OnReply)
	ON_COMMAND(ID_PRIVATE_CHAT, &CControllerDlg::OnPrivateChat)
	ON_COMMAND(ID_COMMENT, &CControllerDlg::OnComment)
	ON_COMMAND(ID_PUSH_LOG, &CControllerDlg::OnPushLog)
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

	m_clientList.InsertColumn(0, _T("ID"), LVCFMT_CENTER, 200);
	m_clientList.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	m_tcpClient.SetHost("127.0.0.1");
	m_tcpClient.SetPort(80);
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

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

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
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
	m_tcpClient.SendData("cmd=getdouyinclients");
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
	std::map<std::string, std::string> result;
	std::istringstream ss(data);
	std::string token;

	while (std::getline(ss, token, ','))
	{
		size_t pos = token.find('=');
		if (pos != std::string::npos)
		{
			std::string key = token.substr(0, pos);
			std::string value = token.substr(pos + 1);
			result[key] = value;
		}
	}

	if (result.find("cmd") == result.end())
	{
		return;
	}

	std::string cmd = result["cmd"];
	if (cmd == "douyinclientchange")
	{
		m_tcpClient.SendData("cmd=getdouyinclients");		
	}
	else if (cmd == "getdouyinclients")
	{
		std::vector<std::wstring> clients;
		if (result.find("clients") != result.end() && !result["clients"].empty())
		{
			std::stringstream clientstream(result["clients"]);
			std::string token;
			while (std::getline(ss, token, '_')) 
			{
				clients.push_back(CImCharset::UTF8ToUnicode(token.c_str()));
			}
		}		
		UpdateClientListUI(clients);
	}
}

void CControllerDlg::UpdateClientListUI(const std::vector<std::wstring>& clients)
{
	m_clientList.DeleteAllItems();
	for (unsigned i=0; i<clients.size(); i++)
	{
		m_clientList.InsertItem(i, clients[i].c_str());
	}
}

void CControllerDlg::OnNMRClickListClient(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;

	int nCheck = m_controlAllCheckBox.GetCheck();
	if (nCheck == BST_CHECKED)
	{
		m_clientToControl = L"";
	}
	else
	{
		int selectIndex = m_clientList.GetNextItem(-1, LVNI_SELECTED);
		if (selectIndex < 0)
		{
			MessageBox(L"请选择一个客户端", L"提示", MB_OK);
			return;
		}

		m_clientToControl = m_clientList.GetItemText(selectIndex, 0);
	}

	CMenu menu;
	menu.LoadMenu(IDR_MENU_CMD);

	CMenu* pContextMenu = menu.GetSubMenu(0);
	if (pContextMenu == nullptr)
	{
		return;
	}

	CPoint point;
	GetCursorPos(&point);
	pContextMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
}


void CControllerDlg::OnAccountOk()
{
	ChangeStatus(true);
}


void CControllerDlg::OnAccountError()
{
	ChangeStatus(false);
}


void CControllerDlg::OnAddFan()
{
	CAddFanDlg dlg;
	if (dlg.DoModal() != IDOK)
	{
		return;
	}

	CString data;
	data.Format(L"cmd=push,id=%s,type=addfan,count=%d", m_clientToControl, dlg.m_fanCount);
	m_tcpClient.SendData(CImCharset::UnicodeToUTF8(data));
}


void CControllerDlg::OnFollow()
{
	SendLog(L"关注成功");
}


void CControllerDlg::OnReply()
{
	SendLog(L"回复成功");
}


void CControllerDlg::OnPrivateChat()
{
	SendLog(L"私聊成功");
}


void CControllerDlg::OnComment()
{
	SendLog(L"评论成功");
}


void CControllerDlg::OnPushLog()
{
	CPushLogDlg dlg;
	if (dlg.DoModal() != IDOK)
	{
		return;
	}

	SendLog((LPCTSTR)dlg.m_logContent);
}

void CControllerDlg::SendLog(const std::wstring& content)
{
	CString data;
	data.Format(L"cmd=push,id=%s,type=log,message=%s", m_clientToControl, content.c_str());
	m_tcpClient.SendData(CImCharset::UnicodeToUTF8(data));
}

void CControllerDlg::ChangeStatus(bool ok)
{
	CString data;
	data.Format(L"cmd=push,id=%s,type=changestatus,status=%d", m_clientToControl, ok?0:1);
	m_tcpClient.SendData(CImCharset::UnicodeToUTF8(data));
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