#include "stdafx.h"
#include "BlobLabeling.h"

/// @brief CBlobLabeling 생성자
/// @remarks Labeling에 사용되는 각종 변수 초기화
CBlobLabeling::CBlobLabeling(void)
{
    m_nThreshold    = 0;
    m_nBlobs        = _DEF_MAX_BLOBS;
    m_Image         = NULL;

    m_vPoint        = NULL;

    m_recBlobs      = NULL;
    m_intBlobs      = NULL;
}

/// @brief CBlobLabeling 소멸자
/// @remarks Labeling에 사용된 각종 변수 해제
CBlobLabeling::~CBlobLabeling(void)
{
    if( m_Image != NULL )
    {
        delete m_Image;
        m_Image = NULL;
    }

    if( m_recBlobs != NULL )
    {
        delete m_recBlobs;
        m_recBlobs = NULL;
    }

    if( m_intBlobs != NULL )
    {
        delete m_intBlobs;
        m_intBlobs = NULL;
    }
}

/// @brief Parameter 설정
/// @remarks Labeling에 사용되는 이미지와 영역 크기 정보를 설정
/// @param[in] image        Labeling에 사용할 이미지. 0 또는 -1의 값을 가지는 1ch 이미지.
/// @param[in] nThreshold   Labeling 결과 한 Blob의 픽셀 수가 nThreshold 이하인 경우 제거
bool CBlobLabeling::SetParam(Mat& image, int nThreshold)
{
	if( image.type() != CV_8UC1 )	return false;

    if( m_recBlobs != NULL )
    {
        delete m_recBlobs;
        m_recBlobs	= NULL;
    }

    if( m_intBlobs != NULL )
    {
        delete m_intBlobs;
        m_intBlobs	= NULL;
    }

    if( m_Image != NULL )
    {
        delete m_Image;
        m_Image = NULL;
    }

	m_nWidth = image.cols;
	m_nHeight = image.rows;

    m_Image = new int [ m_nWidth * m_nHeight ];

    int i,j;

    for(j=0;j<m_nHeight;j++)
    for(i=0;i<m_nWidth ;i++)
    {
        // object를 -1로 체크
//        unsigned char val = (unsigned char)cvGet2D( image, j, i ).val[0];
		unsigned char val = image.at<uchar>(j, i);
        m_Image[j*m_nWidth+i] = ( val == 0 ? 0 : -1 );
    }

    m_nThreshold	= nThreshold;
    m_nBlobs        = _DEF_MAX_BLOBS;

    return true;
}

/// @brief Labeling 실행
void CBlobLabeling::DoLabeling()
{
    m_nBlobs = Labeling(m_Image, m_nWidth, m_nHeight, m_nThreshold);
}

/// @brief Labeling 실행
/// @param[in] image        Labeling에 사용할 이미지. 0 또는 -1의 값을 가지는 1ch 이미지.
/// @param[in] nThreshold   Labeling 결과 한 Blob의 픽셀 수가 nThreshold 이하인 경우 제거
/// @return Label 개수
int CBlobLabeling::Labeling(int* DataBuf, int nWidth, int nHeight, int nThreshold)
{
    int nNumber;

    // 레이블링을 위한 포인트 초기화
    InitvPoint(nWidth, nHeight);

    // 레이블링
    nNumber = _Labeling(DataBuf, nWidth, nHeight, nThreshold);

    // 포인트 메모리 해제
    DeletevPoint();

    if( nNumber != 0 )
    {
        m_recBlobs = new CvRect [nNumber];
        m_intBlobs = new int [nNumber];

        DetectLabelingRegion(nNumber, DataBuf, nWidth, nHeight);
    }

    return nNumber;
}

/// @brief m_vPoint 초기화
void CBlobLabeling::InitvPoint(int nWidth, int nHeight)
{
    int nX, nY;

    m_vPoint = new Visited [nWidth * nHeight];

    for(nY = 0; nY < nHeight; nY++)
    {
        for(nX = 0; nX < nWidth; nX++)
        {
            m_vPoint[nY * nWidth + nX].bVisitedFlag		= false;
            m_vPoint[nY * nWidth + nX].ptReturnPoint.x	= nX;
            m_vPoint[nY * nWidth + nX].ptReturnPoint.y	= nY;
        }
    }
}

/// @brief m_vPoint 해제
void CBlobLabeling::DeletevPoint()
{
    delete m_vPoint;
    m_vPoint = NULL;
}

