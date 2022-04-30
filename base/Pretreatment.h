#pragma once
#include "opencv2\core.hpp"
#include "opencv2\imgcodecs.hpp"
#include "opencv2\highgui.hpp" 
#include "opencv2\imgproc\imgproc.hpp"
#include <string>

//对漫画做预处理
class Pretreatment
{
private:
	std::string    m_imgPath;
	cv::Mat        m_origImg;
	cv::Mat        m_grayImg;

	//提高对比度
	bool ImproveContrast(cv::Mat &inputImg, cv::Mat &outputImg);

	//加黑色边框，确保边缘对话框可以被识别
	bool AddBorder(cv::Mat &img);
	
	void ContoursFilter(std::vector<std::vector<cv::Point>> &contours, std::vector<bool> &vt_isFilter);

	//锐化
	void Sharpen(cv::Mat &img);
public:
	explicit Pretreatment(std::string imgPath);
	~Pretreatment();

	void ShowOriginalImg();

	int PretreatmentImage();

	void PrintImgInfo();

	bool PrespectiveTransform();
};

