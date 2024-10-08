#pragma once

#include <opencv/cv.h>
#include <opencv/highgui.h>

using namespace cv;

#define HORIZONTAL	0
#define	VERTICAL	1
#define BLOB_SIZE	2

#define _DEF_MAX_BLOBS	10000   ///<    �ִ� BLOB ����

#define max(a, b)	(a > b ? a : b)
#define min(a, b)	(a < b ? a : b)

/// @struct Visited
/// @brief  Labeling �� �湮���θ� �����ϴ� ����ü
typedef struct
{
        bool	bVisitedFlag;
        CvPoint ptReturnPoint;

} Visited;

/// @class CBlobLabeling
/// @brief Labeling ���� �� �� Blob�� ���� �̿��� ���� Class
class  CBlobLabeling
{
public:
	CBlobLabeling(void);
	~CBlobLabeling(void);

private:
        int*    	m_Image;        ///< ���̺��� ���� �̹���(0, -1 �Ǵ� ���̺� ��ȣ���� ������ 1ch �̹���)
        int         m_nWidth;       ///< �̹����� ���� ũ��
        int         m_nHeight;      ///< �̹����� ���� ũ��

        int         m_nThreshold;   ///< ���̺� Threshold(�ȼ��� ��)
        Visited*	m_vPoint;       ///< ���̺��� �湮����

public:
        int         m_nBlobs;       ///< ���̺��� ����
	
        CvRect*		m_recBlobs;     ///< �� ���̺� ��������
        int*		m_intBlobs;     ///< �� ���̺� �ε���

public:
        // ���̺� �̹��� ����
        bool	SetParam(Mat& image, int nThreshold);

        // ���̺�(����)
        void	DoLabeling();

private:
        // ���̺�(����)
        int     Labeling(int* DataBuf, int nWidth, int nHeight, int nThreshold);
        void    DetectLabelingRegion(int nLabelNumber, int* DataBuf, int nWidth, int nHeight);

        // ����Ʈ �ʱ�ȭ
        void	InitvPoint(int nWidth, int nHeight);
        void	DeletevPoint();

        // ���̺�(���� �˰���)
        int     _Labeling(int* DataBuf, int nWidth, int nHeight, int nThreshold);
	
        // _Labling ���� ��� �Լ�
        int     __NRFIndNeighbor(int* DataBuf, int nWidth, int nHeight, int nPosX, int nPosY, int *StartX, int *StartY, int *EndX, int *EndY );
        int     __Area(int* DataBuf, int StartX, int StartY, int EndX, int EndY, int nWidth, int nLevel);


        // ����-���κ� �񱳰� ���� ���� ���̺� ����
public:
	void	BlobWidthHeightSmallRatioConstraint(float fRatio);
private:
        int	_BlobWidthHeightSmallRatioConstraint(float fRatio, int nRecNumber, CvRect* rect, int* label);

		// ����-���κ� �񱳰� ���� ū ���̺� ����
public:
	void	BlobWidthHeightBigRatioConstraint(float fRatio);
private:
        int	_BlobWidthHeightBigRatioConstraint(float fRatio, int nRecNumber, CvRect* rect, int* label);

        // ����, ���� ũ�Ⱑ �񱳰� ���� ���� ���̺� ����
public:
	void	BlobSmallSizeConstraint(int nWidth, int nHeight);
private:
        int	_BlobSmallSizeConstraint(int nWidth, int nHeight, int nRecNumber, CvRect* rect, int* label);

        // ����, ���� ũ�Ⱑ �񱳰� ���� ū ���̺� ����
public:
	void	BlobBigSizeConstraint(int nWidth, int nHeight);
private:
        int	_BlobBigSizeConstraint(int nWidth, int nHeight, int nRecNumber, CvRect* rect, int* label);

        // �߽����� �̹����� �𼭸��� �����ϴ� ���̺� ����
public:
        void    BlobEdgeConstraint(int marginX, int marginY);
private:
        int    _BlobEdgeConstraint(int marginX, int marginY, int nRecNumber, CvRect* rect, int* label);

        // ��ü�� ȭ�� ���� ũ�� ������ �񱳰� ���� ���� ���̺� ����
public:
        void    BlobIncludeRatioConstraint(float ratio);
private:
        int    _BlobIncludeRatioConstraint(float ratio, int nRecNumber, CvRect* rect, int* label);

public:
	void	ReArrangeRects(int nMethod = VERTICAL);
private:
	void	_ReArrangeRects(CvRect* rect, int* blobs, int nRecNumber, int nMethod);

        // ���ϴ� ����� �̹��� ������ �����´�.
public:
	void	GetBlobImage(Mat& dest, int nLabel, int nX = 0, int nY = 0);
};
