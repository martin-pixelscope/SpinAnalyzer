#include "stdafx.h"
#include "spin_image.h"

#include "bloblabeling.h"

CPSSpinImage::CPSSpinImage()
{
	m_image = cv::Mat(DEFAULT_SIZE, DEFAULT_SIZE, CV_8UC3);

	m_fX = 0.f;
	m_fY = 0.f;
	m_fZ = 0.f;

	m_fDiameter = 0.f;
	m_fRadius = 0.f;
	m_fTheta = 0.f;
	m_fPhi  = 0.f;
}

CPSSpinImage::~CPSSpinImage()
{
}

bool CPSSpinImage::LoadSpinImage(cv::String filepath)
{
	cv::Mat image = cv::imread(filepath);

	return LoadSpinImage(image);
}

bool CPSSpinImage::LoadSpinImage(cv::Mat & image)
{
	int sz = image.rows;

	if (sz == 0)	return false;

	m_images.clear();	// clear list

	int count = image.cols / sz;
	
	for (int i = 0; i < count; i++)
	{
#if 0
		cv::Mat resized;
		resize(image(cv::Rect(i * sz, 0, sz, sz)), resized, m_image.size());
		m_images.push_back(resized);
#else
		cv::Mat img = image(cv::Rect(i * sz, 0, sz, sz)).clone();
		m_images.push_back(img);
#endif
	}
	
	return true;
}

bool CPSSpinImage::GetSpinImage(int idx, cv::Mat& image, bool bShowGuide)
{
	int sz = m_images.size();
	if (sz <= 0 || idx < 0 || sz - 1 < idx)	return false;
	
	if (bShowGuide)
	{
		cv::resize(m_images.at(idx), image, cv::Size(m_image.cols, m_image.rows));

		cv::line(image, cv::Point(0, image.rows / 2), cv::Point(image.cols, image.rows / 2), CV_RGB(192, 192, 192), 1);
		cv::line(image, cv::Point(image.cols / 2, 0), cv::Point(image.cols / 2, image.rows), CV_RGB(192, 192, 192), 1);

		cv::circle(image, cv::Point(image.cols / 2, image.rows / 2), (image.cols - RECT_BORDER * 2) / 2, CV_RGB(192, 192, 192), 2);
	}
	else
		image = m_images.at(idx).clone();

	return true;
}

std::vector<cv::Point2f> CPSSpinImage::Detect(cv::Mat& image)
{
	std::vector<cv::Point2f> res = std::vector<cv::Point2f>();

	cv::Mat gray;
	cv::cvtColor(image, gray, CV_BGR2GRAY);
	
	cv::Mat g, dog, bin;

	// gaussian filtering
	int filter_size = 13;
	cv::GaussianBlur(gray, g, cv::Size(filter_size, filter_size), 0, 0);
	
	// generate mask
	float ratio = .85f;
	cv::Mat mask = cv::Mat(gray.rows, gray.cols, CV_8UC1);	mask.setTo(0);
	cv::circle(mask, cv::Point(gray.cols/2., gray.rows/2.), gray.cols * ratio / 2.f, cv::Scalar(255), -1);

	// difference of gaussian
	// Object will be dark. substract source image from from gaussian filtered image
	cv::subtract(g, gray, dog, mask);
	cv::threshold(dog, bin, 0., 255., CV_THRESH_BINARY | CV_THRESH_OTSU);

	// labeling to detect candidate blobs
	CBlobLabeling blob;
	blob.SetParam(bin, 5);
	blob.DoLabeling();
	blob.BlobBigSizeConstraint(gray.cols * .25, gray.rows * .25);	// reduce size over 25%
	blob.BlobWidthHeightBigRatioConstraint(2.01f);
	blob.BlobWidthHeightSmallRatioConstraint(.4f);
	blob.ReArrangeRects(BLOB_SIZE);	// sort by size

	if (blob.m_nBlobs)
	{
		cv::Point2f cp = cv::Point2f(blob.m_recBlobs[0].x + blob.m_recBlobs[0].width / 2., blob.m_recBlobs[0].y + blob.m_recBlobs[0].height / 2.);

		// draw result
		cv::line(image, cv::Point2f(cp.x - 2, cp.y), cv::Point2f(cp.x + 2, cp.y), CV_RGB(255, 0, 0));
		cv::line(image, cv::Point2f(cp.x, cp.y - 2), cv::Point2f(cp.x, cp.y + 2), CV_RGB(255, 0, 0));

		// set first blob as a main point
		res.push_back(cv::Point2f(2.f * m_fRadius * (cp.x / (float)gray.cols - 0.5f),
								 -2.f * m_fRadius * (cp.y / (float)gray.rows - 0.5f)));

		if (blob.m_nBlobs > 1)
		{
			cp = cv::Point2f(blob.m_recBlobs[1].x + blob.m_recBlobs[1].width / 2., blob.m_recBlobs[1].y + blob.m_recBlobs[1].height / 2.);

			// draw result
			cv::line(image, cv::Point2f(cp.x - 2, cp.y), cv::Point2f(cp.x + 2, cp.y), CV_RGB(255, 128, 128));
			cv::line(image, cv::Point2f(cp.x, cp.y - 2), cv::Point2f(cp.x, cp.y + 2), CV_RGB(255, 128, 128));

			// set second blob as a guide point
			res.push_back(cv::Point2f(2.f * m_fRadius * (cp.x / (float)gray.cols - 0.5f),
									 -2.f * m_fRadius * (cp.y / (float)gray.rows - 0.5f)));
		}
	}

#if 0	// only for debug
	cv::Mat result = cv::Mat(gray.rows, gray.cols * 4, CV_8UC1);
	gray.copyTo(result(cv::Rect(0, 0, gray.cols, gray.rows)));
	g.copyTo(result(cv::Rect(gray.cols, 0, gray.cols, gray.rows)));
	bin.copyTo(result(cv::Rect(gray.cols * 2, 0, gray.cols, gray.rows)));

	bin.setTo(0);
	for (int b = 0; b < MIN(2, blob.m_nBlobs); b++)
		blob.GetBlobImage(bin, b, blob.m_recBlobs[b].x, blob.m_recBlobs[b].y);
	bin.copyTo(result(cv::Rect(gray.cols * 3, 0, gray.cols, gray.rows)));
	
	cv::namedWindow("result", cv::WINDOW_NORMAL);
	cv::imshow("result", result);
	cv::waitKey(1);
#endif

	return res;
}

