// Mosaic.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "vector"
#include "math.h"


#include<opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>  
#include<iostream>


using namespace std;
using namespace cv;

//设置模板大小为3
#define N 3

//两幅图大地坐标
int Up_L_x1 = 490868, Up_L_x2 = 495731;
int Up_L_y1 = 3410518, Up_L_y2 = 3410798;
int SLOW_R_x1 = 499448, SLOW_R_x2 = 504311;
int SLOW_R_y1 = 3395098, SLOW_R_y2 = 3395348;

//重叠区范围和坐标大小	
//int width = (Up_L_x2 - SLOW_R_x1) / 30;
//int height = (Up_L_y1 - SLOW_R_y2 / 30;

#define width 124	//重叠区大小
#define height 506	
#define detax 162
#define detay1 9
#define detay2 8

//绘制除重叠区外所有部分
void GetBlank(Mat M1, Mat M2, Mat M3) {

	int m = 0, n = 0;

	//Area_1
	for (int i = 0; i < detay1; i++) {
		for (int j = 0; j<detax; j++) {
			M3.at<Vec3b>(i, j) = 8;
		}
	}
	//Area_2、Area_3
	for (int i = 0; i < detay1; i++, m++) {
		for (int j = detax; j < detax+width+detax; j++) {
			M3.at<Vec3b>(i, j) = M2.at<Vec3b>(m, n);
			n++;
		}
		n = 0;
	}
	m = 0; n = 0;

	//Area_4、Area_7
	for (int i = detay1; i < detay1+detay2+height; i++, m++) {
		for (int j = 0; j < detax; j++) {
			M3.at<Vec3b>(i, j) = M1.at<Vec3b>(m, n);
			n++;
		}
		n = 0;
	}
	m = 0; n = 0;
	
	//Area_5
	for (int i = detay1; i < detay1+height; i++) {
		for (int j = detax; j < detax+width; j++) {
			M3.at<Vec3b>(i, j) = 255;
		}
	}

	//Area_6
	m = detay1; n = width;
	for (int i = detay1; i < detay1 + height; i++, m++) {
		for (int j = detax+width-1; j < detax+width+detax; j++) {
			M3.at<Vec3b>(i, j) = M2.at<Vec3b>(m, n);
			n++;
		}
		n = width;
	}
	m = 0; n = 0;
	
	//Area_8
	m = height; n = detax;
	for (int i = height+detay1; i < detay1+height+detay2; i++, m++) {
		for (int j = detax; j < detax+width; j++) {
			M3.at<Vec3b>(i, j) = M1.at<Vec3b>(m, n);
			n++;
		}
		n = detax;
	}
	m = 0; n = 0;
	
	//Area_9
	for (int i = detay1+height; i < detay1 + height+detay2; i++) {
		for (int j = detax+width; j < detax + width+detax; j++) {
			M3.at<Vec3b>(i, j) = 8;
		}
	}
	

}

//获取最小值
int GetMin(int *m,int n) {
	int Min = 100000;
	int k = 0;

	for (int i = 0; i < n; i++) {
		if (m[i] < Min) {
			Min = m[i];
			k = i;
		}
	}
	return k;
}

void GetBestSide(Mat M1, Mat M2, int avalue[height]) {

	int detaIK = 0;

	//以3为模板，则这一行一共能得到122个待镶嵌点。
	int m[122];
	int p = 0, q = 0;

	//模板移动寻求待镶嵌点
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width - 2; j++) {
			//模板大小为3
			for (int k = -1; k < 2; k++) {
				detaIK = abs(M1.at<Vec3b>(i, j + detax + 1 + k)[0] - M2.at<Vec3b>(i + detay1, j + 1 + k)[0])
					+ abs(M1.at<Vec3b>(i, j + detax + 1 + k)[1] - M2.at<Vec3b>(i + detay1, j + 1 + k)[1])
					+ abs(M1.at<Vec3b>(i, j + detax + 1 + k)[2] - M2.at<Vec3b>(i + detay1, j + 1 + k)[2]);
				m[j] += detaIK;
			}

		}
		//将最佳镶嵌边列号赋值于avalue
		avalue[i] = GetMin(m, 122);

		for (int j = 0; j < 122; j++) {
			m[j] = 0;
		}
	}
}

