
// SpinAnalyzerDlg.h: 헤더 파일
//

#pragma once

#include "spin_image.h"

// CSpinAnalyzerDlg 대화 상자
class CSpinAnalyzerDlg : public CDialogEx
{
// 생성입니다.
public:
	CSpinAnalyzerDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SPINANALYZER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnLoad();
	afx_msg void OnBnClickedBtnSet();
	afx_msg void OnBnClickedBtnPre();
	afx_msg void OnBnClickedBtnForward();
	afx_msg void OnBnClickedBtnDetect();

	afx_msg void OnDeltaposSpinX(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpinY(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpinEpsilon(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnBnClickedChkAuto();
	
	BOOL m_bAuto;
		
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	
	int RenderBitmapToScreen(int nDisplayID, const BYTE* bitmap, const int nWidth, const int nHeight, const int nBitsPixel, const bool bStretch);

	CPSSpinImage m_spinimage;

	cv::Mat m_image;

	float m_fX;
	float m_fY;
	float m_fZ;

	float m_fDiameter;
	float m_fRadius;

	float m_fTheta;
	float m_fPhi;
	float m_fEpsilon;
	
	float m_fX2;	// second point-X
	float m_fY2;	// second point-Y

	int m_nIndex;
	
	afx_msg void OnDeltaposSpinX2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpinY2(NMHDR *pNMHDR, LRESULT *pResult);
};