void CPSSpinImage::ResetImageSize(int sz)
{
	if (sz != 0)
	{
		m_image.release();
		m_image = cv::Mat(sz, sz, CV_8UC3);
	}
}

void CPSSpinImage::SetDiameter(float _d)
{
	m_fDiameter = _d;
	m_fRadius   = _d / 2.f;
}

float XY2Phi(float _x, float _y, bool bUse360 = false)
{
	float _phi = 0.f;

	if (bUse360)
	{
		if (_x > 0 && _y >= 0)
			_phi = RAD2DEG(atan(_y / _x));
		else if (_x > 0 && _y < 0)
			_phi = RAD2DEG(atan(_y / _x)) + 360.f;
		else if (_x < 0)
			_phi = RAD2DEG(atan(_y / _x)) + 180.f;
		else if (_x == 0 && _y > 0)
			_phi = 90.f;
		else if (_x == 0 && _y < 0)
			_phi = 270.f;
	}
	else
		_phi = RAD2DEG(atan(_y / _x));

	return _phi;
}

float CPSSpinImage::calculateEpsilon(float _x, float _y, float _x2, float _y2)
{
	if (_x2 == 0. || _y2 == 0.)	return 0.f;

	float _z = sqrt(m_fRadius * m_fRadius - _x * _x - _y * _y);

	float theta = RAD2DEG(acos(_z / m_fRadius));
	float phi   = XY2Phi(_x, _y, true);

	float epsilon = 0.f;

	float min_d = 1E6;

	// draw equator and points
	for (int d = 0; d < 360; d++)
	{
		float x0 = 0.f, y0 = 0.f, z0 = 0.f, x1 = 0.f, y1 = 0.f, z1 = 0.f;

		rotate(0, 0, m_fRadius, theta - 90, (float)d, phi, x0, y0, z0);
			
		if (z0 >= 0)	// calculate only visible point
		{
			float dist = sqrt(pow(_x2 - x0, 2.) + pow(_y2 - y0, 2.));

			if (dist < min_d)
			{
				epsilon = d;
				min_d = dist;
			}
		}
	}

	return epsilon;
}

bool CPSSpinImage::SetPosition(float _x, float _y, float _epsilon)
{
	if (m_fRadius == 0.f)	return false;

	if (m_fRadius * m_fRadius < _x * _x + _y * _y)	return false;

	m_fX = _x;
	m_fY = _y;
	m_fZ = sqrt(m_fRadius * m_fRadius - _x * _x - _y * _y);

	m_fTheta = RAD2DEG(acos(m_fZ / m_fRadius));
	m_fPhi = XY2Phi(_x, _y, true);
	m_fEpsilon = _epsilon;

	return true;
}

void CPSSpinImage::rotate(float _x0, float _y0, float _z0, float theta, float phi, float epsilon, float& _x, float& _y, float& _z)
{
	float _t = DEG2RAD(theta);
	float _p = DEG2RAD(phi);
	float _e = DEG2RAD(epsilon);

	_x = _x0 * cos(_t) * cos(_e) - _y0 * (cos(_p) * sin(_e) - sin(_p) * sin(_t) * cos(_e)) + _z0 * (sin(_p) * sin(_e) + cos(_p) * sin(_t) * cos(_e));
	_y = _x0 * cos(_t) * sin(_e) + _y0 * (cos(_p) * cos(_e) + sin(_p) * sin(_t) * sin(_e)) - _z0 * (sin(_p) * cos(_e) - cos(_p) * sin(_t) * sin(_e));
	_z = _x0 * (-sin(_t)) + _y0 * sin(_p) * cos(_t) + _z0 * cos(_p) * cos(_t);
}

