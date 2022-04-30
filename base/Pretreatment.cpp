#include "Pretreatment.h"
#include <iostream>
#include <direct.h>
#include <stdio.h>
#include <exception>
#include <vector>
#include <io.h>
using namespace std;

Pretreatment::Pretreatment(std::string imgPath):m_imgPath(imgPath)
{
	if (_access(imgPath.c_str(), 0) == -1)
	{
		throw(std::exception((string("\nerror path") + imgPath).c_str()));
	}
	m_origImg = cv::imread(m_imgPath.c_str());
	if (!m_origImg.data)
	{
		throw(std::exception("can't open image"));
	}
}


Pretreatment::~Pretreatment()
{
}


void Pretreatment::PrintImgInfo()
{
	std::cout << "image path: " << m_imgPath << endl;
	std::cout << "width: " << m_origImg.cols << "\theigh: " << m_origImg.rows << endl;
}



//提高对比度，这里采用直方图均衡化，这里效果不好
bool Pretreatment::ImproveContrast(cv::Mat &inputImg, cv::Mat &outputImg)
{
	cv::equalizeHist(inputImg, outputImg);
	return true;
}



//加黑色边框，确保边缘对话框可以被识别
//特殊情况过滤
bool Pretreatment::AddBorder(cv::Mat &img)
{
	//增加黑色边缘，加粗直线（用霍夫变换检测直线）
	cv::copyMakeBorder(img, img, 10, 10, 10, 10, cv::BORDER_CONSTANT);
	return true;
}

//轮廓过滤
//原则：包含区域最小面积
void Pretreatment::ContoursFilter(vector<vector<cv::Point>> &contours, vector<bool> &vt_isFilter)
{
	vector <cv::Rect> vt_outRect; //外接矩形
	for (size_t i = 0; i < contours.size(); ++i)
	{
		bool bTmpFlg = false;
		int min_x = m_origImg.cols;
		int min_y = m_origImg.rows;
		int max_x = 0;
		int max_y = 0;
		for (size_t j = 0; j < contours[i].size(); ++j)
		{
			if (max_x < contours[i][j].x)
				max_x = contours[i][j].x;
			if (min_x > contours[i][j].x)
				min_x = contours[i][j].x;
			if (max_y < contours[i][j].y)
				max_y = contours[i][j].y;
			if (min_y > contours[i][j].y)
				min_y = contours[i][j].y;
		}
		//记录外接矩形框
		vt_outRect.push_back(cv::Rect(cv::Point(min_x, min_y), cv::Point(max_x, max_y)));

		//本身外接矩阵的面积阈值小于40*40(最小字符的大小)
		if (((max_y - min_y) > 30 ) && ((max_x - min_x) > 30))
		{ 
			//本身外接矩阵的面积阈值小于整个面积的XX%
			if (((double)(max_y - min_y) < 0.95 * m_origImg.rows) &&
				((double)(max_x - min_x) < 0.95 * m_origImg.cols)
				)
				bTmpFlg = true;
		}
		vt_isFilter.push_back(bTmpFlg);
	}

	//判断有效的外接矩形框
	for (size_t i = 0; i < contours.size(); ++i)
	{
		if (!vt_isFilter[i])
			continue;

		//判断外接矩形内有其他的外接矩形进行下一步
		cv::Rect outRect = vt_outRect[i]; //获得当前的外接矩形
		bool isContain = false;
		for (size_t j = 0; j < contours.size(); ++j)
		{
			if (i == j)
				continue;
			cv::Rect tmpRect = vt_outRect[j]; //待比较的外接矩形
			if (tmpRect.width < 30 && tmpRect.height < 30) //内部较小面积过滤
				continue;
			int tmp_x1 = tmpRect.x;
			int tmp_y1 = tmpRect.y;
			int tmp_x2 = tmpRect.x + tmpRect.width;
			int tmp_y2 = tmpRect.y + tmpRect.height;
			int out_x1 = outRect.x;
			int out_y1 = outRect.y;
			int out_x2 = outRect.x + outRect.width;
			int out_y2 = outRect.y + outRect.height;
			if ((out_x1 < tmp_x1) && (out_y1 < tmp_y1) &&
				(out_x2 > tmp_x2) && (out_y2 > tmp_y2))
			{		
				//如果是对话框，带比较的外接矩形一定会在轮廓范围内
				if ((cv::pointPolygonTest(contours[i], cv::Point(tmp_x1, tmp_y1), false) > 0) &&
					(cv::pointPolygonTest(contours[i], cv::Point(tmp_x2, tmp_y2), false) > 0) &&
					(cv::pointPolygonTest(contours[i], cv::Point(tmp_x1, tmp_y2), false) > 0) &&
					(cv::pointPolygonTest(contours[i], cv::Point(tmp_x2, tmp_y1), false) > 0)
					)
				{
					isContain = true;
					break;
				}	
			}
		}
		if (!isContain)
		{
			vt_isFilter[i] = false;
		}
	}

}


