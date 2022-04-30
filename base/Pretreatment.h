#pragma once
#include "opencv2\core.hpp"
#include "opencv2\imgcodecs.hpp"
#include "opencv2\highgui.hpp" 
#include "opencv2\imgproc\imgproc.hpp"
#include <string>

//��������Ԥ����
class Pretreatment
{
private:
	std::string    m_imgPath;
	cv::Mat        m_origImg;
	cv::Mat        m_grayImg;

	//��߶Աȶ�
	bool ImproveContrast(cv::Mat &inputImg, cv::Mat &outputImg);

	//�Ӻ�ɫ�߿�ȷ����Ե�Ի�����Ա�ʶ��
	bool AddBorder(cv::Mat &img);
	
	void ContoursFilter(std::vector<std::vector<cv::Point>> &contours, std::vector<bool> &vt_isFilter);

	//��
	void Sharpen(cv::Mat &img);
public:
	explicit Pretreatment(std::string imgPath);
	~Pretreatment();

	void ShowOriginalImg();

	int PretreatmentImage();

	void PrintImgInfo();

	bool PrespectiveTransform();
};

