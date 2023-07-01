
// MyFanControlDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "MyFanControl.h"
#include "MyFanControlDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_SHOWTASK (WM_USER +1)
#define IDR_SHOW 11
#define IDR_EXIT 12

__int64 CompareFileTime(FILETIME time1, FILETIME time2)
{
	__int64 a = __int64(time1.dwHighDateTime) << 32 | time1.dwLowDateTime;
	__int64 b = __int64(time2.dwHighDateTime) << 32 | time2.dwLowDateTime;

	return (b - a);
}
int GetCpuClock(int* CPU_usage)
{
	LARGE_INTEGER c1;
	QueryPerformanceCounter(&c1);
	LARGE_INTEGER c2;
	QueryPerformanceCounter(&c2);
	unsigned __int64 start = __rdtsc();
	///
	FILETIME preidleTime, prekernelTime, preuserTime;
	BOOL res = GetSystemTimes(&preidleTime, &prekernelTime, &preuserTime);
	///
	DWORD startTickCount = GetTickCount();
	while (GetTickCount() - startTickCount < static_cast<DWORD>(100)) {}
	LARGE_INTEGER c3;
	QueryPerformanceCounter(&c3);
	unsigned __int64 end = __rdtsc();
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	//
	FILETIME idleTime, kernelTime, userTime;
	res = GetSystemTimes(&idleTime, &kernelTime, &userTime);
	__int64 idle = CompareFileTime(preidleTime, idleTime);
	__int64 kernel = CompareFileTime(prekernelTime, kernelTime);
	__int64 user = CompareFileTime(preuserTime, userTime);

	int cpu = int(((kernel - idle + user) * 100) / (kernel + user));
	if (CPU_usage)
		*CPU_usage = cpu;
	//
	unsigned __int64 e = (c3.QuadPart - c2.QuadPart) - (c2.QuadPart - c1.QuadPart);
	double elapsed = static_cast<double>(e) / static_cast<double>(freq.QuadPart);
	return int((static_cast<double>((end - start) / elapsed)) / 1000000);
}
////

//整数位数
int IntSize(CString str)
{
	int x, y, z;
	char b[256];
	x = atoi(str);
	y = printf("%d", x);
	z = sprintf(b, "%d", x);  /*不显示打印*/
	return (z);
}

int IntSize_err(CString str)
{
	int num = atoi(str);
	int n = 1 + (int)log10(num);
	return (n);
}


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
public:
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

//BEGIN_MESSAGE_MAP(CMyFanControlDlg, CDialogEx)
//	ON_EN_CHANGE(IDC_EDIT_GPU10, &CMyFanControlDlg::OnEnChangeEditGpu10)
//END_MESSAGE_MAP()


// CMyFanControlDlg 对话框



CMyFanControlDlg::CMyFanControlDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMyFanControlDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_bForceHideWindow = TRUE;//启动时强制隐藏窗口
#ifdef MY_DEBUG
	m_bForceHideWindow = FALSE;//启动时强制隐藏窗口
#endif
	m_hCoreThread = NULL;
	m_nLastCoreUpdateTime = -1;
	m_bWindowVisible = FALSE;
	m_bAdvancedMode = TRUE;
	m_nWindowSize[0] = 0;
	m_nWindowSize[1] = 0;

	//	int LockGPUFrequency_TakeOver_Staus   //标记TakeOver时Lock状态  0 未锁定,1 锁定 ,2 未初始化
	m_TakeOver_LockGPUFrequency_Staus = 2;

	//new
	m_nDutyEditCtlID[0][0] = IDC_EDIT_GPU0;
	m_nDutyEditCtlID[0][1] = IDC_EDIT_GPU1;
	m_nDutyEditCtlID[0][2] = IDC_EDIT_GPU2;
	m_nDutyEditCtlID[0][3] = IDC_EDIT_GPU3;
	m_nDutyEditCtlID[0][4] = IDC_EDIT_GPU4;
	m_nDutyEditCtlID[0][5] = IDC_EDIT_GPU5;
	m_nDutyEditCtlID[0][6] = IDC_EDIT_GPU10;
	m_nDutyEditCtlID[0][7] = IDC_EDIT_GPU11;


	//m_nDutyEditCtlID[0][0] = IDC_EDIT_CPU0;
	//m_nDutyEditCtlID[0][1] = IDC_EDIT_CPU1;
	//m_nDutyEditCtlID[0][2] = IDC_EDIT_CPU2;
	//m_nDutyEditCtlID[0][3] = IDC_EDIT_CPU3;
	//m_nDutyEditCtlID[0][4] = IDC_EDIT_CPU4;
	//m_nDutyEditCtlID[0][5] = IDC_EDIT_CPU5;
	//m_nDutyEditCtlID[0][6] = IDC_EDIT_CPU6;
	//m_nDutyEditCtlID[0][7] = IDC_EDIT_CPU7;
	//m_nDutyEditCtlID[0][8] = IDC_EDIT_CPU8;
	//m_nDutyEditCtlID[0][9] = IDC_EDIT_CPU9;
	//m_nDutyEditCtlID[1][0] = IDC_EDIT_GPU0;
	//m_nDutyEditCtlID[1][1] = IDC_EDIT_GPU1;
	//m_nDutyEditCtlID[1][2] = IDC_EDIT_GPU2;
	//m_nDutyEditCtlID[1][3] = IDC_EDIT_GPU3;
	//m_nDutyEditCtlID[1][4] = IDC_EDIT_GPU4;
	//m_nDutyEditCtlID[1][5] = IDC_EDIT_GPU5;
	//m_nDutyEditCtlID[1][6] = IDC_EDIT_GPU6;
	//m_nDutyEditCtlID[1][7] = IDC_EDIT_GPU7;
	//m_nDutyEditCtlID[1][8] = IDC_EDIT_GPU8;
	//m_nDutyEditCtlID[1][9] = IDC_EDIT_GPU9;
}

CMyFanControlDlg::~CMyFanControlDlg()
{
	DestroyWindow();
}

void CMyFanControlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_STATUS, m_ctlStatus);
	DDX_Control(pDX, IDC_CHECK_TAKEOVER, m_ctlTakeOver);
	//DDX_Control(pDX, IDC_CHECK_FORCE, m_ctlForcedCooling);
	//DDX_Control(pDX, IDC_CHECK_LINEAR, m_ctlLinear);
	DDX_Control(pDX, IDC_EDIT_INTERVAL, m_ctlInterval);
	DDX_Control(pDX, IDC_EDIT_TREANSITION, m_ctlTransition);
	DDX_Control(pDX, IDC_EDIT_FORCE_TEMP, m_ctlForceTemp);
	DDX_Control(pDX, IDC_CHECK_AUTORUN, m_ctlAutorun);
	DDX_Control(pDX, IDC_EDIT_GPU_FREQUENCY, m_ctlFrequency);
	DDX_Control(pDX, IDC_CHECK_LOCK_GPU_FREQUANCY, m_ctlLockGpuFrequancy);
	//add cpu temp 
	DDX_Control(pDX, IDC_EDIT_GPU0, m_ctlGPU0);
	DDX_Control(pDX, IDC_EDIT_GPU1, m_ctlGPU1);
	DDX_Control(pDX, IDC_EDIT_GPU2, m_ctlGPU2);
	DDX_Control(pDX, IDC_EDIT_GPU3, m_ctlGPU3);
	DDX_Control(pDX, IDC_EDIT_GPU4, m_ctlGPU4);
	DDX_Control(pDX, IDC_EDIT_GPU5, m_ctlGPU5);
	DDX_Control(pDX, IDC_EDIT_GPU10,m_ctlGPU10);
	DDX_Control(pDX, IDC_EDIT_GPU11,m_ctlGPU11);
}

const   UINT WM_TaskbarRestart = ::RegisterWindowMessage(TEXT("TaskbarCreated"));

BEGIN_MESSAGE_MAP(CMyFanControlDlg, CDialogEx)
	ON_REGISTERED_MESSAGE(WM_TaskbarRestart, OnTaskBarRestart)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &CMyFanControlDlg::OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_RESET, &CMyFanControlDlg::OnBnClickedButtonReset)
	ON_BN_CLICKED(IDC_BUTTON_LOAD, &CMyFanControlDlg::OnBnClickedButtonLoad)
	ON_MESSAGE(WM_SHOWTASK, &CMyFanControlDlg::OnShowTask)//消息映射
	ON_BN_CLICKED(IDC_CHECK_TAKEOVER, &CMyFanControlDlg::OnBnClickedCheckTakeover)
	//ON_BN_CLICKED(IDC_CHECK_FORCE, &CMyFanControlDlg::OnBnClickedCheckForce)
	//ON_BN_CLICKED(IDC_CHECK_LINEAR, &CMyFanControlDlg::OnBnClickedCheckLinear)
	ON_BN_CLICKED(IDC_BUTTON_ADVANCED, &CMyFanControlDlg::OnBnClickedButtonAdvanced)
	ON_BN_CLICKED(IDC_CHECK_AUTORUN, &CMyFanControlDlg::OnBnClickedCheckAutorun)
	ON_BN_CLICKED(IDC_CHECK_LOCK_GPU_FREQUANCY, &CMyFanControlDlg::OnBnClickedCheckLockGpuFrequancy)
	ON_EN_CHANGE(IDC_EDIT_TREANSITION, &CMyFanControlDlg::OnEnChangeEditTreansition)
	ON_EN_CHANGE(IDC_EDIT_FORCE_TEMP, &CMyFanControlDlg::OnEnChangeEditForceTemp)
	ON_EN_CHANGE(IDC_EDIT_GPU_FREQUENCY, &CMyFanControlDlg::OnEnChangeEditGpuFrequency)
	ON_EN_CHANGE(IDC_EDIT_GPU0, &CMyFanControlDlg::OnEnChangeEditGpu0)
	ON_EN_CHANGE(IDC_EDIT_GPU1, &CMyFanControlDlg::OnEnChangeEditGpu1)
	ON_EN_CHANGE(IDC_EDIT_GPU2, &CMyFanControlDlg::OnEnChangeEditGpu2)
	ON_EN_CHANGE(IDC_EDIT_GPU3, &CMyFanControlDlg::OnEnChangeEditGpu3)
	ON_EN_CHANGE(IDC_EDIT_GPU4, &CMyFanControlDlg::OnEnChangeEditGpu4)
	ON_EN_CHANGE(IDC_EDIT_GPU5, &CMyFanControlDlg::OnEnChangeEditGpu5)
	ON_EN_CHANGE(IDC_EDIT_GPU10, &CMyFanControlDlg::OnEnChangeEditGpu10)
	ON_EN_CHANGE(IDC_EDIT_GPU11, &CMyFanControlDlg::OnEnChangeEditGpu11)
	ON_BN_CLICKED(IDOK, &CMyFanControlDlg::OnBnClickedOk)
	ON_MESSAGE(WM_POWERBROADCAST, OnPowerBroadcast)

END_MESSAGE_MAP()


// CMyFanControlDlg 消息处理程序

BOOL CMyFanControlDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	
	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
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

	// TODO:  在此添加额外的初始化代码
	//获取窗口完整尺寸
	CRect rect;
	this->GetWindowRect(rect);
	m_nWindowSize[0] = rect.Width();
	m_nWindowSize[1] = rect.Height();
	SetAdvancedMode(FALSE);

	SetTray("GPULock");



	if (m_hCoreThread == NULL)
	{
		DWORD dwThreadID = 0;
		m_hCoreThread = CreateThread(NULL, NULL, CoreThread, this, NULL, &dwThreadID);
	}
	SetTimer(0, 100, NULL);
	m_ctlAutorun.SetCheck(SetAutorunReg(FALSE) || SetAutorunTask(FALSE));

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMyFanControlDlg::OnSysCommand(UINT nID, LPARAM lParam)
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



LRESULT CMyFanControlDlg::OnTaskBarRestart(WPARAM wParam, LPARAM lParam) //重建tray
{
	static BOOL added = FALSE;
	NOTIFYICONDATA nid;
	nid.cbSize = (DWORD)sizeof(NOTIFYICONDATA);
	nid.hWnd = this->m_hWnd;
	nid.uID = IDR_MAINFRAME;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.uCallbackMessage = WM_SHOWTASK;//自定义的消息名称  
	nid.hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
	Shell_NotifyIcon(NIM_ADD, &nid);
	added = TRUE;
	return 0;
}	


// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMyFanControlDlg::OnPaint()
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
HCURSOR CMyFanControlDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

DWORD CMyFanControlDlg::CoreThread(LPVOID lParam)
{

	CMyFanControlDlg* pDlg = (CMyFanControlDlg*)lParam;
	pDlg->m_core.Run();

	return 0;
}

void CMyFanControlDlg::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	if (m_bForceHideWindow)
	{
		lpwndpos->flags &= ~SWP_SHOWWINDOW;
	}


	CDialogEx::OnWindowPosChanging(lpwndpos);

	// TODO:  在此处添加消息处理程序代码
}


void CMyFanControlDlg::OnOK()
{
	// TODO:  在此添加专用代码和/或调用基类
	//GetNextDlgTabItem(GetFocus())->SetFocus(); 
	if (!m_core.m_nExit)
		m_core.m_nExit = 1;

	if (m_core.m_nExit == 1)//等待内核线程结束
	{
		int count = 0;
		while (m_core.m_nExit == 1 && count++ < 100)
		{
			Sleep(100);
		}
	}
	if (m_core.m_nExit)
	{
		KillTimer(0);
		SetTray(NULL);
		CDialogEx::OnOK();
	}
}