/// @brief Labeling 동작 함수
/// @remarks Size가 nWidth이고 nHeight인 DataBuf에서 nThreshold보다 작은 영역을 제외한 나머지를 blob으로 획득
/// @param[in] DataBuf      Labeling에 사용할 이미지 데이터 버퍼. 0 또는 -1의 값을 가짐.
/// @param[in] nWidth       데이터 버퍼의 가로 길이
/// @param[in] nHeight      데이터 버퍼의 가로 길이
/// @param[in] nThreshold   Labeling 결과 한 Blob의 픽셀 수가 nThreshold 이하인 경우 제거
/// @return Label 개수
int CBlobLabeling::_Labeling(int *DataBuf, int nWidth, int nHeight, int nThreshold)
{
    int num = 0;
    int nX, nY, k, l;
    int StartX , StartY, EndX , EndY;

    // Find connected components
    for(nY = 0; nY < nHeight; nY++)
    {
        for(nX = 0; nX < nWidth; nX++)
        {
            if( DataBuf[nY * nWidth + nX] == -1 )    // Is this a new component?
            {
                num++;

                DataBuf[nY * nWidth + nX] = num;

                StartX = nX, StartY = nY, EndX = nX, EndY= nY;

                __NRFIndNeighbor(DataBuf, nWidth, nHeight, nX, nY, &StartX, &StartY, &EndX, &EndY);

                if(__Area(DataBuf, StartX, StartY, EndX, EndY, nWidth, num) < nThreshold)
                {
                    for(k = StartY; k <= EndY; k++)
                    {
                        for(l = StartX; l <= EndX; l++)
                        {
                            if(DataBuf[k * nWidth + l] == num)
                                DataBuf[k * nWidth + l] = 0;
                        }
                    }
                    --num;

                    if(num > _DEF_MAX_BLOBS)    return  _DEF_MAX_BLOBS;
                }
            }
        }
    }

    return num;
}

/// @brief Labeling 결과의 Blob 정보를 구하는 함수
/// @param[in] nLabelNumber 해당 Blob의 Label
/// @param[in] DataBuf      Labeling Result
/// @param[in] nWidth       DataBuf 가로 크기
/// @param[in] nHeight      DataBuf 세로 크기
void CBlobLabeling::DetectLabelingRegion(int nLabelNumber, int* DataBuf, int nWidth, int nHeight)
{
    int nX, nY;
    int nLabelIndex ;

    bool* bFirstFlag = new bool [ nLabelNumber ];
    for(int b=0;b<nLabelNumber;b++) bFirstFlag[b] = false;

    for(nY = 0; nY < nHeight; nY++)
    {
        for(nX = 0; nX < nWidth; nX++)
        {
            nLabelIndex = DataBuf[nY * nWidth + nX];

            if(nLabelIndex > 0)	// Is this a new component?
            {
                if(bFirstFlag[nLabelIndex-1] == false)
                {
                    m_recBlobs[nLabelIndex-1].x         = nX;
                    m_recBlobs[nLabelIndex-1].y     	= nY;
                    m_recBlobs[nLabelIndex-1].width 	= 0;
                    m_recBlobs[nLabelIndex-1].height	= 0;

                    bFirstFlag[nLabelIndex-1] = true;
                }
                else
                {
                    int left	= m_recBlobs[nLabelIndex-1].x;
                    int right	= left + m_recBlobs[nLabelIndex-1].width;
                    int top     = m_recBlobs[nLabelIndex-1].y;
                    int bottom	= top + m_recBlobs[nLabelIndex-1].height;

                    if( left   >= nX )	left	= nX;
                    if( right  <= nX )	right	= nX+1;
                    if( top    >= nY )	top 	= nY;
                    if( bottom <= nY )	bottom	= nY+1;

                    m_recBlobs[nLabelIndex-1].x     	= left;
                    m_recBlobs[nLabelIndex-1].y     	= top;
                    m_recBlobs[nLabelIndex-1].width 	= right - left;
                    m_recBlobs[nLabelIndex-1].height	= bottom - top;

                    m_intBlobs[nLabelIndex-1]       	= nLabelIndex;
                }
            }
        }
    }

    delete bFirstFlag;
}

