#pragma once

#include <opencv/cv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#define DEFAULT_SIZE	400
#define RECT_BORDER		 10

#ifndef PI
#define PI 3.14159265359
#endif

#ifndef RAD2DEG
#define RAD2DEG(x)	(x * 180.0 / PI)
#endif

#ifndef DEG2RAD
#define DEG2RAD(x)	(x * PI / 180.0)
#endif

class CPSSpinImage
{
public:
	CPSSpinImage();
	~CPSSpinImage();

	/**
	 *	for real image
	 */
	std::vector<cv::Mat> m_images;

	bool LoadSpinImage(cv::String filepath);
	bool LoadSpinImage(cv::Mat& image);

	bool GetSpinImage(int idx, cv::Mat& image, bool bShowGuide = false);

	std::vector<cv::Point2f> Detect(cv::Mat& image);

	/**
	 *	for generated image
	 */
	void ResetImageSize(int sz);

	void SetDiameter(float _d);
	bool SetPosition(float _x, float _y, float _epsilon = 0.f);

	float calculateEpsilon(float _x, float _y, float _x2, float _y2);

	float x() { return m_fX; }
	float y() { return m_fY; }
	float z() { return m_fZ; }

	float radius() { return m_fRadius; }
	float theta() { return m_fTheta; }
	float phi() { return m_fPhi; }

	void rotate(float _x0, float _y0, float _z0, float theta, float phi, float epsilon, float& _x, float& _y, float& _z);

	cv::Mat generate(cv::Mat source = cv::Mat());

private:
	float m_fX;
	float m_fY;
	float m_fZ;
	float m_fDiameter;
	float m_fRadius;
	float m_fTheta;
	float m_fPhi;
	float m_fEpsilon;

	cv::Mat m_image;
};