void CMyFanControlDlg::OnCancel()
{
	// TODO:  在此添加专用代码和/或调用基类
	if (m_bWindowVisible)
	{
		this->ShowWindow(SW_HIDE);
	}
	else
	{
		this->ShowWindow(SW_SHOW);
		SetForegroundWindow();
	}
	//CDialogEx::OnCancel();
}


void CMyFanControlDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	static int nCheckThreadCount = 0;//检查工作线程状态计数器，每100ms+1
	CDialogEx::OnTimer(nIDEvent);
	if (m_core.m_nExit == 2)
	{
		OnOK();
	}

	//检查工作线程是否卡死
	//nCheckThreadCount++;
	//if (nCheckThreadCount > 150)//内核已经15秒未完成一个循环，认为卡死，结束程序
	//{
	//	KillTimer(0);
	//	m_core.m_nExit = 2;
	//	TerminateThread(m_hCoreThread, -1);//强制结束进程
	//	CloseHandle(m_hCoreThread);
	//	m_hCoreThread = NULL;
	//	MessageBox("检测到工作线程卡死，程序将立刻结束，如果重试后问题仍然存在，说明本程序可能不适用于此电脑。");
	//	OnOK();
	//}

	if (m_core.m_nInit != 1)
		return;


	//根据窗口显示状态来刷新
	static BOOL LastVisible = FALSE;
	m_bWindowVisible = IsWindowVisible();
	if (m_bWindowVisible && !LastVisible)
	{
		m_core.m_bUpdateRPM = TRUE;//更新风扇转速
		UpdateGui(TRUE);
	}
	else if (!m_bWindowVisible && LastVisible)
	{
		m_core.m_bUpdateRPM = FALSE;
		//AfxMessageBox("窗口已隐藏");
	}
	LastVisible = m_bWindowVisible;
	// 内核已经更新数据，需要更新界面
	if (m_nLastCoreUpdateTime != m_core.m_nLastUpdateTime)
	{
		if (m_bWindowVisible)
			UpdateGui(FALSE);
		nCheckThreadCount = 0;
		m_nLastCoreUpdateTime = m_core.m_nLastUpdateTime;
	}
	//
}

void CMyFanControlDlg::UpdateGui(BOOL bFull)
{
	int row = m_ctlStatus.GetItemCount();
	int col = m_ctlStatus.GetHeaderCtrl()->GetItemCount();
	if (row != 6 || col != 3)
	{
		if (!bFull)
		{
			UpdateGui(TRUE);//需要完整更新界面
			return;
		}
		//得到控件大小
		CRect rect;

		m_ctlStatus.GetWindowRect(rect);
		int width = rect.Width();
		//设置行列数
		m_ctlStatus.DeleteAllItems();//删除所有单元格
		while (m_ctlStatus.DeleteColumn(0));//删除所有列
		int i = 0;
		m_ctlStatus.InsertColumn(i++, "", LVCFMT_CENTER, int(width * 0.32));
		m_ctlStatus.InsertColumn(i++, "CPU", LVCFMT_CENTER, int(width * 0.33));
		m_ctlStatus.InsertColumn(i++, "GPU", LVCFMT_CENTER, int(width * 0.33));
		i = 0;
		m_ctlStatus.InsertItem(i++, "当前温度");
		m_ctlStatus.InsertItem(i++, "设定挡位");
		m_ctlStatus.InsertItem(i++, "转速%");
		//m_ctlStatus.InsertItem(i++, "设定负载");
		m_ctlStatus.InsertItem(i++, "转速RPM");
		m_ctlStatus.InsertItem(i++, "运行频率");
		m_ctlStatus.InsertItem(i++, "使用率%");
		//m_ctlStatus.InsertItem(i++, "CurveUV_limitmv");
	}
	//显示状态信息
	char str[256];
	for (int i = 0; i < 2; i++)
	{
		sprintf_s(str, 256, "%d", m_core.m_nCurTemp[i]);
		m_ctlStatus.SetItemText(0, i + 1, str);
		sprintf_s(str, 256, "%d", m_core.m_nSetDutyLevel[i]);
		m_ctlStatus.SetItemText(1, i + 1, str);
		sprintf_s(str, 256, "%d", m_core.m_nCurDuty[i]);
		m_ctlStatus.SetItemText(2, i + 1, str);
		//sprintf_s(str, 256, "%d", m_core.m_nSetDuty[i]);
		//m_ctlStatus.SetItemText(3, i + 1, str);
		if (m_core.m_nCurRPM[i] >= 0)
		{
			sprintf_s(str, 256, "%d", m_core.m_nCurRPM[i]);
			m_ctlStatus.SetItemText(3, i + 1, str);
		}
		else
		{
			m_ctlStatus.SetItemText(3, i + 1, "-");
		}
		if (i == 1)
		{

			sprintf_s(str, 256, "%d/%d", m_core.m_GpuInfo.m_nGPU_Temp, m_core.m_config.downTemplimit);
			m_ctlStatus.SetItemText(0, i + 1, str);

			sprintf_s(str, 256, "%d/%d", m_core.m_GpuInfo.m_nGraphicsClock, m_core.m_GpuInfo.m_nMemoryClock);
			m_ctlStatus.SetItemText(4, i + 1, str);
			sprintf_s(str, 256, "%d", m_core.m_GpuInfo.m_nGPU_Util);
			m_ctlStatus.SetItemText(5, i + 1, str);
			//CurveUV_limit Add 
			//sprintf_s(str, 256, "%d", m_core.m_config.CurveUV_limit);
			//m_ctlStatus.SetItemText(6, i + 1, str);

		}
		/*
		else if (i == 0)
		{
			int cpu_usage;
			int CpuClock = GetCpuClock(&cpu_usage);
			sprintf_s(str, 256, "%d", CpuClock);
			m_ctlStatus.SetItemText(4, i + 1, str);
			sprintf_s(str, 256, "%d", cpu_usage);
			m_ctlStatus.SetItemText(5, i + 1, str);
		}*/

	}
	//强制冷却状态
	//int fc = m_ctlForcedCooling.GetCheck();
	//if (fc ^ m_core.m_bForcedCooling)
	//{
	//	m_ctlForcedCooling.SetCheck(m_core.m_bForcedCooling);
	//}
	//////////////////////////////////////////////////////////

	//add update 
	if (m_core.m_config.TakeOver)
		if (m_core.m_config.LockGPUFrequency)
		{
			int lf = m_ctlLockGpuFrequancy.GetCheck();
			if (lf ^ m_core.m_config.LockGPUFrequency)
				m_ctlLockGpuFrequancy.SetCheck(TRUE);
			char str[256];
			m_ctlFrequency.GetWindowTextA(str, 256);
			int nFrequency = atoi(str);
			//if (nFrequency ^ 0 && nFrequency ^ m_core.m_GpuInfo.m_nGraphicsClock)
			if (nFrequency ^ m_core.m_GpuInfo.m_nGraphicsClock)
			{
				sprintf_s(str, 256, "%d", m_core.m_GpuInfo.m_nGraphicsClock);
				m_ctlFrequency.SetWindowTextA(str);
			}
		}
		else
		{
			int lf = m_ctlLockGpuFrequancy.GetCheck();
			if (lf ^ m_core.m_config.LockGPUFrequency)
			{
				m_ctlLockGpuFrequancy.SetCheck(FALSE);
				char str[256];
				sprintf_s(str, 256, "%d", m_core.m_config.GPUFrequency);
				m_ctlFrequency.SetWindowTextA(str);
			}

		}

	if (!bFull)
		return;
	//接管控制 动态锁定
	int to = m_ctlTakeOver.GetCheck();
	if (to ^ m_core.m_config.TakeOver)
	{
		m_ctlTakeOver.SetCheck(m_core.m_config.TakeOver);
	}
	//线性控制
	//int lc = m_ctlLinear.GetCheck();
	//if (lc ^ m_core.m_config.Linear)
	//{
	//	m_ctlLinear.SetCheck(m_core.m_config.Linear);
	//}

	//更新间隔
	sprintf_s(str, 256, "%d", m_core.m_config.UpdateInterval);
	m_ctlInterval.SetWindowTextA(str);
	//GPU显存频率(过渡温度) TransitionTemp -->GPU Mem
	sprintf_s(str, 256, "%d", m_core.m_config.TransitionTemp);
	m_ctlTransition.SetWindowTextA(str);
	//GPU核心+ ( 强制冷却温度) ForceTemp -->GPU Core
	sprintf_s(str, 256, "%d", m_core.m_config.ForceTemp);
	m_ctlForceTemp.SetWindowTextA(str);
	//GPU频率
	sprintf_s(str, 256, "%d", m_core.m_config.GPUFrequency);
	m_ctlFrequency.SetWindowTextA(str);
	//CurveUV_limit Show Gui
	sprintf_s(str, 256, "%d", m_core.m_config.CurveUV_limit);
	m_ctlGPU10.SetWindowTextA(str);
	//OverClock2
	sprintf_s(str, 256, "%d", m_core.m_config.OverClock2);
	m_ctlGPU11.SetWindowTextA(str);
	//GPU限频
	int lf = m_ctlLockGpuFrequancy.GetCheck();
	if (lf ^ m_core.m_config.LockGPUFrequency)
	{
		m_ctlLockGpuFrequancy.SetCheck(m_core.m_config.LockGPUFrequency);
	}


	//自定义转速控制

	for (int i = 0; i < 1; i++)
	{
		for (int j = 0; j < 6; j++)
		{
			sprintf_s(str, 256, "%d", m_core.m_config.DutyList[i][j]);
			GetDlgItem(m_nDutyEditCtlID[i][j])->SetWindowTextA(str);
		}
	}
}

