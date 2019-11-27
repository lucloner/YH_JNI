#define _CRT_SECURE_NO_WARNINGS
#include"com_BlankPageDetectDLL.h"
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc_c.h"
#include <iostream>
#include <fcntl.h>
#include <io.h>
#include <math.h>
#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <string.h>


//#pragma warning(disable:4113)
//
//jmp_buf mark;
//int fperr;
//void __cdecl  fphandler(int num);
//void fpcheck(void);


using namespace cv;
using namespace std;

//class CException
//{
//public:
//	string msg;
//	CException(string s) : msg(s) {}
//};
//
//int checkExc(JNIEnv *env) {
//	if (env->ExceptionCheck()) {
//		env->ExceptionDescribe(); // writes to logcat
//		env->ExceptionClear();
//		return 1;
//	}
//	return -1;
//}
//
//void JNU_ThrowByName(JNIEnv *env, const char *name, const char *msg)
//{
//	// 查找异常类
//	jclass cls = env->FindClass(name);
//	/* 如果这个异常类没有找到，VM会抛出一个NowClassDefFoundError异常 */
//	if (cls != NULL) {
//		env->ThrowNew(cls, msg);  // 抛出指定名字的异常
//	}
//	/* 释放局部引用 */
//	env->DeleteLocalRef(cls);
//}

bool SetResolution(const char* path, int iResolution)
{
	FILE * file = fopen(path, "rb+");// - 打开图片文件
	if (!file)return false;
	int len = _filelength(_fileno(file));// - 获取文件大小
	char* buf = new char[len];
	fread(buf, sizeof(char), len, file);// - 将图片数据读入缓存
	char * pResolution = (char*)&iResolution;// - iResolution为要设置的分辨率的数值，如72dpi
											 // - 设置JPG图片的分辨率
	buf[0x0D] = 1;// - 设置使用图片密度单位
				  // - 水平密度，水平分辨率
	buf[0x0E] = pResolution[1];
	buf[0x0F] = pResolution[0];
	// - 垂直密度，垂直分辨率
	buf[0x10] = pResolution[1];
	buf[0x11] = pResolution[0];

	// - 将文件指针移动回到文件开头
	fseek(file, 0, SEEK_SET);
	// - 将数据写入文件，覆盖原始的数据，让修改生效
	fwrite(buf, sizeof(char), len, file);
	fclose(file);
	return true;
}


// 仿照matlab，自适应求高低两个门限
void _AdaptiveFindThreshold(Mat *dx, Mat *dy, double *low, double *high)
{
    IplImage *imge;
    CvHistogram *hist;
    CvSize size;
    int i, j;
    int hist_size = 255;
    float range_0[] = { 0, 256 };
    float* ranges[] = { range_0 };
    double PercentOfPixelsNotEdges = 0.7;
    float maxv = 0;



    try{
        size = dx->size();
	}catch (Exception e) {
        cout << "[_AdaptiveFindThreshold]:" << e.msg << endl;
    }

    imge = cvCreateImage(size, IPL_DEPTH_32F, 1);
  	// 计算边缘的强度, 并存于图像中
  	for (i = 0; i < size.height; i++)
  	{
  		const short* _dx = (short*)(dx->data + dx->step*i);
  		const short* _dy = (short*)(dy->data + dy->step*i);
  		float* _image = (float *)(imge->imageData + imge->widthStep*i);
  		for (j = 0; j < size.width; j++)
  		{
  			_image[j] = (float)(abs(_dx[j]) + abs(_dy[j]));
  			maxv = maxv < _image[j] ? _image[j] : maxv;

  		}
  	}
  	if (maxv == 0) {
  		*high = 0;
  		*low = 0;
  		cvReleaseImage(&imge);
  		return;
  	}

  	// 计算直方图
  	range_0[1] = maxv;
  	hist_size = (int)(hist_size > maxv ? maxv : hist_size);
  	hist = cvCreateHist(1, &hist_size, CV_HIST_ARRAY, ranges, 1);
  	cvCalcHist(&imge, hist, 0, NULL);
  	int total = (int)(size.height * size.width * PercentOfPixelsNotEdges);
  	float sum = 0;
  	int icount = hist->mat.dim[0].size;

  	float *h = (float*)cvPtr1D(hist->bins, 0);
  	for (i = 0; i < icount; i++)
  	{
  		sum += h[i];
  		if (sum > total)
  			break;
  	}
  	// 计算高低门限
  	*high = (i + 1) * maxv / hist_size;
  	*low = *high * 0.4;
	cvReleaseImage(&imge);
  	cvReleaseHist(&hist);
}

