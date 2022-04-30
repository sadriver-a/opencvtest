// base.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include <iostream>
#include "opencv2\core.hpp"
#include "opencv2\imgcodecs.hpp"
#include "opencv2\highgui.hpp" 
#include "opencv2\imgproc\imgproc.hpp"
#include "Pretreatment.h"
#include <exception>

using namespace cv;
using namespace std;



int main()
{
	Pretreatment *pImgPretreat = nullptr;
	try {
		pImgPretreat = new Pretreatment("P:/ahs/opencv/test_img/2.jpg");
		pImgPretreat->PrintImgInfo();
	}
	catch (std::exception e)
	{
		cout << "Pretreatment init error: " << e.what() << endl;
		return -1;
	}
	pImgPretreat->ShowOriginalImg();
	/*pImgPretreat->PretreatmentImage();*/
	pImgPretreat->PrespectiveTransform();

	waitKey(0);

	return 0;
}