BOOL CMyFanControlDlg::CheckAndSave()
{
	//检查设置
	char str[256];
	m_ctlInterval.GetWindowTextA(str, 256);
	int nInterval = atoi(str);
	if (nInterval < 1 || nInterval > 300)
	{
		AfxMessageBox("更新间隔必须为1-300");
		return FALSE;
	}
	//GPU Mem 频率
	CString nchar;
	m_ctlTransition.GetWindowTextA(nchar);
	int nTransition = atoi(nchar);
	if (nTransition < -1000 || nTransition > 2000)
	{
		AfxMessageBox("GPU显存频率必须为-1000至1000");
		return FALSE;
	}
	else
	{
		//if (str ^ str.Format(_T("%f"), nTransition))
		//{
		//	sprintf_s(str, 256, "%d", nTransition);
		//	m_ctlTransition.SetWindowTextA(str);
		//}
		if (nchar.GetLength() ^ IntSize(nchar))
		{
			nTransition = m_core.m_config.GPUOverMEMClock;
			AfxMessageBox("只支持正负整数");
		}

	}


	m_ctlForceTemp.GetWindowTextA(str, 256);
	int nForceTemp = atoi(str);
	if (nForceTemp < 0 || nForceTemp > 350)
	{
		//AfxMessageBox("强制冷却温度必须为0-225");
		AfxMessageBox("GPU超频过高");
		return FALSE;
	}
	//else if (m_core.m_config.OverClock2 > 200 && nForceTemp > 200 && nForceTemp < 350)
	else if (nForceTemp > 200 && nForceTemp < 350)
	{
		char str2[256];
		AfxMessageBox("GPU超频设定警告，已超过200,上限350,注意黑屏风险");
	}
	//
	m_ctlFrequency.GetWindowTextA(str, 256);
	int nFrequency = atoi(str);
	if (!CheckInputFrequency(nFrequency))
		return FALSE;

	//int OverClock = nForceTemp; //定义默认超频150
	//if (nFrequency == 0)
	//	nFrequency = m_core.m_GpuInfo.m_nStandardFrequency;
	//
	//int nDutyList[2][10];
	//int rowLen = 8;
	int nDutyList[1][8]; //7 lab CurveUV_limit ,8 OverClock2
	for (int i = 0; i < 1; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			GetDlgItem(m_nDutyEditCtlID[i][j])->GetWindowTextA(str, 256);
			nDutyList[i][j] = atoi(str);
			if (j ^ 4 && j ^ 5)
			{
				//if ((nDutyList[i][j] < 0 || nDutyList[i][j]>100) && j != 6 && j != 7) //CurveUV_limit spec
				if ((nDutyList[i][j] < 0 || nDutyList[i][j]>350) && j != 6) //CurveUV_limit spec
				{
					char str2[256];
					sprintf_s(str2, 256, "%s设定错误，必须为0-350", i ? "CPU" : "GPU");
					AfxMessageBox(str2);
					return FALSE;
				}
				else
				{
					//if (j == 7 && (nDutyList[i][j] > 200 && nDutyList[i][j] <= 350))
					//{
					//	char str2[256];
					//	sprintf_s(str2, 256, "%s设定警告，已超过200,上限350,注意黑屏风险", i ? "CPU" : "GPU");
					//	AfxMessageBox(str2);

					//}
					if (j == 3 && nDutyList[i][j - 1] < nDutyList[i][j])
					{
						char str2[256];
						sprintf_s(str2, 256, "%s温控错误，恒定温度:%d必须小于温控降频:%d", i ? "CPU" : "GPU", nDutyList[i][j], nDutyList[i][j - 1]);
						AfxMessageBox(str2);
						return FALSE;
					}

				}

			}
			//else
			//{
			//	char str2[256];
			//	if (j == 5 && nDutyList[i][j] < m_core.m_config.UpdateInterval)
			//	{
			//		sprintf_s(str2, 256, "%s设定错误,Timelimit必须大于刷新时间:%d",i ? "CPU" : "GPU", m_core.m_config.UpdateInterval);
			//		AfxMessageBox(str2);
			//		return FALSE;
			//	}
			//}
		}
	}
	//
	//int upClockPercent;//占用率升频阈值
	//int downClockPercent;//占用率降频阈值
	//int downTemplimit;//温控降频阈值
	//int upTemplimit;//温控升频阈值
	m_core.m_config.upClockPercent = nDutyList[0][0];//占用率升频阈值
	m_core.m_config.downClockPercent = nDutyList[0][1];//占用率降频阈值
	m_core.m_config.downTemplimit = nDutyList[0][2];//温控降频阈值
	m_core.m_config.upTemplimit = nDutyList[0][3];//温控升频阈值
	m_core.m_config.upClocklimit = nDutyList[0][4];//温控升频阈值
	m_core.m_config.timelimit = nDutyList[0][5];//温控时长
	m_core.m_config.CurveUV_limit = nDutyList[0][6];//CurveUV_limit mv
	m_core.m_config.OverClock2 = nDutyList[0][7];//OverClock2 Mhz

	//应用设置
	m_core.m_config.UpdateInterval = nInterval;
	m_core.m_config.TransitionTemp = nTransition; //GPU Mem频率
	m_core.m_config.ForceTemp = nForceTemp;
	m_core.m_config.GPUFrequency = nFrequency;
	m_core.m_config.GPUOverClock = nForceTemp;
	m_core.m_config.GPUOverMEMClock = nTransition;

	if (!m_core.m_config.TakeOver)
	{
		if (m_TakeOver_LockGPUFrequency_Staus == 1 || m_TakeOver_LockGPUFrequency_Staus == 2);   // 已开启后标记为 0 未锁定, 1 锁定, 2 未初始化
			m_core.m_config.GPU_LockClock = nFrequency;  //TakeOver Limit Clock

	}
	else
	{
		//动态调整开启后
		if (m_TakeOver_LockGPUFrequency_Staus == 2)
		{
			m_core.m_config.GPU_LockClock = nFrequency;  //TakeOver Limit Clock
			//m_TakeOver_LockGPUFrequency_Staus = 1;   //Status  0 未锁定,1 锁定 ,2 未初始化
			if (m_core.m_config.LockGPUFrequency)
				//锁定标记
			{
				//m_TakeOver_LockGPUFrequency_Staus = m_core.m_config.LockGPUFrequency;  // ==1
				m_TakeOver_LockGPUFrequency_Staus = 1;   // 已开启后标记为 0 未锁定, 1 锁定, 2 未初始化

				//m_core.m_config.LockGPUFrequency = m_core.m_config.GPU_LockClock;
			}
			else
			{
				//m_core.m_config.GPUFrequency = m_core.m_config.GPU_LockClock;
				m_TakeOver_LockGPUFrequency_Staus = 0;   // 已开启后标记为 0 未锁定, 1 锁定, 2 未初始化

			}
		}

		if (!m_core.m_config.LockGPUFrequency)
			//重置GPUFrequency
		{
			m_core.m_config.GPUFrequency = 0;
		}

	}
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 6; j++)
		{
			m_core.m_config.DutyList[i][j] = nDutyList[i][j];
		}
	}
	//保存
	m_core.m_config.SaveConfig();
	return TRUE;
}