void AdaptiveFindThreshold(const Mat* image, double *low, double *high, int aperture_size = 3)
{
	//cv::Mat src = cv::cvarrToMat(image);
	Mat src = *image;
	const int cn = src.channels();
	cv::Mat dx(src.rows, src.cols, CV_16SC(cn));
	cv::Mat dy(src.rows, src.cols, CV_16SC(cn));

	cv::Sobel(src, dx, CV_16S, 1, 0, aperture_size, 1, 0, cv::BORDER_REPLICATE);
	cv::Sobel(src, dy, CV_16S, 0, 1, aperture_size, 1, 0, cv::BORDER_REPLICATE);

	Mat _dx = dx, _dy = dy;

	try{
		_AdaptiveFindThreshold(&_dx, &_dy, low, high);
	}catch (Exception e) {
        cout << "[AdaptiveFindThreshold]:" << e.msg << endl;
    }
}

//度数转换
double DegreeTrans(double theta)
{
	double res = theta / CV_PI * 180;
	return res;
}

//通过霍夫变换计算角度
int CalcDegree(const Mat &srcImage, double &degree)
{
	Mat midImage,dstImage;
	Mat tmpImage;

	vector<Vec2f> lines(2000000);

	double low_thresh = 0.0;
	double high_thresh = 0.0;


	tmpImage = srcImage;// cvarrToMat(filtimg);


	midImage = tmpImage;
	//Mat km = cvMat(3, 3, CV_32FC1, high); //构造单通道浮点矩阵，将图像IplImage结构转换为图像数组

	//cvFilter2D(filtimg, filtdst, &km, cvPoint(-1, -1));  //设参考点为核的中心

	try{
		AdaptiveFindThreshold(&midImage, &low_thresh, &high_thresh);
	}catch (Exception e) {
        cout << "[AdaptiveFindThreshold]:" << e.msg << endl;
    }

	if (high_thresh < 2)
    {
      Canny(srcImage, midImage, low_thresh, high_thresh * 200, 3);
    }

    //imshow("边缘提取1", midImage);

    if (high_thresh < 4)
    {
      Canny(srcImage, midImage, low_thresh, high_thresh * 100, 3);
    }
    else if (high_thresh < 10)
    {
      Canny(srcImage, midImage, low_thresh, high_thresh * 40, 3);
    }
    else if (high_thresh < 20)
    {
      Canny(srcImage, midImage, low_thresh * 10, high_thresh * 30, 3);
    }
    else if (high_thresh < 30)
    {
      Canny(srcImage, midImage, low_thresh * 10, high_thresh * 20, 3);
    }
    else if (high_thresh < 40)
    {
      Canny(srcImage, midImage, low_thresh * 10, high_thresh * 10, 3);
    }
    else if (high_thresh < 50)
    {
      Canny(srcImage, midImage, low_thresh * 10, high_thresh * 2, 3);
    }
    else if (high_thresh < 60)
    {
      Canny(srcImage, midImage, low_thresh * 10, high_thresh, 3);
    }
    else if (high_thresh < 70)
    {
      Canny(srcImage, midImage, low_thresh, high_thresh, 3);
    }
    else
    {
      Canny(srcImage, midImage, 10, 60, 3);
    }

    //通过霍夫变换检测直线
    lines.clear();
    HoughLines(midImage, lines, 1, CV_PI / 180, 1900, 0, 0);//第5个参数就是阈值，阈值越大，检测精度越高
    //													  //由于图像不同，阈值不好设定，因为阈值设定过高导致无法检测直线，阈值过低直线太多，速度很慢

    ////所以根据阈值由大到小设置了三个阈值，如果经过大量试验后，可以固定一个适合的阈值。
    if (lines.size()<3)
    {
      HoughLines(midImage, lines, 1, CV_PI / 180, 1600, 0, 0);
    }
    if (lines.size()<3)
    {
      HoughLines(midImage, lines, 1, CV_PI / 180, 1400, 0, 0);
    }
    if (lines.size()<3)
    {
      HoughLines(midImage, lines, 1, CV_PI / 180, 1200, 0, 0);
    }
    if (lines.size()<3)
    {
      HoughLines(midImage, lines, 1, CV_PI / 180, 900, 0, 0);
    }
    if (lines.size()<3)
    {
      HoughLines(midImage, lines, 1, CV_PI / 180, 800, 0, 0);
    }
    if (lines.size()<3)
    {
      HoughLines(midImage, lines, 1, CV_PI / 180, 750, 0, 0);
    }
    if (lines.size()<3)
    {
      HoughLines(midImage, lines, 1, CV_PI / 180, 700, 0, 0);
    }
    if (lines.size()<3)
    {
      HoughLines(midImage, lines, 1, CV_PI / 180, 650, 0, 0);
    }
    if (lines.size()<3)
    {
      HoughLines(midImage, lines, 1, CV_PI / 180, 600, 0, 0);
    }
    if (lines.size()<3)
    {
      HoughLines(midImage, lines, 1, CV_PI / 180, 550, 0, 0);
    }
    if (lines.size()<3)
    {
      HoughLines(midImage, lines, 1, CV_PI / 180, 500, 0, 0);
    }
    if (lines.size()<3)
    {
      HoughLines(midImage, lines, 1, CV_PI / 180, 450, 0, 0);
    }
    if (lines.size()<3)
    {
      HoughLines(midImage, lines, 1, CV_PI / 180, 400, 0, 0);
    }
    if (lines.size()<3)
    {
      HoughLines(midImage, lines, 1, CV_PI / 180, 350, 0, 0);
    }
    if (lines.size()<3)
    {
      HoughLines(midImage, lines, 1, CV_PI / 180, 300, 0, 0);
    }
    if (lines.size()<3)
    {
      HoughLines(midImage, lines, 1, CV_PI / 180, 250, 0, 0);
    }
    if (lines.size()<3)
    {
      HoughLines(midImage, lines, 1, CV_PI / 180, 200, 0, 0);
    }
    if (lines.size()<3)
    {
      HoughLines(midImage, lines, 1, CV_PI / 180, 175, 0, 0);
    }
    if (lines.size()<3)
    {
      HoughLines(midImage, lines, 1, CV_PI / 180, 160, 0, 0);
    }
    if (lines.size()<3)
    {
      HoughLines(midImage, lines, 1, CV_PI / 180, 150, 0, 0);
    }
    if (lines.size()<3)
    {
      HoughLines(midImage, lines, 1, CV_PI / 180, 140, 0, 0);
    }
    if (lines.size()<3)
    {
      HoughLines(midImage, lines, 1, CV_PI / 180, 125, 0, 0);
    }
    if (lines.size()<3)
    {
      HoughLines(midImage, lines, 1, CV_PI / 180, 110, 0, 0);
    }
    if (lines.size()<3)
    {
      HoughLines(midImage, lines, 1, CV_PI / 180, 100, 0, 0);
    }


    float sum = 0;
    int linesizever = 0;
    int count = 0;
    //依次画出每条线段

    for (size_t i = 0; i < lines.size(); i++)
    {
      count++;

      float rho = lines[i][0];
      float theta = lines[i][1];
      Point pt1, pt2;

      double a = cos(theta), b = sin(theta);

      //只选角度最小的作为旋转角度
      if ((theta < 0.3925 &&theta >= 0.0) || (theta < 1.9625 &&theta>1.1775) || (theta <3.14 &&theta>2.7475))
      {
        sum += theta;
        linesizever++;
      }

      line(dstImage, pt1, pt2, Scalar(55, 100, 195), 1, 16); //Scalar函数用于调节线段颜色
    }

    if (lines.size() > 3)
    {
      linesizever++;
    }

    //释放向量内存
    vector<Vec2f>().swap(lines);

    midImage.release();

    float average=0.0;
    if (linesizever > 0)
    {
      average = sum / linesizever; //对所有角度求平均，这样做旋转效果会更好
                     //cout << "average theta:" << average << endl;
      double angle = DegreeTrans(average) - 90;
      degree = angle;
      return 0;
    }
    else
    {
      return 1;
    }
}