void AverageBrightness(Mat M1, Mat M2, int avalue[height]) {
	int LAVE1 = 0, LAVE2 = 0, LAVE3 = 0;
	int RAVE1 = 0, RAVE2 = 0, RAVE3 = 0;

	for (int i = 0; i < height; i++) {
		LAVE1 += M1.at<Vec3b>(i, detax + 1 + avalue[i])[0];
		LAVE2 += M1.at<Vec3b>(i, detax + 1 + avalue[i])[1];
		LAVE3 += M1.at<Vec3b>(i, detax + 1 + avalue[i])[2];
		RAVE1 += M2.at<Vec3b>(i + detay1, 1 + avalue[i])[0];
		RAVE2 += M2.at<Vec3b>(i + detay1, 1 + avalue[i])[1];
		RAVE3 += M2.at<Vec3b>(i + detay1, 1 + avalue[i])[2];
	}

	LAVE1 = LAVE1 / height;
	LAVE2 = LAVE2 / height;
	LAVE3 = LAVE3 / height;
	RAVE1 = RAVE1 / height;
	RAVE2 = RAVE2 / height;
	RAVE3 = RAVE3 / height;
	
	for (int i = 0; i < detay1+height; i++) {
		for (int j = 0; j< detax+width; j++) {
			M2.at<Vec3b>(i, j)[0] += -LAVE1 + RAVE1;
			M2.at<Vec3b>(i, j)[1] += -LAVE2 + RAVE2;
			M2.at<Vec3b>(i, j)[2] += -LAVE3 + RAVE3;
		}
	}

	double lMin1 = 0, lMin2 = 0, lMin3 = 0, lmin1 = 500, lmin2 = 500, lmin3 = 500;
	double rMin1 = 0, rMin2 = 0, rMin3 = 0, rmin1 = 500, rmin2 = 500, rmin3 = 500;
	double fA1 = 0, fB1 = 0, fA2 = 0, fB2 = 0, fA3 = 0, fB3 = 0;

	for (int i = 0; i < height; i++) {
		if (M1.at<Vec3b>(i, detax + 1 + avalue[i])[0] > lMin1)
			lMin1 = M1.at<Vec3b>(i, detax + 1 + avalue[i])[0];
		if (M1.at<Vec3b>(i, detax + 1 + avalue[i])[1] > lMin2)
			lMin2 = M1.at<Vec3b>(i, detax + 1 + avalue[i])[1];
		if (M1.at<Vec3b>(i, detax + 1 + avalue[i])[2] > lMin3)
			lMin3 = M1.at<Vec3b>(i, detax + 1 + avalue[i])[2];

		if (M2.at<Vec3b>(i + detay1, 1 + avalue[i])[0] > rMin1)
			rMin1 = M2.at<Vec3b>(i + detay1, 1 + avalue[i])[0];
		if (M2.at<Vec3b>(i + detay1, 1 + avalue[i])[1] > rMin2)
			rMin2 = M2.at<Vec3b>(i + detay1, 1 + avalue[i])[1];
		if (M2.at<Vec3b>(i + detay1, 1 + avalue[i])[2] > rMin3)
			rMin3 = M2.at<Vec3b>(i + detay1, 1 + avalue[i])[2];

		if (M1.at<Vec3b>(i, detax + 1 + avalue[i])[0] < lmin1)
			lmin1 = M1.at<Vec3b>(i, detax + 1 + avalue[i])[0];
		if (M1.at<Vec3b>(i, detax + 1 + avalue[i])[1] < lmin2)
			lmin2 = M1.at<Vec3b>(i, detax + 1 + avalue[i])[1];
		if (M1.at<Vec3b>(i, detax + 1 + avalue[i])[2] < lmin3)
			lmin3 = M1.at<Vec3b>(i, detax + 1 + avalue[i])[2];

		if (M2.at<Vec3b>(i, detax + 1 + avalue[i])[0] < rmin1)
			rmin1 = M2.at<Vec3b>(i, detax + 1 + avalue[i])[0];
		if (M2.at<Vec3b>(i, detax + 1 + avalue[i])[1] < rmin2)
			rmin2 = M2.at<Vec3b>(i, detax + 1 + avalue[i])[1];
		if (M2.at<Vec3b>(i, detax + 1 + avalue[i])[2] < rmin3)
			rmin3 = M2.at<Vec3b>(i, detax + 1 + avalue[i])[2];
	}

	fA1 = (lMin1 - lmin1) / (rMin1 - rmin1);
	fB1 = -fA1*rmin1 + lmin1;
	
	fA2 = (lMin2 - lmin2) / (rMin2 - rmin2);
	fB2 = -fA2*rmin2 + lmin2;

	fA3 = (lMin3 - lmin3) / (rMin3 - rmin3);
	fB3 = -fA3*rmin3 + lmin3;

	//printf("\n\n%.2lf,%.2lf\n%.2lf,%.2lf\n%.2lf,%.2lf"
	//	, fA1, fB1, fA2, fB2, fA3, fB3);

	//对右图进行修改
	for (int i = 0; i < detay1+height; i++) {
		for (int j = 0; j< detax+width; j++) {
			M2.at<Vec3b>(i, j)[0] = fA1*M2.at<Vec3b>(i, j)[0] + fB1;
			M2.at<Vec3b>(i, j)[1] = fA2*M2.at<Vec3b>(i, j)[1] + fB2;
			M2.at<Vec3b>(i, j)[2] = fA3*M2.at<Vec3b>(i, j)[2] + fB3;
		}
	}
}