cv::Mat CPSSpinImage::generate(cv::Mat source)
{
	int marker_size = 24;

	// set background;
	if (source.rows != 0)
		cv::resize(source, m_image, m_image.size());
	else
		m_image.setTo(cv::Scalar(255, 255, 255));

	cv::line(m_image, cv::Point(0, m_image.rows / 2.f), cv::Point(m_image.cols, m_image.rows / 2.f), CV_RGB(224, 224, 224), 1);
	cv::line(m_image, cv::Point(m_image.cols / 2.f, 0), cv::Point(m_image.cols / 2.f, m_image.rows), CV_RGB(224, 224, 224), 1);

	if (m_fDiameter != 0.f)
	{
		float ratio = (m_image.cols - RECT_BORDER * 2) / m_fDiameter;

		cv::Point2f cp = cv::Point2f(m_image.cols / 2.f, m_image.cols / 2.f);
		
		cv::circle(m_image, cp, 2, CV_RGB(64, 64, 64), -1);

		// draw candidate line
		float _x = 0.f, _y = 0.f, _z = 0.f;

		// opposite position
		rotate(m_fX, m_fY, m_fZ, 0, 0, 180, _x, _y, _z);

		// draw
		cv::Point2f pt = cv::Point2f(cp.x + _x * ratio, cp.y - _y * ratio);
		cv::line(m_image, cp, pt, CV_RGB(255, 192, 192), 1);
		cv::circle(m_image, pt, marker_size, CV_RGB(255, 0, 0), 1);

		// draw equator and points
		for (int d = 0; d < 360; d++)
		{
			float x0 = 0.f, y0 = 0.f, z0 = 0.f, x1 = 0.f, y1 = 0.f, z1 = 0.f;

			rotate(0, 0, m_fRadius, m_fTheta - 90, (float)d, m_fPhi, x0, y0, z0);
			rotate(0, 0, m_fRadius, m_fTheta - 90, (float)d+1, m_fPhi, x1, y1, z1);

			cv::line(m_image, cv::Point2f(cp.x + x0 * ratio, cp.y - y0 * ratio), cv::Point2f(cp.x + x1 * ratio, cp.y - y1 * ratio), z0 >= 0 ? CV_RGB(64, 64, 192) : CV_RGB(192, 192, 192));
		}

		// draw 4 balls
		rotate(0, 0, m_fRadius, m_fTheta - 90, m_fEpsilon, m_fPhi, _x, _y, _z);
		pt = cv::Point2f(cp.x + _x * ratio, cp.y - _y * ratio);
		cv::line(m_image, cp, pt, CV_RGB(128, 0, 0), 1);
		cv::circle(m_image, pt, marker_size, CV_RGB(255, 128, 128), _z >= 0 ? -1 : 1);

		rotate(0, 0, m_fRadius, m_fTheta - 90, m_fEpsilon + 90, m_fPhi, _x, _y, _z);
		pt = cv::Point2f(cp.x + _x * ratio, cp.y - _y * ratio);
		cv::line(m_image, cp, pt, CV_RGB(192, 192, 192), 1);
		cv::circle(m_image, pt, marker_size, CV_RGB(255, 128, 128), _z >= 0 ? -1 : 1);

		rotate(0, 0, m_fRadius, m_fTheta - 90, m_fEpsilon + 180, m_fPhi, _x, _y, _z);
		pt = cv::Point2f(cp.x + _x * ratio, cp.y - _y * ratio);
		cv::circle(m_image, pt, marker_size, CV_RGB(255, 128, 128), _z >= 0 ? -1 : 1);

		rotate(0, 0, m_fRadius, m_fTheta - 90, m_fEpsilon + 270, m_fPhi, _x, _y, _z);
		pt = cv::Point2f(cp.x + _x * ratio, cp.y - _y * ratio);
		cv::line(m_image, cp, pt, CV_RGB(192, 192, 192), 1);
		cv::circle(m_image, pt, marker_size, CV_RGB(255, 128, 128), _z >= 0 ? -1 : 1);

		// draw main point
		pt = cv::Point2f(cp.x + m_fX * ratio, cp.y - m_fY * ratio);
		cv::line(m_image, cp, pt, CV_RGB(255, 192, 192), 1);
		cv::circle(m_image, pt, marker_size, CV_RGB(255, 0, 0), -1);
		
		// draw out-line
		cv::circle(m_image, cp, m_fRadius * ratio + marker_size / 2, CV_RGB(255, 255, 255), marker_size);
		cv::circle(m_image, cp, m_fRadius * ratio, CV_RGB(128, 128, 128), 2);
	}

	return m_image;
}
