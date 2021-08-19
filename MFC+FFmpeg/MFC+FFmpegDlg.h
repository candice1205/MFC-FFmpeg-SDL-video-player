
// MFC+FFmpegDlg.h: 標頭檔
//

#pragma once


// CMFCFFmpegDlg 對話方塊
class CMFCFFmpegDlg : public CDialogEx
{
// 建構
public:
	CMFCFFmpegDlg(CWnd* pParent = NULL);	// 標準建構函式

// 對話方塊資料
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCFFMPEG_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支援


// 程式碼實作
protected:
	HICON m_hIcon;
	CWinThread *pThreadPlay;

	// 產生的訊息對應函式
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedPlay();
	afx_msg void OnBnClickedAbort();
	afx_msg void OnBnClickedFiledialog();
	afx_msg void OnEnChangeUrl();
	afx_msg void OnBnClickedPause();
	afx_msg void OnBnClickedStop();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnStnClickedScreen();
	CEdit m_url;
};