void CMyFanControlDlg::OnBnClickedButtonSave()
{
	// TODO:  在此添加控件通知处理程序代码
	m_core.ResetGPUFrequancy();
	if (CheckAndSave())
		UpdateGui(TRUE);
}


void CMyFanControlDlg::OnBnClickedButtonReset()
{
	// TODO:  在此添加控件通知处理程序代码
	m_core.m_config.LoadDefault();
	m_core.m_GpuInfo.ReloadAPI();
	m_core.ResetGPUFrequancy();
	UpdateGui(TRUE);
}


void CMyFanControlDlg::OnBnClickedButtonLoad()
{
	// TODO:  在此添加控件通知处理程序代码
	m_core.m_GpuInfo.ReloadAPI();
	m_core.m_config.LoadConfig();
	m_core.ResetGPUFrequancy();
	UpdateGui(TRUE);
}

//UINT g_uTaskbarCreated = RegisterWindowMessage(TEXT("TaskbarCreated"));



void CMyFanControlDlg::SetTray(PCSTR string)//设置托盘图标
{
	static BOOL added = FALSE;
	NOTIFYICONDATA nid;
	nid.cbSize = (DWORD)sizeof(NOTIFYICONDATA);
	nid.hWnd = this->m_hWnd;
	nid.uID = IDR_MAINFRAME;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.uCallbackMessage = WM_SHOWTASK;//自定义的消息名称  
	nid.hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
	if (string)
	{
		strcpy_s(nid.szTip, 128, string);//信息提示内容  
		if (!added)
		{
			Shell_NotifyIcon(NIM_ADD, &nid);//在托盘区添加图标
			added = TRUE;
		}
		else
		{
			Shell_NotifyIcon(NIM_MODIFY, &nid);//在托盘区添加图标
		}
	}
	else
		Shell_NotifyIcon(NIM_DELETE, &nid);
}