/// @brief Blob Labeling을 실제 행하는 함수
/// @param[in] DataBuf   Labeling에 사용할 이미지 데이터 버퍼. 0 또는 -1의 값을 가짐.
/// @param[in] nWidth   데이터 버퍼의 가로 길이
/// @param[in] nHeight  데이터 버퍼의 가로 길이
/// @param[in] nPosX    현재 탐색중인 X좌표
/// @param[in] nPosY    현재 탐색중인 Y좌표
/// @param[out] StartX  탐색 시작지점 X좌표
/// @param[out] StartY  탐색 시작지점 Y좌표
/// @param[out] EndX    탐색 종료지점 X좌표
/// @param[out] EndY    탐색 종료지점 Y좌표
/// @return 0
int CBlobLabeling::__NRFIndNeighbor(int* DataBuf, int nWidth, int nHeight, int nPosX, int nPosY, int *StartX, int *StartY, int *EndX, int *EndY )
{
    CvPoint CurrentPoint;

    CurrentPoint.x = nPosX;
    CurrentPoint.y = nPosY;

    m_vPoint[CurrentPoint.y * nWidth +  CurrentPoint.x].bVisitedFlag    = true;
    m_vPoint[CurrentPoint.y * nWidth +  CurrentPoint.x].ptReturnPoint.x = nPosX;
    m_vPoint[CurrentPoint.y * nWidth +  CurrentPoint.x].ptReturnPoint.y = nPosY;

    while(1)
    {
        if( (CurrentPoint.x != 0) && (DataBuf[CurrentPoint.y * nWidth + CurrentPoint.x - 1] == -1) )   // -X 방향
        {
            if( m_vPoint[CurrentPoint.y * nWidth +  CurrentPoint.x - 1].bVisitedFlag == false )
            {
                DataBuf[CurrentPoint.y  * nWidth + CurrentPoint.x  - 1]                 = DataBuf[CurrentPoint.y * nWidth + CurrentPoint.x];	// If so, mark it
                m_vPoint[CurrentPoint.y * nWidth +  CurrentPoint.x - 1].bVisitedFlag	= true;
                m_vPoint[CurrentPoint.y * nWidth +  CurrentPoint.x - 1].ptReturnPoint	= CurrentPoint;
                CurrentPoint.x--;

                if(CurrentPoint.x <= 0)
                    CurrentPoint.x = 0;

                if(*StartX >= CurrentPoint.x)
                    *StartX = CurrentPoint.x;

                continue;
            }
        }

        if( (CurrentPoint.x != nWidth - 1) && (DataBuf[CurrentPoint.y * nWidth + CurrentPoint.x + 1] == -1) )   // -X 방향
        {
            if( m_vPoint[CurrentPoint.y * nWidth +  CurrentPoint.x + 1].bVisitedFlag == false )
            {
                DataBuf[CurrentPoint.y * nWidth + CurrentPoint.x + 1]       		= DataBuf[CurrentPoint.y * nWidth + CurrentPoint.x];	// If so, mark it
                m_vPoint[CurrentPoint.y * nWidth +  CurrentPoint.x + 1].bVisitedFlag	= true;
                m_vPoint[CurrentPoint.y * nWidth +  CurrentPoint.x + 1].ptReturnPoint	= CurrentPoint;
                CurrentPoint.x++;

                if(CurrentPoint.x >= nWidth - 1)
                    CurrentPoint.x = nWidth - 1;

                if(*EndX <= CurrentPoint.x)
                    *EndX = CurrentPoint.x;

                continue;
            }
        }

        if( (CurrentPoint.y != 0) && (DataBuf[(CurrentPoint.y - 1) * nWidth + CurrentPoint.x] == -1) )   // -X 방향
        {
            if( m_vPoint[(CurrentPoint.y - 1) * nWidth +  CurrentPoint.x].bVisitedFlag == false )
            {
                DataBuf[(CurrentPoint.y - 1) * nWidth + CurrentPoint.x]                 = DataBuf[CurrentPoint.y * nWidth + CurrentPoint.x];	// If so, mark it
                m_vPoint[(CurrentPoint.y - 1) * nWidth +  CurrentPoint.x].bVisitedFlag	= true;
                m_vPoint[(CurrentPoint.y - 1) * nWidth +  CurrentPoint.x].ptReturnPoint = CurrentPoint;
                CurrentPoint.y--;

                if(CurrentPoint.y <= 0)
                    CurrentPoint.y = 0;

                if(*StartY >= CurrentPoint.y)
                    *StartY = CurrentPoint.y;

                continue;
            }
        }

        if( (CurrentPoint.y != nHeight - 1) && (DataBuf[(CurrentPoint.y + 1) * nWidth + CurrentPoint.x] == -1) )   // -X 방향
        {
            if( m_vPoint[(CurrentPoint.y + 1) * nWidth +  CurrentPoint.x].bVisitedFlag == false )
            {
                DataBuf[(CurrentPoint.y + 1) * nWidth + CurrentPoint.x]                 = DataBuf[CurrentPoint.y * nWidth + CurrentPoint.x];	// If so, mark it
                m_vPoint[(CurrentPoint.y + 1) * nWidth +  CurrentPoint.x].bVisitedFlag	= true;
                m_vPoint[(CurrentPoint.y + 1) * nWidth +  CurrentPoint.x].ptReturnPoint = CurrentPoint;
                CurrentPoint.y++;

                if(CurrentPoint.y >= nHeight - 1)
                    CurrentPoint.y = nHeight - 1;

                if(*EndY <= CurrentPoint.y)
                    *EndY = CurrentPoint.y;

                continue;
            }
        }

        if(     (CurrentPoint.x == m_vPoint[CurrentPoint.y * nWidth + CurrentPoint.x].ptReturnPoint.x)
            &&	(CurrentPoint.y == m_vPoint[CurrentPoint.y * nWidth + CurrentPoint.x].ptReturnPoint.y) )
        {
            break;
        }
        else
        {
            CurrentPoint = m_vPoint[CurrentPoint.y * nWidth + CurrentPoint.x].ptReturnPoint;
        }
    }

    return 0;
}

