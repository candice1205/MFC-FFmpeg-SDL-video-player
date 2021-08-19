
// MFC+FFmpegDlg.cpp: 實作檔案
//

#include "pch.h"
#include "framework.h"
#include "MFC+FFmpeg.h"
#include "MFC+FFmpegDlg.h"
#include "afxdialogex.h"

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"

extern "C" {
#include "libavutil/avutil.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "libavutil/avconfig.h"
#include "libavutil/pixfmt.h"
#include <SDL.h>
#include <SDL_thread.h>
#include <SDL_main.h>
#include <SDL_opengl.h>
};

#include <time.h>



#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#pragma warning(disable: 4996)   

// 對 App About 使用 CAboutDlg 對話方塊

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 對話方塊資料
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支援

// 程式碼實作
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


// CMFCFFmpegDlg 對話方塊



CMFCFFmpegDlg::CMFCFFmpegDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFCFFMPEG_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCFFmpegDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_URL, m_url);
}

BEGIN_MESSAGE_MAP(CMFCFFmpegDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()

	ON_BN_CLICKED(IDC_PLAY, &CMFCFFmpegDlg::OnBnClickedPlay)
	ON_BN_CLICKED(IDC_ABORT, &CMFCFFmpegDlg::OnBnClickedAbort)
	ON_BN_CLICKED(IDC_FILEDIALOG, &CMFCFFmpegDlg::OnBnClickedFiledialog)
	ON_EN_CHANGE(IDC_URL, &CMFCFFmpegDlg::OnEnChangeUrl)
	ON_BN_CLICKED(IDC_PAUSE, &CMFCFFmpegDlg::OnBnClickedPause)
	ON_BN_CLICKED(IDC_STOP, &CMFCFFmpegDlg::OnBnClickedStop)
	ON_STN_CLICKED(IDC_SCREEN, &CMFCFFmpegDlg::OnStnClickedScreen)
END_MESSAGE_MAP()

// CMFCFFmpegDlg 訊息處理常式

BOOL CMFCFFmpegDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 將 [關於...] 功能表加入系統功能表。

	// IDM_ABOUTBOX 必須在系統命令範圍之中。
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

	// 設定此對話方塊的圖示。當應用程式的主視窗不是對話方塊時，
	// 框架會自動從事此作業
	SetIcon(m_hIcon, TRUE);			// 設定大圖示
	SetIcon(m_hIcon, FALSE);		// 設定小圖示

	// TODO: 在此加入額外的初始設定

	return TRUE;  // 傳回 TRUE，除非您對控制項設定焦點
}

void CMFCFFmpegDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 如果將最小化按鈕加入您的對話方塊，您需要下列的程式碼，
// 以便繪製圖示。對於使用文件/檢視模式的 MFC 應用程式，
// 框架會自動完成此作業。

void CMFCFFmpegDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 繪製的裝置內容

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 將圖示置中於用戶端矩形
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 描繪圖示
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 當使用者拖曳最小化視窗時，
// 系統呼叫這個功能取得游標顯示。
HCURSOR CMFCFFmpegDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
//Refresh Event
#define SFM_REFRESH_EVENT  (SDL_USEREVENT + 1)

#define SFM_BREAK_EVENT  (SDL_USEREVENT + 2)

int thread_exit = 0;
int thread_pause = 0;


int sfp_refresh_thread(void *opaque) {

	thread_exit = 0;
	thread_pause = 0;

	while (thread_exit == 0) {
		if (!thread_pause) {
			SDL_Event event;
			event.type = SFM_REFRESH_EVENT;
			SDL_PushEvent(&event);
		}
		SDL_Delay(40);
	}
	//Quit
	SDL_Event event;
	event.type = SFM_BREAK_EVENT;
	SDL_PushEvent(&event);
	thread_exit = 0;
	thread_pause = 0;
	return 0;
}