LRESULT CMyFanControlDlg::OnShowTask(WPARAM wParam, LPARAM lParam)
{//wParam接收的是图标的ID，而lParam接收的是鼠标的行为 
	if (wParam != IDR_MAINFRAME)
		return 1;
	switch (lParam)
	{
	case WM_LBUTTONUP://左键单击显示主界面
	{

	}break;
	case WM_RBUTTONUP://右击弹出菜单
	{
		LPPOINT lpoint = new tagPOINT;
		::GetCursorPos(lpoint);//得到鼠标位置
		CMenu menu;
		menu.CreatePopupMenu();
		if (m_bWindowVisible)
			menu.AppendMenu(MFT_STRING, IDR_SHOW, "隐藏");
		else
			menu.AppendMenu(MFT_STRING, IDR_SHOW, "显示");
		menu.AppendMenu(MFT_SEPARATOR);
		menu.AppendMenu(MFT_STRING, IDR_EXIT, "退出");
		SetForegroundWindow();//不加此行在菜单外点击菜单不销毁
		int xx = TrackPopupMenu(menu, TPM_RETURNCMD, lpoint->x, lpoint->y, NULL, this->m_hWnd, NULL);//显示菜单并获取选项ID
		if (xx == IDR_SHOW)
		{
			OnCancel();

		}
		else if (xx == IDR_EXIT)
		{
			OnOK();
		}
		HMENU hmenu = menu.Detach();
		menu.DestroyMenu();
		delete lpoint;
	}break;
	case WM_LBUTTONDBLCLK:
	{
		this->ShowWindow(SW_SHOW);
		SetForegroundWindow();
	}break;
	case WM_MOUSEMOVE:
	{
		static int LastUpdate = -1;
		if (LastUpdate != m_core.m_nLastUpdateTime)
		{
			char str[128];
			//sprintf_s(str, 128, "CPU：%d℃，%d%%\nGPU：%d℃，%d%%", m_core.m_nCurTemp[0], m_core.m_nCurDuty[0], m_core.m_nCurTemp[1], m_core.m_nCurDuty[1]);
			sprintf_s(str, 128, "GPU：%dM  +%d\nGMem：%dM +%d\nGTemp: %d", m_core.m_GpuInfo.m_nGraphicsClock, m_core.m_config.GPUOverClock, m_core.m_GpuInfo.m_nMemoryClock, m_core.m_config.GPUOverMEMClock, m_core.m_GpuInfo.m_nGPU_Temp);
			SetTray(str);
			LastUpdate = m_core.m_nLastUpdateTime;
		}

	}break;
	}
	return 0;
}

// Handle the WM_POWERBROADCAST message to process a message concerning power management
// such as going to Sleep or Waking Up.

//http://brightguo.com/windows-sleep-and-resume-event/
//如果系统自动起来（非人为点击操作），会发送PBT_APMRESUMEAUTOMATIC事件
//
//如果系统因人为唤醒（鼠标、键盘、电源键），会先发送PBT_APMRESUMEAUTOMATIC事件，再发送PBT_APMRESUMESUSPEND事件（休眠唤醒事件），同时系统点亮屏幕。这时你的程序需要重新打开因为系统睡眠时关闭的文件。
//
//而在PBT_APMRESUMESUSPEND中，说，只有窗口先收到 PBT_APMSUSPEND事件（开始休眠的事件）后，才会再收到它，否则只能收到 PBT_APMRESUMECRITICAL事件

LRESULT CMyFanControlDlg::OnPowerBroadcast(WPARAM wParam, LPARAM lParam)
{
	LRESULT  lrProcessed = 0;  // indicate if message processed or not

	switch (wParam) {
	case PBT_APMPOWERSTATUSCHANGE:
		TRACE0("PBT_APMPOWERSTATUSCHANGE  received\n");
		//AfxMessageBox("PBT_APMPOWERSTATUSCHANGE  received\n");	
		break;
	case PBT_APMRESUMEAUTOMATIC:
		m_core.m_GpuInfo.ReloadAPI();
		m_core.ResetSleepStatus();
		TRACE0("PBT_APMRESUMEAUTOMATIC  received\n");
		//AfxMessageBox("PBT_APM唤醒自动  received\n");
		break;
	case PBT_APMRESUMESUSPEND:
		m_core.m_GpuInfo.ReloadAPI();
		m_core.ResetSleepStatus();
		TRACE0("PBT_APMRESUMESUSPEND  received\n");
		//AfxMessageBox("PBT_唤醒  received\n");
		break;
	case PBT_APMSUSPEND:
		//m_core.ResetSleepStatus();
		TRACE0("PBT_APMSUSPEND  received\n");
		//AfxMessageBox("PBT_睡眠  received\n");
		break;
	}

	// indicate if framework needs to handle message or we did ourselves.
	return lrProcessed;
}


//LRESULT CALLBACK WndProc(HWND hWnd, UINT uMessage, WPARAM wParam,
//	LPARAM lParam)
//{
//	static UINT s_uTaskbarRestart;
//
//	switch (uMessage)
//	{
//	case WM_CREATE:
//		s_uTaskbarRestart = RegisterWindowMessage(TEXT("TaskbarCreated"));
//		break;
//
//	default:
//		if (uMessage == s_uTaskbarRestart)
//		{
//			this.SetTray();//这个就是往托盘区画图标的函数，需要自己写。
//
//		}
//		break;
//	}
//	return DefWindowProc(hWnd, uMessage, wParam, lParam);
//}

void CMyFanControlDlg::OnBnClickedCheckTakeover()
{
	// TODO:  在此添加控件通知处理程序代码
	int val = m_ctlTakeOver.GetCheck();
	m_core.m_config.TakeOver = val;
	if (!val)
	{	
		//未动态锁定
		//m_core.m_config.LockGPUFrequency = 0; // TakeOver False to UnLock
		//m_core.m_config.LockGPUFrequency = m_core.m_config.GPU_LockClock;
		//m_ctlLockGpuFrequancy.SetCheck(FALSE);
		//m_TakeOver_LockGPUFrequency_Staus = 2;
		char str[256];
		//sprintf_s(str, 256, "%d", m_core.m_GpuInfo.m_nLockClock);
		sprintf_s(str, 256, "%d", m_core.m_config.GPU_LockClock);
		m_ctlFrequency.SetWindowTextA(str);
	}
	else
	{
		if (m_TakeOver_LockGPUFrequency_Staus == 2 )
		{
			if (m_core.m_config.LockGPUFrequency)
				//锁定标记
			{
				//m_TakeOver_LockGPUFrequency_Staus = m_core.m_config.LockGPUFrequency;  // ==1
				m_TakeOver_LockGPUFrequency_Staus = 1;   // 已开启后标记为 0 未锁定, 1 锁定, 2 未初始化

				//m_core.m_config.LockGPUFrequency = m_core.m_config.GPU_LockClock;
			}
			else
			{
				//m_core.m_config.GPUFrequency = m_core.m_config.GPU_LockClock;
				m_TakeOver_LockGPUFrequency_Staus = 0;   // 已开启后标记为 0 未锁定, 1 锁定, 2 未初始化

			}
	//			
		}
	//	else
	//	{
	//		m_core.m_config.GPU_LockClock = 0;
	//	}
	}
}


//void CMyFanControlDlg::OnBnClickedCheckForce()
//{
//	// TODO:  在此添加控件通知处理程序代码
//	int val = m_ctlForcedCooling.GetCheck();
//	m_core.m_bForcedCooling = val;
//}


//void CMyFanControlDlg::OnBnClickedCheckLinear()
//{
//	// TODO:  在此添加控件通知处理程序代码
//	int val = m_ctlLinear.GetCheck();
//	m_core.m_config.Linear = val;
//}