/// @brief Blob 영역 중 지정된 Label을 가진 영역의 크기(픽셀 수)를 구하는 함수
/// @param[in] DataBuf   Labeling에 사용할 이미지 데이터 버퍼
/// @param[in] StartX   탐색 시작지점 X좌표
/// @param[in] StartY   탐색 시작지점 Y좌표
/// @param[in] EndX     탐색 종료지점 X좌표
/// @param[in] EndY     탐색 종료지점 Y좌표
/// @param[in] nWidth   데이터 버퍼의 가로 길이
/// @param[in] nLevel   지정된 Blob의 Label
/// @return 탐색영역 내에서 지정된 Label의 픽셀 수
int CBlobLabeling::__Area(int* DataBuf, int StartX, int StartY, int EndX, int EndY, int nWidth, int nLevel)
{
    int nArea = 0;
    int nX, nY;

    for (nY = StartY; nY < EndY; nY++)
        for (nX = StartX; nX < EndX; nX++)
            if (DataBuf[nY * nWidth + nX] == nLevel)
                ++nArea;

    return nArea;
}

/// @brief 가로/세로 비율이 지정한 비율보다 작은 Blob 제거
/// @remarks Width/Hight 가 fRatio보다 작을 경우, 그것들을 버리고, 결과를 rect로 변경\n
/// Input으로 들어온 rect와 그것의 개수 nRecNumber는 수행후, 변경된 값 들어감
/// @param[in] fRatio 가로/세로 비율
void CBlobLabeling::BlobWidthHeightSmallRatioConstraint(float fRatio)
{
    m_nBlobs = _BlobWidthHeightSmallRatioConstraint(fRatio, m_nBlobs, m_recBlobs, m_intBlobs);
}

/// @brief 가로/세로 비율이 지정한 비율보다 작은 Blob 제거하는 실제 함수
/// @param[in] fRatio   가로/세로 비율
/// @param[in] rect     Blob 영역 정보
/// @param[in] blobs    Blob Index 정보
/// @param[in] nRecNumber Blob 개수
/// @return 정리된 Blob 개수
int CBlobLabeling::_BlobWidthHeightSmallRatioConstraint(float fRatio, int nRecNumber, CvRect* rect, int* blobs)
{
    if(nRecNumber == 0)	return 0;

    int nX;
    int ntempRec = 0;

    CvRect *temp = new CvRect[nRecNumber];
    int *labeled = new int[nRecNumber];

    for(nX = 0; nX < nRecNumber; nX++)
    {
        temp[nX]    = rect[nX];
        labeled[nX] = blobs[nX];
    }

    for(nX = 0; nX < nRecNumber; nX++)
    {
        if( rect[nX].height / (float)rect[nX].width > fRatio )
        {
            rect[ntempRec] = temp[nX];
            blobs[ntempRec] = labeled[nX];

            ntempRec++;
        }
    }

    delete temp;
    delete labeled;

    return ntempRec;
}

