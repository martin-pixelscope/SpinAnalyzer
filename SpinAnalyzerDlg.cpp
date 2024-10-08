
// SpinAnalyzerDlg.cpp: 구현 파일
//

#include "stdafx.h"
#include "SpinAnalyzer.h"
#include "SpinAnalyzerDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace cv;

// CSpinAnalyzerDlg 대화 상자

CSpinAnalyzerDlg::CSpinAnalyzerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SPINANALYZER_DIALOG, pParent)
	, m_fDiameter(61.5f)
	, m_fX(10.f)
	, m_fY(10.f)
	, m_fX2(0.f)
	, m_fY2(0.f)
	, m_fZ(0)
	, m_fRadius(0)
	, m_fTheta(0)
	, m_fPhi(0)
	, m_fEpsilon(0)
	, m_nIndex(0)
	, m_bAuto(TRUE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSpinAnalyzerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_ED_DIAMETER, m_fDiameter);
	DDX_Text(pDX, IDC_ED_X, m_fX);
	DDX_Text(pDX, IDC_ED_Y, m_fY);
	DDX_Text(pDX, IDC_ED_Z, m_fZ);
	DDX_Text(pDX, IDC_ED_RADIUS, m_fRadius);
	DDX_Text(pDX, IDC_ED_THETA, m_fTheta);
	DDX_Text(pDX, IDC_ED_PHI, m_fPhi);
	DDX_Text(pDX, IDC_ED_X2, m_fX2);
	DDX_Text(pDX, IDC_ED_Y2, m_fY2);
	DDX_Text(pDX, IDC_ED_EPSILON, m_fEpsilon);
	DDX_Text(pDX, IDC_ED_INDEX, m_nIndex);
	DDX_Check(pDX, IDC_CHK_AUTO, m_bAuto);
}

BEGIN_MESSAGE_MAP(CSpinAnalyzerDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_LOAD, &CSpinAnalyzerDlg::OnBnClickedBtnLoad)
	ON_BN_CLICKED(IDC_BTN_SET, &CSpinAnalyzerDlg::OnBnClickedBtnSet)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_X, &CSpinAnalyzerDlg::OnDeltaposSpinX)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_Y, &CSpinAnalyzerDlg::OnDeltaposSpinY)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_EPSILON, &CSpinAnalyzerDlg::OnDeltaposSpinEpsilon)
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_BTN_PRE, &CSpinAnalyzerDlg::OnBnClickedBtnPre)
	ON_BN_CLICKED(IDC_BTN_FORWARD, &CSpinAnalyzerDlg::OnBnClickedBtnForward)
	ON_BN_CLICKED(IDC_BTN_DETECT, &CSpinAnalyzerDlg::OnBnClickedBtnDetect)
	ON_BN_CLICKED(IDC_CHK_AUTO, &CSpinAnalyzerDlg::OnBnClickedChkAuto)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_X2, &CSpinAnalyzerDlg::OnDeltaposSpinX2)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_Y2, &CSpinAnalyzerDlg::OnDeltaposSpinY2)
END_MESSAGE_MAP()


// CSpinAnalyzerDlg 메시지 처리기

BOOL CSpinAnalyzerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	CRect rect;
	GetDlgItem(IDC_ST_RESULT)->GetWindowRect(rect);

	m_spinimage.SetDiameter(m_fDiameter);

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CSpinAnalyzerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CSpinAnalyzerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CSpinAnalyzerDlg::OnBnClickedBtnLoad()
{
	CString filter = _T("Spin Image Files(*.png;*.jpg;*.bmp)|*.png;*.jpg;*.bmp|");

	CFileDialog	dlg(TRUE, NULL, NULL, OFN_EXPLORER | OFN_FILEMUSTEXIST, filter, NULL, NULL);

	if (dlg.DoModal() == IDOK)
	{
		CString filename = dlg.GetPathName();
		cv::String str;
		str = CT2CA(filename);

		if( !m_spinimage.LoadSpinImage(str) )
		{
			AfxMessageBox(_T("Can not open image file! "));
			return;
		}

		m_nIndex = 0;
		UpdateData(FALSE);

		if (m_bAuto)
		{
			OnBnClickedBtnDetect();
		}
		else if (m_spinimage.GetSpinImage(0, m_image, true))
			RenderBitmapToScreen(IDC_ST_DISPLAY, m_image.data, m_image.cols, m_image.rows, 24, true);
			
	}
}