void CMyFanControlDlg::SetAdvancedMode(BOOL bAdvanced)
{
	CRect rect;
	this->GetWindowRect(rect);
	if (bAdvanced)
	{
		MoveWindow(rect.left, rect.top, m_nWindowSize[0], m_nWindowSize[1], TRUE);
		GetDlgItem(IDC_BUTTON_ADVANCED)->SetWindowTextA("简单模式");
	}
	else
	{
		MoveWindow(rect.left, rect.top, m_nWindowSize[0] * 335 / 582, m_nWindowSize[1] * 283 / 463, FALSE);
		GetDlgItem(IDC_BUTTON_ADVANCED)->SetWindowTextA("高级模式");
	}
	m_bAdvancedMode = !m_bAdvancedMode;
}

void CMyFanControlDlg::OnBnClickedButtonAdvanced()
{
	// TODO:  在此添加控件通知处理程序代码
	SetAdvancedMode(!m_bAdvancedMode);
}


void CMyFanControlDlg::OnBnClickedCheckAutorun()
{
	int val = m_ctlAutorun.GetCheck();
	int set_rv;
	if (val)
	{
		int rv = MessageBox("请选择开机自动启动方式：\r\n\r\n按\"是\"：注册表启动项自启动\r\n按\"否\"：任务计划自启动（管理员权限）\r\n按\"取消\"：放弃操作", "开机自动启动", MB_YESNOCANCEL);

		if (IDYES == rv)
		{
			set_rv = SetAutorunReg(TRUE, TRUE);
			set_rv = SetAutorunReg(FALSE);
		}
		else if (IDNO == rv)
		{
			set_rv = SetAutorunTask(TRUE, TRUE);
			set_rv = SetAutorunTask(FALSE);
		}
		else
		{
			set_rv = FALSE;
		}
		m_ctlAutorun.SetCheck(set_rv);
	}
	else
	{
		if (SetAutorunReg(FALSE))
			set_rv = SetAutorunReg(TRUE, FALSE);
		if (SetAutorunTask(FALSE))
			set_rv = SetAutorunTask(TRUE, FALSE);
		m_ctlAutorun.SetCheck(SetAutorunReg(FALSE) || SetAutorunTask(FALSE));
	}
	return;


}