//static jmp_buf buf;
//
//void exception_handler(int signum) {
//	printf("ERROR signal is %d", signum);
//	longjmp(buf, 1);
//}

JNIEXPORT jint JNICALL Java_com_BlankPageDetectDLL_BlankPageDetect
(JNIEnv *env, jclass cls, jstring SrcPath)
{
    int result = 0;
    const char *c_str = NULL;
    jboolean isCopy;	// 返回JNI_TRUE表示原字符串的拷贝，返回JNI_FALSE表示返回原字符串的指针
    string srcpath;
    double degree=0.0;
    Mat sourceImage;
    Mat src;
    try{
        c_str = env->GetStringUTFChars(SrcPath, &isCopy);
        srcpath = c_str;
        sourceImage = imread(srcpath);
	}catch (Exception e) {
        cout << "[file error]:" << e.msg << endl;
        return -1;
    }
    Rect rect(100, 60/*srcImg.rows /4*/, sourceImage.cols - 200, sourceImage.rows - 200);
    src = sourceImage(rect);

    try{
        result = CalcDegree(src, degree);
	}catch (Exception e) {
        cout << "[AdaptiveFindThreshold]:" << e.msg << endl;
    }

    return  result;
}


//void fphandler(int num)
//{
//	fperr = num;
//	_fpreset();
//	longjmp(mark, -1);
//
//}
//
//void fpcheck(void)
//{
//	char fpstr[30];
//	//switch (fperr)
//	//{
//	//case _FPE_INVALID:
//	//	strcpy(fpstr, "Invalid number");
//	//	break;
//
//	//case _FPE_OVERFLOW:
//	//	strcpy(fpstr, "Overflow");
//	//	break;
//	//case _FPE_UNDERFLOW:
//	//	strcpy(fpstr, "UnderFlow");
//	//	break;
//	//case _FPE_ZERODIVIDE:
//	//	strcpy(fpstr, "Divide by zero");
//	//	break;
//	//}
//	printf("Error");
//	//return 3;
//
//
//}