BOOL CSpinAnalyzerDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE)
		{
			cv::destroyAllWindows();
			return TRUE;
		}
		if (pMsg->wParam == VK_RETURN)
		{
			OnBnClickedBtnLoad();
			return TRUE;
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

int CSpinAnalyzerDlg::RenderBitmapToScreen(int nDisplayID, const BYTE* bitmap, const int nWidth, const int nHeight, const int nBitsPixel, const bool bStretch)
{
	CWnd* pVideoFrameWnd = GetDlgItem(nDisplayID);

	CClientDC dc(pVideoFrameWnd);

	CRect rectClient;
	pVideoFrameWnd->GetClientRect(rectClient);

	BITMAPINFOHEADER bmih;
	ZeroMemory(&bmih, sizeof(BITMAPINFOHEADER));

	bmih.biSize = sizeof(BITMAPINFOHEADER);
	bmih.biWidth = nWidth;
	bmih.biHeight = -(LONG)nHeight;	// Top-down Bitmap
	bmih.biPlanes = 1;
	bmih.biBitCount = nBitsPixel;
	bmih.biCompression = BI_RGB;
	bmih.biSizeImage = 0;
	bmih.biXPelsPerMeter = 0;
	bmih.biYPelsPerMeter = 0;
	bmih.biClrUsed = 0;
	bmih.biClrImportant = 0;

	SetStretchBltMode(dc.GetSafeHdc(), COLORONCOLOR);

	CRect rectDest;

	if (bStretch)
	{
		rectDest = rectClient;
	}
	else
	{
		rectDest.SetRect(rectClient.left, rectClient.top, rectClient.left + nWidth, rectClient.top + nHeight);
	}

	/*
	StretchDIBits(
	_In_  HDC        hdc,
	_In_  int        XDest,
	_In_  int        YDest,
	_In_  int        nDestWidth,
	_In_  int        nDestHeight,
	_In_  int        XSrc,
	_In_  int        YSrc,
	_In_  int        nSrcWidth,
	_In_  int        nSrcHeight,
	_In_  const VOID       *lpBits,
	_In_  const BITMAPINFO *lpBitsInfo,
	_In_  UINT       iUsage,
	_In_  DWORD      dwRop
	);
	*/
	INT cScanLines = StretchDIBits(
		dc.GetSafeHdc(),
		rectDest.left,
		rectDest.top,
		rectDest.Width(),
		rectDest.Height(),
		0,
		0,
		nWidth,
		nHeight,
		bitmap,
		(BITMAPINFO*)&bmih,
		DIB_RGB_COLORS,
		SRCCOPY);

	return cScanLines;
}

void CSpinAnalyzerDlg::OnBnClickedBtnSet()
{
	UpdateData(TRUE);

	if (m_fX2 != 0. || m_fY2 != 0.) 
	{
		m_fEpsilon = m_spinimage.calculateEpsilon(m_fX, m_fY, m_fX2, m_fY2);
		m_fX2 = 0.f;
		m_fY2 = 0.f;

		UpdateData(FALSE);
	}

	if (m_spinimage.SetPosition(m_fX, m_fY, m_fEpsilon))
	{
		m_fZ = m_spinimage.z();

		m_fRadius = m_spinimage.radius();
		m_fTheta  = m_spinimage.theta();
		m_fPhi    = m_spinimage.phi();

		UpdateData(FALSE);
	
		cv::Mat img = m_spinimage.generate();
		RenderBitmapToScreen(IDC_ST_RESULT, img.data, img.cols, img.rows, 24, true);
	}
}


void CSpinAnalyzerDlg::OnDeltaposSpinX(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	
	UpdateData(TRUE);
	m_fX -= .05 * pNMUpDown->iDelta;
	UpdateData(FALSE);

	OnBnClickedBtnSet();

	*pResult = 0;
}

void CSpinAnalyzerDlg::OnDeltaposSpinY(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	
	UpdateData(TRUE);
	m_fY -= .05 * pNMUpDown->iDelta;
	UpdateData(FALSE);

	OnBnClickedBtnSet();

	*pResult = 0;
}

void CSpinAnalyzerDlg::OnDeltaposSpinEpsilon(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	
	UpdateData(TRUE);
	m_fEpsilon -= pNMUpDown->iDelta;
	m_fEpsilon = m_fEpsilon < 0 ? m_fEpsilon + 360 : m_fEpsilon > 360 ? m_fEpsilon - 360 : m_fEpsilon;
	UpdateData(FALSE);

	OnBnClickedBtnSet();

	*pResult = 0;
}

void CSpinAnalyzerDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if ((nFlags & MK_LBUTTON) == MK_LBUTTON
	||	(nFlags & MK_RBUTTON) == MK_RBUTTON )
	{
		CRect rect;
		GetDlgItem(IDC_ST_RESULT)->GetWindowRect(rect);
		ScreenToClient(rect);

		if (rect.left   < point.x
		&&	rect.top    < point.y
		&&	rect.right  > point.x
		&&	rect.bottom > point.y)
		{
			if ((nFlags & MK_LBUTTON) == MK_LBUTTON)
			{
				m_fX  =  2.f * m_fRadius * ((point.x - rect.left) / (float)rect.Width() - 0.5f);
				m_fY  = -2.f * m_fRadius * ((point.y - rect.top) / (float)rect.Height() - 0.5f);
			}
			else
			{
				m_fX2 =  2.f * m_fRadius * ((point.x - rect.left) / (float)rect.Width() - 0.5f);
				m_fY2 = -2.f * m_fRadius * ((point.y - rect.top) / (float)rect.Height() - 0.5f);
			}

#if 0
			CString strMsg = _T("");
			strMsg.Format(TEXT("OnMouseMove() (%d, %d) - (%.2f, %.2f)\n"), point.x, point.y, m_fX, m_fY);
			TRACE(strMsg);
#endif
			UpdateData(FALSE);

			OnBnClickedBtnSet();
		}
	}

	CDialogEx::OnMouseMove(nFlags, point);
}