//在M3中绘制重叠区部分
void Mosaic(Mat M1, Mat M2, Mat M3, int avalue[height]) {
	for (int i = 0; i < height; i++) {
		M3.at<Vec3b>(i + detay1, detax) = M1.at<Vec3b>(i, detax);
		M3.at<Vec3b>(i + detay1, detax+width) = M2.at<Vec3b>(i+ detay1, width);

		for (int j = 0; j < width-2; j++) {
			if (j < avalue[i]) {
				M3.at<Vec3b>(i + detay1, detax + 1 + j) = M1.at<Vec3b>(i, j + detax + 1);
			}
			else {
				M3.at<Vec3b>(i + detay1, detax + 1 + j) = M2.at<Vec3b>(i + detay1, j + 1);
			}
		}
	}
}

//拼接边缘平滑
void Smoothen(Mat M1, Mat M2, Mat M3, int avalue[height]) {

	int n = 5;
	int s = 2 * n - 1;
	int k = 0;
	int p1, p2;
	for (int i = 0; i < height; i++) {
		k = avalue[i];
		for (int j = 0; j < detax + width + detax; j++) {

			p1 = (k - j + (s + 1) / 2) / (s + 1);
			p2 = ((s + 1) / 2 - k + j) / (s + 1);
			if ((j >= k - (s - 1) / 2) && (j <= k + (s - 1) / 2)) {
				M3.at<Vec3b>(i + detay1, j) = M1.at<Vec3b>(i, j)*p1 + M2.at<Vec3b>(i + detay1, j);
			}
		}
	}
}


int main()
{
	char imageName1[] = "leftliner.bmp";
	Mat M1 = imread(imageName1, IMREAD_COLOR);   // 读入图片 
	char imageName2[] = "rightliner.bmp";
	Mat M2 = imread(imageName2, IMREAD_COLOR);   // 读入图片 

	if (M1.empty()|| M2.empty())     // 判断文件是否正常打开  
	{
		fprintf(stderr, "Can not load image %s\n", imageName1);
		waitKey(6000);  // 等待6000 ms后窗口自动关闭   
		return -1;
	}

	//合成图像的长宽
	int w = detax + width + detax;
	int h = detay1 + height + detay2;
	Mat M3(h,w,CV_8UC3, Scalar(100,100,100));


	int avalue[height];//镶嵌边

	GetBlank(M1, M2, M3);


	GetBestSide(M1, M2, avalue);		//获取最佳镶嵌边

	AverageBrightness(M1, M2, avalue);	//亮度平均和拉伸

	Mosaic(M1, M2, M3, avalue);			//裁剪

	Smoothen(M1, M2, M3, avalue);		//拼接边平滑


	namedWindow("Mosaic",0);
	imshow("Mosaic", M3);

	printf("\n\n");

	waitKey();
    return 0;
}