/// @brief 가로/세로 비율이 지정한 비율보다 큰 Blob 제거
/// @remarks Width/Hight 가 fRatio보다 작을 경우, 그것들을 버리고, 결과를 rect로 변경\n
/// Input으로 들어온 rect와 그것의 개수 nRecNumber는 수행후, 변경된 값 들어감
/// @param[in] fRatio 가로/세로 비율
void CBlobLabeling::BlobWidthHeightBigRatioConstraint(float fRatio)
{
    m_nBlobs = _BlobWidthHeightBigRatioConstraint(fRatio, m_nBlobs, m_recBlobs, m_intBlobs);
}

/// @brief 가로/세로 비율이 지정한 비율보다 큰 Blob 제거하는 실제 함수
/// @param[in] fRatio   가로/세로 비율
/// @param[in] rect     Blob 영역 정보
/// @param[in] blobs    Blob Index 정보
/// @param[in] nRecNumber Blob 개수
/// @return 정리된 Blob 개수
int CBlobLabeling::_BlobWidthHeightBigRatioConstraint(float fRatio, int nRecNumber, CvRect* rect, int* blobs)
{
    if(nRecNumber == 0)	return 0;

    int nX;
    int ntempRec = 0;

    CvRect *temp = new CvRect[nRecNumber];
    int *labeled = new int[nRecNumber];

    for(nX = 0; nX < nRecNumber; nX++)
    {
        temp[nX]    = rect[nX];
        labeled[nX] = blobs[nX];
    }

    for(nX = 0; nX < nRecNumber; nX++)
    {
        if( rect[nX].height / (float)rect[nX].width < fRatio )
        {
            rect[ntempRec] = temp[nX];
            blobs[ntempRec] = labeled[nX];

            ntempRec++;
        }
    }

    delete temp;
    delete labeled;

    return ntempRec;
}

/// @brief 지정한 nWidth와 nHeight보다 작은 Blob을 모두 제거
/// @param[in] nWidth   가로 크기 Threshold
/// @param[in] nHeight  세로 크기 Threshold
void CBlobLabeling::BlobSmallSizeConstraint(int nWidth, int nHeight)
{
    m_nBlobs = _BlobSmallSizeConstraint(nWidth, nHeight, m_nBlobs, m_recBlobs, m_intBlobs);
}

/// @brief 지정한 nWidth와 nHeight보다 작은 Blob을 모두 제거하는 실제 함수
/// @param[in] nWidth       가로 크기 Threshold
/// @param[in] nHeight      세로 크기 Threshold
/// @param[in] nRecNumber   Blob 개수
/// @param[in] rect         Blob 영역 정보
/// @param[in] blobs        Blob Index 정보
/// @return 정리된 Blob 개수
int CBlobLabeling::_BlobSmallSizeConstraint(int nWidth, int nHeight, int nRecNumber, CvRect* rect, int* label)
{
    if(nRecNumber == 0)	return 0;

    int nX;
    int ntempRec = 0;

    CvRect* temp = new CvRect[nRecNumber];
    int* labeled = new int[nRecNumber];

    for(nX = 0; nX < nRecNumber; nX++)
    {
        temp[nX]    = rect[nX];
        labeled[nX] = label[nX];
    }

    for(nX = 0; nX < nRecNumber; nX++)
    {
        if( (rect[nX].width >= nWidth) && (rect[nX].height >= nHeight) )
        {
            temp[ntempRec] = rect[nX];
            labeled[ntempRec] = label[nX];

            ntempRec++;
        }
    }

    for(nX = 0; nX < ntempRec; nX++)
    {
        rect[nX] = temp[nX];
        label[nX] = labeled[nX];
    }

    delete temp;
    delete labeled;

    return ntempRec;
}

/// @brief 지정한 nWidth와 nHeight보다 큰 Blob을 모두 제거
/// @param[in] nWidth   가로 크기 Threshold
/// @param[in] nHeight  세로 크기 Threshold
void CBlobLabeling::BlobBigSizeConstraint(int nWidth, int nHeight)
{
    m_nBlobs = _BlobBigSizeConstraint(nWidth, nHeight, m_nBlobs, m_recBlobs, m_intBlobs);
}