//锐化
void Pretreatment::Sharpen(cv::Mat &img)
{
	cv::Mat kernel = (cv::Mat_<float>(3, 3) << 1, 1, 1, 1, -8, 1, 1, 1, 1);
	cv::Mat imgLaplance;
	cv::Mat origImg;
	cv::filter2D(img, imgLaplance, CV_32F, kernel);
	img.convertTo(origImg, CV_32F);
	cv::Mat resultImg = origImg - imgLaplance;

	resultImg.convertTo(resultImg, CV_8UC3);
	img = resultImg;
}

int Pretreatment::PretreatmentImage()
{
	//Sharpen(m_origImg);
	AddBorder(m_origImg);

	//转化为
	cv::cvtColor(m_origImg, m_grayImg, cv::COLOR_BGR2GRAY);
	cv::namedWindow("灰度图", CV_WINDOW_AUTOSIZE);
	imshow("灰度图", m_grayImg);

	//滤波,高斯模糊
	//cv::blur(m_grayImg, m_grayImg, cv::Size(3, 3));      //均值滤波
	//cv::namedWindow("模糊", CV_WINDOW_AUTOSIZE);
	//imshow("模糊", m_grayImg);

	//ImproveContrast(m_grayImg, m_grayImg); //直方图均衡化之后的效果不理想


	//canny边缘检测//二值化，只需要取区间二值化
	cv::Mat edgeImg;
	cv::Canny(m_grayImg, edgeImg, 120, 200, 3, false);     //关于阈值的取值策略
	cv::namedWindow("canny", CV_WINDOW_AUTOSIZE);
	imshow("canny", edgeImg);

	//cv::Mat edgeImg1;
	//cv::Canny(m_grayImg, edgeImg1, 170, 220, 3, false);     //关于阈值的取值策略
	//cv::namedWindow("canny1", CV_WINDOW_AUTOSIZE);
	//imshow("canny1", edgeImg1);


	//形态学操作，过滤掉一些过于接近的点和线条，闭操作
	//腐蚀，膨胀，开运算（OPEN），闭运算（CLOSE），形态学梯度（膨胀减去腐蚀），顶帽（原图像与开操作之间的差值）
//这里注意字体被被腐蚀膨胀了没有关系，需要将黑色部分扩大，这样可以减少黑色区域
	//这里闭运算和形态学梯度
	cv::Mat dst;
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(9,9));
	cv::morphologyEx(edgeImg, dst, CV_MOP_CLOSE, kernel);
	cv::namedWindow("闭操作", CV_WINDOW_AUTOSIZE);
	imshow("闭操作", dst);
	//cv::morphologyEx(dst, dst, CV_MOP_OPEN, kernel); //会过滤掉
	//cv::namedWindow("开操作", CV_WINDOW_AUTOSIZE);
	//imshow("开操作", dst);

	//取轮廓
	vector<vector<cv::Point>> contours;
	vector<cv::Vec4i> hierarchy;
	cv::findContours(dst, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);


	//过滤轮廓
	std::vector<bool> vt_isFilter;
	ContoursFilter(contours, vt_isFilter);


	cv::RNG rng;
	cv::Mat drawImg = cv::Mat::zeros(dst.size(), CV_8UC3);
	for (size_t i = 0; i < contours.size(); ++i)
	{
		if (!vt_isFilter[i])
			continue;
		cv::Scalar color = cv::Scalar(rng.uniform(0,255), rng.uniform(0, 255), rng.uniform(0, 255));
		cv::drawContours(drawImg, contours, i, color, 1,
			cv::LINE_8, hierarchy, 0);
		//cv::drawContours(drawImg, contours, 134, cv::Scalar(256,255,255), 1,
		//	cv::LINE_8, hierarchy, 0);
	}
	cv::namedWindow("轮廓", CV_WINDOW_AUTOSIZE);
	imshow("轮廓", drawImg);
	return 0;
}


void Pretreatment::ShowOriginalImg() 
{
	cv::namedWindow("原始图像", CV_WINDOW_AUTOSIZE);
	imshow("原始图像", m_origImg);
}


bool Pretreatment::PrespectiveTransform()
{
	const cv::Point2f ps1[] = { {130,150}, {600,32}, {200,300},{520,400} ,{220,100},{420,100} };
	const cv::Point2f ps2[] = { {130,250}, {600,50}, {200,300},{520,400}, {220,100},{420,100} };
	cv::Mat M = cv::getPerspectiveTransform(ps1, ps2);
	cout << M << endl;
	cv::Mat        outImg;
	cv::warpPerspective(m_origImg, outImg, M, cv::Size(m_origImg.cols, m_origImg.rows));
	cv::namedWindow("投影变换", CV_WINDOW_AUTOSIZE);
	cv::imshow("投影变换", outImg);
	return true;
}

int Pretreatment::PretreatmentImage1()
{

	return 0;
}