BOOL CMyFanControlDlg::SetAutorunReg(BOOL bWrite, BOOL bAutorun)
{
	HKEY hKey;
	if (RegOpenKey(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", &hKey) != ERROR_SUCCESS)
	{
		AfxMessageBox("无法打开注册表");
		return FALSE;
	}
	PCSTR strProduct = "LanTianFanMonitor";

	if (bWrite)
	{
		if (bAutorun)
		{
			CString			strPath = GetExePath() + "\\MyFanControl.exe";
			unsigned long	nSize = 0;


			nSize = strPath.GetLength();
			if (RegSetValueEx(hKey, strProduct, 0, REG_SZ,
				(unsigned char*)strPath.GetBuffer(strPath.GetLength()), nSize) != ERROR_SUCCESS)
			{
				AfxMessageBox("无法写入注册表启动项，需要用管理员权限运行");
				RegCloseKey(hKey);
				return FALSE;
			}
		}
		else
		{
			if (RegDeleteValue(hKey, strProduct) != ERROR_SUCCESS)
			{
				AfxMessageBox("无法删除注册表启动项，需要用管理员权限运行");
				RegCloseKey(hKey);
				return FALSE;
			}
		}

		RegCloseKey(hKey);
	}
	else
	{
		//检查注册表项
		BOOL		  bRet = FALSE;
		unsigned long lSize = sizeof(bRet);
		if (RegQueryValueEx(hKey, strProduct, NULL, NULL, NULL, &lSize) != ERROR_SUCCESS)
		{
			RegCloseKey(hKey);
			return FALSE;
		}
		RegCloseKey(hKey);
		return lSize > 0 ? TRUE : FALSE;
	}

	return TRUE;
}

BOOL CMyFanControlDlg::SetAutorunTask(BOOL bWrite, BOOL bAutorun)
{
	CString strTaskName = "蓝天风扇监控";
	CString strPath = GetExePath() + "\\MyFanControl.exe";
	CString strcmd;
	CString strXmlPath = GetExePath() + "\\task.xml";
	if (bWrite)
	{
		if (bAutorun)
		{
			BOOL  rv = CMyFanControlDlg::CreateTaskXml(strXmlPath, strPath);
			if (!rv)
			{
				AfxMessageBox("无法创建任务计划程序xml文件");
				return FALSE;
			}
			strcmd.Format("SCHTASKS /Create /F /XML %s /TN %s", strXmlPath, strTaskName);

		}
		else
		{
			strcmd = "SCHTASKS /Delete /F /TN " + strTaskName;
		}
	}
	else
	{
		strcmd = "SCHTASKS /Query /TN " + strTaskName;
	}
	CString rs = ExecuteCmd(strcmd);
	if (bWrite && bAutorun)
		remove(strXmlPath);
	if (rs == "[执行失败]" || rs.Find("拒绝访问") >= 0)
	{
		CString str;
		str.Format("无法%s任务计划程序，需要用管理员权限运行", bWrite ? (bAutorun ? "创建" : "删除") : ("读取"));
		AfxMessageBox(str);
		return FALSE;
	}
	PCTSTR strFind = bWrite ? (bAutorun ? "成功创建" : "成功删除") : strTaskName;
	if (rs.Find(strFind) >= 0)
		return TRUE;
	return FALSE;
}

CString CMyFanControlDlg::ExecuteCmd(CString str)
{
	SECURITY_ATTRIBUTES sa;
	HANDLE hRead, hWrite;

	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;
	if (!CreatePipe(&hRead, &hWrite, &sa, 0))
	{
#ifdef MY_DEBUG
		AfxMessageBox("无法创建管道");
#endif
		return "[执行失败]";
	}
	STARTUPINFO si = { sizeof(si) };
	PROCESS_INFORMATION pi;
	si.hStdError = hWrite;
	si.hStdOutput = hWrite;
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	char cmdline[1024];
	strcpy_s(cmdline, 1024, str);
	if (!CreateProcess(NULL, cmdline, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi))
	{
#ifdef MY_DEBUG
		AfxMessageBox("无法创建命令行进程");
#endif
		return "[执行失败]";
	}
	CloseHandle(hWrite);

	char buffer[4096] = "";
	memset(buffer, 0, 4096);
	CString output;
	DWORD byteRead;
	int i = 0;
	while (true)
	{
		Sleep(100);
		if (ReadFile(hRead, buffer, 4095, &byteRead, NULL) == NULL)
			break;
		if (byteRead)
			output += buffer;
		if (i++ >= 50)
			break;
	}
	return output;
}

BOOL CMyFanControlDlg::CreateTaskXml(PCSTR strXmlPath, PCSTR strTargetPath)
{
	//运行权限
	//最高权限<RunLevel>HighestAvailable</RunLevel>\r\n
	//普通权限<RunLevel>LeastPrivilege</RunLevel>\r\n
	//用户组
	//SYSTEM<UserId>S-1-5-18</UserId>\r\n
	//USERS<GroupId>S-1-5-32-545</GroupId>\r\n
	PCSTR XmlStr = "\
<?xml version=\"1.0\" encoding=\"UTF-16\"?>\r\n\
<Task version=\"1.2\" xmlns=\"http://schemas.microsoft.com/windows/2004/02/mit/task\">\r\n\
  <RegistrationInfo>\r\n\
    <Date>2018-11-16T16:16:29</Date>\r\n\
    <Author>HQ</Author>\r\n\
    <URI>\\蓝天风扇监控</URI>\r\n\
  </RegistrationInfo>\r\n\
  <Triggers>\r\n\
    <LogonTrigger>\r\n\
      <Enabled>true</Enabled>\r\n\
    </LogonTrigger>\r\n\
  </Triggers>\r\n\
  <Principals>\r\n\
    <Principal id=\"Author\">\r\n\
      <GroupId>S-1-5-32-545</GroupId>\r\n\
      <RunLevel>HighestAvailable</RunLevel>\r\n\
    </Principal>\r\n\
  </Principals>\r\n\
  <Settings>\r\n\
    <MultipleInstancesPolicy>IgnoreNew</MultipleInstancesPolicy>\r\n\
    <DisallowStartIfOnBatteries>false</DisallowStartIfOnBatteries>\r\n\
    <StopIfGoingOnBatteries>false</StopIfGoingOnBatteries>\r\n\
    <AllowHardTerminate>false</AllowHardTerminate>\r\n\
    <StartWhenAvailable>false</StartWhenAvailable>\r\n\
    <RunOnlyIfNetworkAvailable>false</RunOnlyIfNetworkAvailable>\r\n\
    <IdleSettings>\r\n\
      <StopOnIdleEnd>false</StopOnIdleEnd>\r\n\
      <RestartOnIdle>false</RestartOnIdle>\r\n\
    </IdleSettings>\r\n\
    <AllowStartOnDemand>true</AllowStartOnDemand>\r\n\
    <Enabled>true</Enabled>\r\n\
    <Hidden>false</Hidden>\r\n\
    <RunOnlyIfIdle>false</RunOnlyIfIdle>\r\n\
    <WakeToRun>false</WakeToRun>\r\n\
    <ExecutionTimeLimit>PT0S</ExecutionTimeLimit>\r\n\
    <Priority>7</Priority>\r\n\
  </Settings>\r\n\
  <Actions Context=\"Author\">\r\n\
    <Exec>\r\n\
      <Command>%s</Command>\r\n\
    </Exec>\r\n\
  </Actions>\r\n\
</Task>\r\n";
	char str[10240];
	sprintf_s(str, 10240, XmlStr, strTargetPath);
	FILE* fp;
	fp = fopen(strXmlPath, "wt");
	if (!fp)
		return FALSE;
	fwrite(str, strlen(str), 1, fp);

	fclose(fp);
	return TRUE;
}

void CMyFanControlDlg::OnBnClickedCheckLockGpuFrequancy()
{
	// TODO:  在此添加控件通知处理程序代码
	int val = m_ctlLockGpuFrequancy.GetCheck();
	if (val)
	{
		char str[256];
		m_ctlFrequency.GetWindowTextA(str, 256);
		int nFrequency = atoi(str);
		if (!CheckInputFrequency(nFrequency))
			m_ctlLockGpuFrequancy.SetCheck(FALSE);
	}

	m_core.m_config.LockGPUFrequency = m_ctlLockGpuFrequancy.GetCheck();
}

BOOL CMyFanControlDlg::CheckInputFrequency(int nFrequency)
{
	char str[256];
	//m_core.m_GpuInfo.m_nOverClock = m_core.m_config.GPUOverClock;
	if (nFrequency < 0 || nFrequency > m_core.m_GpuInfo.m_nMaxFrequency)
	{
		char str2[256];
		sprintf_s(str2, 256, "GPU频率限制必须为0-%d，默认频率为%d，0为默认频率", m_core.m_GpuInfo.m_nMaxFrequency, m_core.m_GpuInfo.m_nStandardFrequency);
		AfxMessageBox(str2);
		return FALSE;
	}
	else
	{
		if (nFrequency > m_core.m_GpuInfo.m_nStandardFrequency)
		{
			char str2[256];
			sprintf_s(str2, 256, "GPU默认频率为%d，超频会降低系统稳定性，并会增加发热量。\n注意：由于功率限制，可能无法达到设定的频率。\n是否确认要超频？", m_core.m_GpuInfo.m_nStandardFrequency);
			int rv = MessageBox(str2, "确认要超频？", MB_YESNO);

			if (IDYES == rv)
			{
			}
			else if (IDNO == rv)
			{
				return FALSE;
			}
		}
		else if (nFrequency == 0)
		{
			nFrequency = m_core.m_GpuInfo.m_nStandardFrequency;
			//m_core.m_config.GPUFrequency = m_core.m_GpuInfo.m_nStandardFrequency;
			//m_core.m_config.GPUOverClock = m_core.m_config.ForceTemp;
			sprintf_s(str, 256, "%d", m_core.m_config.GPUFrequency);
			m_ctlFrequency.SetWindowTextA(str);
		}
	}


	return TRUE;
}



void CMyFanControlDlg::OnEnChangeEditTreansition()
{
	//GPU显存频率
	//char nChar[256];
	//m_ctlTransition.GetWindowTextA(nChar, 256);
	//int nTransition = atoi(nChar);
	//if (size(nChar) ^ IntSize(nTransition))
	//{
	//	AfxMessageBox("GPU显存频率必须为-500至1000");
	//}

	//
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}


void CMyFanControlDlg::OnEnChangeEditForceTemp()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}


void CMyFanControlDlg::OnEnChangeEditGpuFrequency()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}



void CMyFanControlDlg::OnEnChangeEditGpu0()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	//if (m_ctlGPU0)
	//{
	//	CString str;
	//	GetDlgItemText(IDC_EDIT_GPU0, str);
	//}

	//char str[256];
	//m_ctlGPU0.GetWindowTextA(str, 256);
	//int nForceTemp = atoi(str);

}

void CMyFanControlDlg::OnEnChangeEditGpu1()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}
void CMyFanControlDlg::OnEnChangeEditGpu2()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}
void CMyFanControlDlg::OnEnChangeEditGpu3()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}
void CMyFanControlDlg::OnEnChangeEditGpu4()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}
void CMyFanControlDlg::OnEnChangeEditGpu5()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}

void CMyFanControlDlg::OnEnChangeEditGpu10()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}

void CMyFanControlDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnOK();
}





void CMyFanControlDlg::OnEnChangeEditGpu11()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}