/// @brief 지정한 nWidth와 nHeight보다 큰 Blob을 모두 제거하는 실제 함수
/// @param[in] nWidth       가로 크기 Threshold
/// @param[in] nHeight      세로 크기 Threshold
/// @param[in] nRecNumber   Blob 개수
/// @param[in] rect         Blob 영역 정보
/// @param[in] blobs        Blob Index 정보
/// @return 정리된 Blob 개수
int CBlobLabeling::_BlobBigSizeConstraint(int nWidth, int nHeight, int nRecNumber, CvRect* rect, int* label)
{
    if(nRecNumber == 0)	return 0;

    int nX;
    int ntempRec = 0;

    CvRect* temp = new CvRect [nRecNumber];
    int* labeled = new int [nRecNumber];

    for(nX = 0; nX < nRecNumber; nX++)
    {
        temp[nX] = rect[nX];
        labeled[nX] = label[nX];
    }

    for(nX = 0; nX < nRecNumber; nX++)
    {
        if( (rect[nX].width < nWidth) && (rect[nX].height < nHeight) )
        {
            temp[ntempRec] = rect[nX];
            labeled[ntempRec] = label[nX];

            ntempRec++;
        }
    }

    for(nX = 0; nX < ntempRec; nX++)
    {
        rect[nX] = temp[nX];
        label[nX] = labeled[nX];
    }

    delete temp;
    delete labeled;

    return ntempRec;
}

/// @brief 이미지 외곽 부분으로부터 지정한 거리 안에 있는(이미지 외곽에 붙은) Blob을 제거
/// @param[in] marginX  가로 거리 Threshold
/// @param[in] marginY  세로 거리 Threshold
void CBlobLabeling::BlobEdgeConstraint(int marginX, int marginY)
{
    m_nBlobs = _BlobEdgeConstraint(marginX, marginY, m_nBlobs, m_recBlobs, m_intBlobs);
}

/// @brief 이미지 외곽 부분으로부터 지정한 거리 안에 있는 Blob을 제거하는 실제 함수
/// @param[in] marginX      가로 거리 Threshold
/// @param[in] marginY      세로 거리 Threshold
/// @param[in] nRecNumber   Blob 개수
/// @param[in] rect         Blob 영역 정보
/// @param[in] blobs        Blob Index 정보
/// @return 정리된 Blob 개수
int CBlobLabeling::_BlobEdgeConstraint(int marginX, int marginY, int nRecNumber, CvRect *rect, int *label)
{
    if(nRecNumber == 0)	return 0;

    int nX;
    int ntempRec = 0;

    CvRect* temp = new CvRect [nRecNumber];
    int* labeled = new int [nRecNumber];

    for(nX = 0; nX < nRecNumber; nX++)
    {
            temp[nX] = rect[nX];
            labeled[nX] = label[nX];
    }

    for(nX = 0; nX < nRecNumber; nX++)
    {
        int l = rect[nX].x;
        int t = rect[nX].y;
        int r = rect[nX].x + rect[nX].width;
        int b = rect[nX].y + rect[nX].height;

        if(     l > marginX && r < m_nWidth - marginX
            &&  t > marginY && b < m_nHeight - marginY )
        {
                temp[ntempRec] = rect[nX];
                labeled[ntempRec] = label[nX];

                ntempRec++;
        }
    }

    for(nX = 0; nX < ntempRec; nX++)
    {
            rect[nX] = temp[nX];
            label[nX] = labeled[nX];
    }

    delete temp;
    delete labeled;

    return ntempRec;
}

/// @brief 이미지를 둘러 싼 영역 내에서 실제 객체가 차지하는 비율이 지정한 값 보다 적은 Blob을 제거
/// @param[in] ratio 객체가 차지하는 비율 하한치
void CBlobLabeling::BlobIncludeRatioConstraint(float ratio)
{
    m_nBlobs = _BlobIncludeRatioConstraint(ratio, m_nBlobs, m_recBlobs, m_intBlobs);
}

