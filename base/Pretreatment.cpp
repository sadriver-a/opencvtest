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



//��߶Աȶȣ��������ֱ��ͼ���⻯������Ч������
bool Pretreatment::ImproveContrast(cv::Mat &inputImg, cv::Mat &outputImg)
{
	cv::equalizeHist(inputImg, outputImg);
	return true;
}



//�Ӻ�ɫ�߿�ȷ����Ե�Ի�����Ա�ʶ��
//�����������
bool Pretreatment::AddBorder(cv::Mat &img)
{
	//���Ӻ�ɫ��Ե���Ӵ�ֱ�ߣ��û���任���ֱ�ߣ�
	cv::copyMakeBorder(img, img, 10, 10, 10, 10, cv::BORDER_CONSTANT);
	return true;
}

//��������
//ԭ�򣺰���������С���
void Pretreatment::ContoursFilter(vector<vector<cv::Point>> &contours, vector<bool> &vt_isFilter)
{
	vector <cv::Rect> vt_outRect; //��Ӿ���
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
		//��¼��Ӿ��ο�
		vt_outRect.push_back(cv::Rect(cv::Point(min_x, min_y), cv::Point(max_x, max_y)));

		//������Ӿ���������ֵС��40*40(��С�ַ��Ĵ�С)
		if (((max_y - min_y) > 30 ) && ((max_x - min_x) > 30))
		{ 
			//������Ӿ���������ֵС�����������XX%
			if (((double)(max_y - min_y) < 0.95 * m_origImg.rows) &&
				((double)(max_x - min_x) < 0.95 * m_origImg.cols)
				)
				bTmpFlg = true;
		}
		vt_isFilter.push_back(bTmpFlg);
	}

	//�ж���Ч����Ӿ��ο�
	for (size_t i = 0; i < contours.size(); ++i)
	{
		if (!vt_isFilter[i])
			continue;

		//�ж���Ӿ���������������Ӿ��ν�����һ��
		cv::Rect outRect = vt_outRect[i]; //��õ�ǰ����Ӿ���
		bool isContain = false;
		for (size_t j = 0; j < contours.size(); ++j)
		{
			if (i == j)
				continue;
			cv::Rect tmpRect = vt_outRect[j]; //���Ƚϵ���Ӿ���
			if (tmpRect.width < 30 && tmpRect.height < 30) //�ڲ���С�������
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
				//����ǶԻ��򣬴��Ƚϵ���Ӿ���һ������������Χ��
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


//��
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

	//ת��Ϊ
	cv::cvtColor(m_origImg, m_grayImg, cv::COLOR_BGR2GRAY);
	cv::namedWindow("�Ҷ�ͼ", CV_WINDOW_AUTOSIZE);
	imshow("�Ҷ�ͼ", m_grayImg);

	//�˲�,��˹ģ��
	//cv::blur(m_grayImg, m_grayImg, cv::Size(3, 3));      //��ֵ�˲�
	//cv::namedWindow("ģ��", CV_WINDOW_AUTOSIZE);
	//imshow("ģ��", m_grayImg);

	//ImproveContrast(m_grayImg, m_grayImg); //ֱ��ͼ���⻯֮���Ч��������


	//canny��Ե���//��ֵ����ֻ��Ҫȡ�����ֵ��
	cv::Mat edgeImg;
	cv::Canny(m_grayImg, edgeImg, 120, 200, 3, false);     //������ֵ��ȡֵ����
	cv::namedWindow("canny", CV_WINDOW_AUTOSIZE);
	imshow("canny", edgeImg);

	//cv::Mat edgeImg1;
	//cv::Canny(m_grayImg, edgeImg1, 170, 220, 3, false);     //������ֵ��ȡֵ����
	//cv::namedWindow("canny1", CV_WINDOW_AUTOSIZE);
	//imshow("canny1", edgeImg1);


	//��̬ѧ���������˵�һЩ���ڽӽ��ĵ���������ղ���
	//��ʴ�����ͣ������㣨OPEN���������㣨CLOSE������̬ѧ�ݶȣ����ͼ�ȥ��ʴ������ñ��ԭͼ���뿪����֮��Ĳ�ֵ��
//����ע�����屻����ʴ������û�й�ϵ����Ҫ����ɫ���������������Լ��ٺ�ɫ����
	//������������̬ѧ�ݶ�
	cv::Mat dst;
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(9,9));
	cv::morphologyEx(edgeImg, dst, CV_MOP_CLOSE, kernel);
	cv::namedWindow("�ղ���", CV_WINDOW_AUTOSIZE);
	imshow("�ղ���", dst);
	//cv::morphologyEx(dst, dst, CV_MOP_OPEN, kernel); //����˵�
	//cv::namedWindow("������", CV_WINDOW_AUTOSIZE);
	//imshow("������", dst);

	//ȡ����
	vector<vector<cv::Point>> contours;
	vector<cv::Vec4i> hierarchy;
	cv::findContours(dst, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);


	//��������
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
	cv::namedWindow("����", CV_WINDOW_AUTOSIZE);
	imshow("����", drawImg);
	return 0;
}


void Pretreatment::ShowOriginalImg() 
{
	cv::namedWindow("ԭʼͼ��", CV_WINDOW_AUTOSIZE);
	imshow("ԭʼͼ��", m_origImg);
}


bool Pretreatment::PrespectiveTransform()
{
	const cv::Point2f ps1[] = { {130,150}, {600,32}, {200,300},{520,400} ,{220,100},{420,100} };
	const cv::Point2f ps2[] = { {130,250}, {600,50}, {200,300},{520,400}, {220,100},{420,100} };
	cv::Mat M = cv::getPerspectiveTransform(ps1, ps2);
	cout << M << endl;
	cv::Mat        outImg;
	cv::warpPerspective(m_origImg, outImg, M, cv::Size(m_origImg.cols, m_origImg.rows));
	cv::namedWindow("ͶӰ�任", CV_WINDOW_AUTOSIZE);
	cv::imshow("ͶӰ�任", outImg);
	return true;
}

int Pretreatment::PretreatmentImage1()
{

	return 0;
}