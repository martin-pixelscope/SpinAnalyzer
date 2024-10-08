#pragma once

#include <opencv/cv.hpp>

class CPSSpinAnalyzer
{
public:
	CPSSpinAnalyzer();
	~CPSSpinAnalyzer();

	std::vector<cv::Mat> m_balls;
};