void CSpinAnalyzerDlg::OnBnClickedBtnPre()
{
	UpdateData(TRUE);
	if (m_spinimage.GetSpinImage(--m_nIndex, m_image, true))
	{
		RenderBitmapToScreen(IDC_ST_DISPLAY, m_image.data, m_image.cols, m_image.rows, 24, true);
		UpdateData(FALSE);

		if (m_bAuto)	OnBnClickedBtnDetect();
	}
}

void CSpinAnalyzerDlg::OnBnClickedBtnForward()
{
	UpdateData(TRUE);
	if (m_spinimage.GetSpinImage(++m_nIndex, m_image, true))
	{
		RenderBitmapToScreen(IDC_ST_DISPLAY, m_image.data, m_image.cols, m_image.rows, 24, true);
		UpdateData(FALSE);

		if (m_bAuto)	OnBnClickedBtnDetect();
	}
}

void CSpinAnalyzerDlg::OnBnClickedBtnDetect()
{
	UpdateData(TRUE);

	cv::Mat source;
	if (m_spinimage.GetSpinImage(m_nIndex, source))
	{	
		std::vector<cv::Point2f> res = m_spinimage.Detect(source);

		if(res.size())
		{
			m_fX = res.at(0).x;
			m_fY = res.at(0).y;

			if (res.size() > 1)
			{
				m_fX2 = res.at(1).x;
				m_fY2 = res.at(1).y;
			}

			RenderBitmapToScreen(IDC_ST_DISPLAY, source.data, source.cols, source.rows, 24, true);
			UpdateData(FALSE);

			// generate ball posture
			OnBnClickedBtnSet();
		}
	}
}

void CSpinAnalyzerDlg::OnBnClickedChkAuto()
{
	UpdateData(TRUE);
}

void CSpinAnalyzerDlg::OnDeltaposSpinX2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

	UpdateData(TRUE);
	m_fX2 -= .05 * pNMUpDown->iDelta;
	UpdateData(FALSE);

	OnBnClickedBtnSet();

	*pResult = 0;
}

void CSpinAnalyzerDlg::OnDeltaposSpinY2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	
	UpdateData(TRUE);
	m_fY2 -= .05 * pNMUpDown->iDelta;
	UpdateData(FALSE);

	OnBnClickedBtnSet();

	*pResult = 0;
}
