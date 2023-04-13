
// MyFanControlDlg.h : ͷ�ļ�
//

#pragma once

#include "Core.h"
#include "afxcmn.h"
#include "afxwin.h"

//#define MY_DEBUG//����ģʽ����ģʽ�£�����������ģʽ����


// CMyFanControlDlg �Ի���
class CMyFanControlDlg : public CDialogEx
{
// ����
public:
	CMyFanControlDlg(CWnd* pParent = NULL);	// ��׼���캯��
	~CMyFanControlDlg();

// �Ի�������
	enum { IDD = IDD_MYFANCONTROL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	BOOL m_bForceHideWindow;//����ʱǿ�����ش���
	CCore m_core;//�¶ȿ����ں�
	HANDLE m_hCoreThread;//�ں��߳�
	int m_nLastCoreUpdateTime;//�ں�������ʱ��
	CListCtrl m_ctlStatus;// ״̬��ؼ�
	CButton m_ctlTakeOver;
	//CButton m_ctlForcedCooling;
	//CButton m_ctlLinear;
	CEdit m_ctlInterval;
	CEdit m_ctlTransition;
	CEdit m_ctlForceTemp;//ǿ����ȴ���¶�
	//add cpu temp
	CEdit m_ctlFrequency;
	CEdit m_ctlGPU0;//ǿ����ȴ���¶�
	CEdit m_ctlGPU1;//ǿ����ȴ���¶�
	CEdit m_ctlGPU2;//ǿ����ȴ���¶�
	CEdit m_ctlGPU3;//ǿ����ȴ���¶�
	CEdit m_ctlGPU4;//ǿ����ȴ���¶�
	CEdit m_ctlGPU5;//ǿ����ȴ���¶�
	CEdit m_ctlGPU10;//CurveUV_limit mv
	CEdit m_ctlGPU11;//CurveUV_limit mv
	//
	int m_nDutyEditCtlID[2][10];//ת�����ÿؼ���id
	BOOL m_bWindowVisible;//�����Ƿ���ʾ
	int m_nWindowSize[2];//�������ڳߴ�
	BOOL m_bAdvancedMode;//�߼�ģʽ
	int m_TakeOver_LockGPUFrequency_Staus;  //���TakeOverʱLock״̬

public:
	static DWORD WINAPI CoreThread(LPVOID lParam);
	afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	void UpdateGui(BOOL bFull);//���½��棬bFullΪ�Ƿ��������£������Զ���ת�����ã����Ϊ��ֻ���·���״̬
	BOOL CheckAndSave();//������д�Ƿ���ȷ�������ȷ�򱣴淵��TRUE������ȷ�򷵻�FALSE
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnBnClickedButtonReset();
	afx_msg void OnBnClickedButtonLoad();
	void SetTray(PCSTR string);//�������̣���������ʾ�����ݣ��������NULL����ɾ��ͼ��
	afx_msg LRESULT OnShowTask(WPARAM wParam, LPARAM lParam);//ϵͳ�����¼�����
	afx_msg LRESULT OnTaskBarRestart(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedCheckTakeover();
	//afx_msg void OnBnClickedCheckForce();
	//afx_msg void OnBnClickedCheckLinear();
	void SetAdvancedMode(BOOL bAdvanced);//���ø߼�ģʽ
	afx_msg void OnBnClickedButtonAdvanced();
	CButton m_ctlAutorun;
	afx_msg void OnBnClickedCheckAutorun();
	BOOL SetAutorunReg(BOOL bWrite = FALSE, BOOL bAutorun = FALSE);//�����Զ����е�ע����bWriteд����ȡ��bAutorun��д����д��ִ�л��ǲ�ִ��
	BOOL SetAutorunTask(BOOL bWrite = FALSE, BOOL bAutorun = FALSE);//�����Զ����еļƻ�����bWriteд����ȡ��bAutorun��д����д��ִ�л��ǲ�ִ��
	CString ExecuteCmd(CString str);//�����½���ִ�������д���
	BOOL CreateTaskXml(PCSTR strXmlPath, PCSTR strTargetPath);//�����ƻ����������ģ��XML�ļ�
	afx_msg void OnBnClickedCheckLockGpuFrequancy();
	BOOL CheckInputFrequency(int frequency);//��������Ƶ���Ƿ����
	CButton m_ctlLockGpuFrequancy;
	afx_msg void OnEnChangeEditTreansition();
	afx_msg void OnEnChangeEditForceTemp();
	afx_msg void OnEnChangeEditGpuFrequency();
	afx_msg void OnEnChangeEditGpu0();
	afx_msg void OnEnChangeEditGpu1();
	afx_msg void OnEnChangeEditGpu2();
	afx_msg void OnEnChangeEditGpu3();
	afx_msg void OnEnChangeEditGpu4();
	afx_msg void OnEnChangeEditGpu5();
	afx_msg void OnEnChangeEditGpu10();
	afx_msg void OnEnChangeEditGpu11();
	afx_msg void OnBnClickedOk();
	//int IntSize(char* x);
	//static UINT NEAR WM_FIND = RegisterWindowMessage("COMMDLG_FIND");
	

	//static  UINT WM_TASKBARCREATED = ::RegisterWindowMessage(_T("TaskbarCreated"));
	////ON_REGISTERED_MESSAGE(WM_TaskbarRestart, OnTaskBarRestart)
	//BEGIN_MESSAGE_MAP(CMyFanControlDlg, CDialogEx)
	//	//{{AFX_MSG_MAP(CMyWnd)
	//	//ON_REGISTERED_MESSAGE(WM_FIND, OnFind)
	//	ON_REGISTERED_MESSAGE(WM_TaskbarRestart, OnTaskBarRestart)
	//	//}}AFX_MSG_MAP
	//END_MESSAGE_MAP()
};