//================================解碼/播放==========================================
int ffmpegplayer(LPVOID lpParam)
{

	AVFormatContext	*pFormatCtx;
	int				i, videoindex, ptsVideo;
	AVCodecContext	*pCodecCtx;
	AVCodec			*pCodec;
	AVFrame	*pFrame, *pFrameYUV;
	uint8_t *out_buffer;
	AVPacket *packet;
	int ret, got_picture;
	double time1,time2;

	//------------SDL start----------------

	int screen_w, screen_h;
	SDL_Window *screen;
	SDL_Renderer* sdlRenderer;
	SDL_Texture* sdlTexture;
	SDL_Rect sdlRect;
	SDL_Thread *video_tid;
	SDL_Event event;

	struct SwsContext *img_convert_ctx;

	//===========================================
	//檔案路徑
	CMFCFFmpegDlg *dlg = (CMFCFFmpegDlg *)lpParam;
	char filepath[250] = { 0 };
	GetWindowTextA(dlg->m_url, (LPSTR)filepath, 250);

	//==============更新事件+解碼===========================
	av_register_all();
	avformat_network_init();
	pFormatCtx = avformat_alloc_context();

	if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0) {
		AfxMessageBox(_T("Couldn't open input stream.\n"));
		return -1;
	}
	if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
		AfxMessageBox(_T("Couldn't find stream information.\n"));
		return -1;
	}
	videoindex = -1;
	for (i = 0; i < pFormatCtx->nb_streams; i++)
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoindex = i;
			break;
		}
	if (videoindex == -1) {
		AfxMessageBox(_T("Didn't find a video stream.\n"));
		return -1;
	}
	pCodecCtx = pFormatCtx->streams[videoindex]->codec;
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL) {
		AfxMessageBox(_T("Codec not found.\n"));
		return -1;
	}
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
		AfxMessageBox(_T("Could not open codec.\n"));
		return -1;
	}
	pFrame = av_frame_alloc();
	pFrameYUV = av_frame_alloc();
	out_buffer = (uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, 
		pCodecCtx->width, pCodecCtx->height));
	avpicture_fill((AVPicture *)pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P, 
		pCodecCtx->width, pCodecCtx->height);



	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
		pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);



	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER)) {
		AfxMessageBox(_T("Could not initialize SDL\n"));
		return -1;
	}
	//SDL 2.0 Support for multiple windows
	screen_w = pCodecCtx->width;
	screen_h = pCodecCtx->height;
	//===============全屏顯示======================
	//screen = SDL_CreateWindow("Simplest ffmpeg player's Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		//screen_w, screen_h,SDL_WINDOW_OPENGL);
	//=============顯示在MFC上=========================
	screen = SDL_CreateWindowFrom(dlg->GetDlgItem(IDC_SCREEN)->GetSafeHwnd());
	//=================RGB轉YUV==========================
	if (!screen) {
		AfxMessageBox(_T("SDL: could not create window - exiting\n"));
		return -1;
	}
	sdlRenderer = SDL_CreateRenderer(screen, -1, 0);
	//IYUV: Y + U + V  (3 planes)
	//YV12: Y + V + U  (3 planes)
	sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, pCodecCtx->width, pCodecCtx->height);

	sdlRect.x = 0;
	sdlRect.y = 0;
	sdlRect.w = screen_w;
	sdlRect.h = screen_h;

	packet = (AVPacket *)av_malloc(sizeof(AVPacket));

	video_tid = SDL_CreateThread(sfp_refresh_thread, NULL, NULL);
	//----------------------------SDL End------------------------------------
	//Event Loop

	for (;;) {
		//Wait
		SDL_WaitEvent(&event);
		if (event.type == SFM_REFRESH_EVENT) {
			//------------------------------
			if (av_read_frame(pFormatCtx, packet) >= 0) {
				if (packet->stream_index == videoindex) {
					ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
					if (ret < 0) {
						AfxMessageBox(_T("Decode Error.\n"));
						return -1;
					}
					if (got_picture) {

						time1 = clock();
						sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);
						//--------------SDL---------------------------
						SDL_UpdateTexture(sdlTexture, NULL, pFrameYUV->data[0], pFrameYUV->linesize[0]);
						SDL_RenderClear(sdlRenderer);
						//SDL_RenderCopy( sdlRenderer, sdlTexture, &sdlRect, &sdlRect );  
						SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
						SDL_RenderPresent(sdlRenderer);
						//Sleep(1000);
						time2 = clock();
						TRACE("decode time=%f\n", time2-time1);

						//------------------SDL End-----------------------
						TRACE("Decode 1 frame\n");
					}
				}
				av_free_packet(packet);
			}
			else {
				//Exit Thread
				thread_exit = 1;
			}
		}
		else if (event.type == SDL_QUIT) {
			thread_exit = 1;
		}
		else if (event.type == SFM_BREAK_EVENT) {
			break;
		}

	}

	sws_freeContext(img_convert_ctx);

	SDL_DestroyWindow(screen);
	SDL_Quit();
	dlg->GetDlgItem(IDC_SCREEN)->ShowWindow(SW_SHOWNORMAL);
	//---------------------------------------------------------------
	av_frame_free(&pFrameYUV);
	av_frame_free(&pFrame);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);

	return 0;
}

//===============================播放thread==========================================
UINT Thread_Play(LPVOID lpParam) {
	CMFCFFmpegDlg *dlg = (CMFCFFmpegDlg *)lpParam;
	ffmpegplayer(lpParam);
	return 0;
}


//=================================播放==============================================
void CMFCFFmpegDlg::OnBnClickedPlay()
{
	pThreadPlay = AfxBeginThread(Thread_Play, this);
}

//=================================關於==============================================
void CMFCFFmpegDlg::OnBnClickedAbort()
{
	CAboutDlg dlg1;
	dlg1.DoModal();
}

//=================================檔案==============================================
void CMFCFFmpegDlg::OnBnClickedFiledialog()
{
	CString FilePathName;
	CFileDialog dlg(TRUE, NULL, NULL, NULL, NULL);
	if (dlg.DoModal() == IDOK) {
		FilePathName = dlg.GetPathName();
		m_url.SetWindowText(FilePathName);
	}
}

//=================================路徑==============================================
void CMFCFFmpegDlg::OnEnChangeUrl()
{
}

//=================================暫停==============================================
void CMFCFFmpegDlg::OnBnClickedPause()
{
	thread_pause = !thread_pause;
}

//=================================停止==============================================
void CMFCFFmpegDlg::OnBnClickedStop()
{
	thread_exit = 1;
}

//===============================影片screen==========================================
void CMFCFFmpegDlg::OnStnClickedScreen()
{
}