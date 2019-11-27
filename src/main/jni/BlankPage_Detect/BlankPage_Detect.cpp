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
//	// �����쳣��
//	jclass cls = env->FindClass(name);
//	/* �������쳣��û���ҵ���VM���׳�һ��NowClassDefFoundError�쳣 */
//	if (cls != NULL) {
//		env->ThrowNew(cls, msg);  // �׳�ָ�����ֵ��쳣
//	}
//	/* �ͷžֲ����� */
//	env->DeleteLocalRef(cls);
//}

bool SetResolution(const char* path, int iResolution)
{
	FILE * file = fopen(path, "rb+");// - ��ͼƬ�ļ�
	if (!file)return false;
	int len = _filelength(_fileno(file));// - ��ȡ�ļ���С
	char* buf = new char[len];
	fread(buf, sizeof(char), len, file);// - ��ͼƬ���ݶ��뻺��
	char * pResolution = (char*)&iResolution;// - iResolutionΪҪ���õķֱ��ʵ���ֵ����72dpi
											 // - ����JPGͼƬ�ķֱ���
	buf[0x0D] = 1;// - ����ʹ��ͼƬ�ܶȵ�λ
				  // - ˮƽ�ܶȣ�ˮƽ�ֱ���
	buf[0x0E] = pResolution[1];
	buf[0x0F] = pResolution[0];
	// - ��ֱ�ܶȣ���ֱ�ֱ���
	buf[0x10] = pResolution[1];
	buf[0x11] = pResolution[0];

	// - ���ļ�ָ���ƶ��ص��ļ���ͷ
	fseek(file, 0, SEEK_SET);
	// - ������д���ļ�������ԭʼ�����ݣ����޸���Ч
	fwrite(buf, sizeof(char), len, file);
	fclose(file);
	return true;
}


// ����matlab������Ӧ��ߵ���������
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
  	// �����Ե��ǿ��, ������ͼ����
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

  	// ����ֱ��ͼ
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
  	// ����ߵ�����
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

//����ת��
double DegreeTrans(double theta)
{
	double res = theta / CV_PI * 180;
	return res;
}

//ͨ������任����Ƕ�
int CalcDegree(const Mat &srcImage, double &degree)
{
	Mat midImage,dstImage;
	Mat tmpImage;

	vector<Vec2f> lines(2000000);

	double low_thresh = 0.0;
	double high_thresh = 0.0;


	tmpImage = srcImage;// cvarrToMat(filtimg);


	midImage = tmpImage;
	//Mat km = cvMat(3, 3, CV_32FC1, high); //���쵥ͨ��������󣬽�ͼ��IplImage�ṹת��Ϊͼ������

	//cvFilter2D(filtimg, filtdst, &km, cvPoint(-1, -1));  //��ο���Ϊ�˵�����

	try{
		AdaptiveFindThreshold(&midImage, &low_thresh, &high_thresh);
	}catch (Exception e) {
        cout << "[AdaptiveFindThreshold]:" << e.msg << endl;
    }

	if (high_thresh < 2)
    {
      Canny(srcImage, midImage, low_thresh, high_thresh * 200, 3);
    }

    //imshow("��Ե��ȡ1", midImage);

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

    //ͨ������任���ֱ��
    lines.clear();
    HoughLines(midImage, lines, 1, CV_PI / 180, 1900, 0, 0);//��5������������ֵ����ֵԽ�󣬼�⾫��Խ��
    //													  //����ͼ��ͬ����ֵ�����趨����Ϊ��ֵ�趨���ߵ����޷����ֱ�ߣ���ֵ����ֱ��̫�࣬�ٶȺ���

    ////���Ը�����ֵ�ɴ�С������������ֵ�����������������󣬿��Թ̶�һ���ʺϵ���ֵ��
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
    //���λ���ÿ���߶�

    for (size_t i = 0; i < lines.size(); i++)
    {
      count++;

      float rho = lines[i][0];
      float theta = lines[i][1];
      Point pt1, pt2;

      double a = cos(theta), b = sin(theta);

      //ֻѡ�Ƕ���С����Ϊ��ת�Ƕ�
      if ((theta < 0.3925 &&theta >= 0.0) || (theta < 1.9625 &&theta>1.1775) || (theta <3.14 &&theta>2.7475))
      {
        sum += theta;
        linesizever++;
      }

      line(dstImage, pt1, pt2, Scalar(55, 100, 195), 1, 16); //Scalar�������ڵ����߶���ɫ
    }

    if (lines.size() > 3)
    {
      linesizever++;
    }

    //�ͷ������ڴ�
    vector<Vec2f>().swap(lines);

    midImage.release();

    float average=0.0;
    if (linesizever > 0)
    {
      average = sum / linesizever; //�����нǶ���ƽ������������תЧ�������
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
    jboolean isCopy;	// ����JNI_TRUE��ʾԭ�ַ����Ŀ���������JNI_FALSE��ʾ����ԭ�ַ�����ָ��
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