/// @brief 이미지를 둘러 싼 영역 내에서 실제 객체가 차지하는 비율이 지정한 값 보다 적은 Blob을 제거하는 실제 함수
/// @param[in] ratio        객체가 차지하는 비율 하한치
/// @param[in] nRecNumber   Blob 개수
/// @param[in] rect         Blob 영역 정보
/// @param[in] blobs        Blob Index 정보
/// @return 정리된 Blob 개수
int CBlobLabeling::_BlobIncludeRatioConstraint(float ratio, int nRecNumber, CvRect *rect, int *label)
{
    if(nRecNumber == 0)	return 0;

    int nX;
    int ntempRec = 0;

    CvRect* temp = new CvRect [nRecNumber];
    int* labeled = new int [nRecNumber];

    for(nX = 0; nX < nRecNumber; nX++)
    {
        temp[nX] = rect[nX];
        labeled[nX] = label[nX];
    }

    for(nX = 0; nX < nRecNumber; nX++)
    {
        int cnt = 0;

        for( int j = 0; j < rect[nX].height; j++ )
        for( int i = 0; i < rect[nX].width;  i++ )
        {
            unsigned char val = m_Image[ (j + rect[nX].y) * m_nWidth + (i + rect[nX].x) ];

            if( val == label[nX] )	cnt++;
        }

        float constraint = (float)cnt / (rect[nX].width * rect[nX].height);

        if( constraint > ratio )
        {
                temp[ntempRec] = rect[nX];
                labeled[ntempRec] = label[nX];

                ntempRec++;
        }
    }

    for(nX = 0; nX < ntempRec; nX++)
    {
            rect[nX] = temp[nX];
            label[nX] = labeled[nX];
    }

    delete temp;
    delete labeled;

    return ntempRec;
}

void CBlobLabeling::ReArrangeRects(int nMethod)
{
	_ReArrangeRects(m_recBlobs, m_intBlobs, m_nBlobs, nMethod);
}

void CBlobLabeling::_ReArrangeRects(CvRect* rect, int* blobs, int nRecNumber, int nMethod)
{
	if( nRecNumber < 2 )	return;

	int i, j;
	CvRect	tmp;
	int		tmp_i;

	if		( nMethod == HORIZONTAL )
	{
		for(i=0;i<nRecNumber-1;i++)
		{
			for(j=i+1;j<nRecNumber;j++)
			{
				if( rect[i].x > rect[j].x )
				{
					tmp		= rect[i];
					rect[i] = rect[j];
					rect[j] = tmp;

					tmp_i		= blobs[i];
					blobs[i]	= blobs[j];
					blobs[j]	= tmp_i;
				}
			}
		}
	}
	else if ( nMethod == VERTICAL )
	{
		for(i=0;i<nRecNumber-1;i++)
		{
			for(j=i+1;j<nRecNumber;j++)
			{
				if(		 (rect[i].y >  rect[j].y)
					||	((rect[i].y == rect[j].y) && (rect[i].x > rect[j].x) ) )
				{
					tmp		= rect[i];
					rect[i] = rect[j];
					rect[j] = tmp;

					tmp_i		= blobs[i];
					blobs[i]	= blobs[j];
					blobs[j]	= tmp_i;
				}	
			}
		}
	}
	else if ( nMethod == BLOB_SIZE )
	{
		for(i=0;i<nRecNumber-1;i++)
		{
			for(j=i+1;j<nRecNumber;j++)
			{
				if(	rect[i].width * rect[i].height < rect[j].width * rect[j].height )
				{
					tmp		= rect[i];
					rect[i] = rect[j];
					rect[j] = tmp;

					tmp_i		= blobs[i];
					blobs[i]	= blobs[j];
					blobs[j]	= tmp_i;
				}	
			}
		}
	}
}

/// @brief 원하는 블롭의 이미지 정보만 가져온다
/// @remarks 원하는 블롭의 이미지만 복사하므로, 함수 실행 이전에 이미지변수는 초기화 되어야 함.
/// @param[out] dest    블롭 이미지
/// @param[in] nLabel   원하는 Blob의 Label
/// @param[in] nX       image offset x좌표
/// @param[in] nY       image offset y좌표
void CBlobLabeling::GetBlobImage(Mat& dest, int nLabel, int nX, int nY)
{
    CvRect rect = m_recBlobs[ nLabel ];
    int nNum	= m_intBlobs[ nLabel ];

    for( int j = 0; j < rect.height; j++ )
    for( int i = 0; i < rect.width;  i++ )
    {
        int val = m_Image[ (j + rect.y) * m_nWidth + (i + rect.x) ];

//        if( val == nNum )   dest->imageData[ (nY + j) * dest->widthStep + (nX + i) ] = (unsigned char)255;
		if( val == nNum )	dest.at<uchar>( nY + j, nX + i ) = (uchar)255;
    }
}
