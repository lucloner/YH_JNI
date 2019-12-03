#define _CRT_SECURE_NO_WARNINGS
#include "com_BlankPageDetectDLL.h"
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc_c.h"
#include <iostream>
#include <fcntl.h>
#if defined(__linux__) || defined(__linux)
#  include <sys/io.h>
#else
#  include <io.h>
#endif

#include <math.h>
#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <string.h>

//调试开关
#define DEBUG 0

using namespace cv;
using namespace std;

extern int image_height,image_width;
extern Mat origImg;
static int image_height,image_width;
static Mat origImg;
//此处控制阈值
const int poolSize=100;
const double theta_range=CV_PI*0.001;
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
        image_height=size.height;
        image_width=size.width;
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

int drawDetectedLines(Mat& result,vector<Vec2f> lines){
	std::vector<cv::Vec2f>::const_iterator it = lines.begin();
    int sum_horizon=0,sum_vertical=0;
    const int direction=image_height-image_width;
    double fix=0;
	while (it != lines.end())
	{
		// 以下两个参数用来检测直线属于垂直线还是水平线
		double rho = (*it)[0];
		double theta = (*it)[1];
#if (DEBUG!=0)
		//求画直线的点
		double v_x_1=rho / cos(theta);
		double v_y_1=0;
		double v_x_2=(rho - result.rows*sin(theta)) / cos(theta);
		double v_y_2=result.rows;
		double h_x_1=0;
		double h_y_1=rho / sin(theta);
		double h_x_2=result.cols;
		double h_y_2=(rho - result.cols*cos(theta)) / sin(theta);
		//求与极坐标垂直的直线
		cv::Point l1(v_x_1, v_y_1);
		cv::Point l2(h_x_1, h_y_1);
		cv::line(result, l1, l2, cv::Scalar(127,127,127,127), 1);
#endif
	    if (theta < CV_PI / 4. || theta > 3. * CV_PI / 4.)
		{  // 若检测为垂直线,直线交于图片的上下两边,先找交点
            if(theta<theta_range||theta>2.*CV_PI-theta_range||(theta<CV_PI+theta_range&&theta>CV_PI-theta_range)){
                sum_vertical++;
#if (DEBUG!=0)
                cv::Point pt1(v_x_1, v_y_1);
                cv::Point pt2(v_x_2, v_y_2);
                cv::line(result, pt1, pt2, cv::Scalar(0,0,255), 1);
#endif
            }
		}
		else // 若检测为水平线,直线交于图片的左右两边,先找交点
		{
            if((theta<CV_PI/2.+theta_range&&theta>CV_PI/2.-theta_range)||(theta<CV_PI*3./4.+theta_range&&theta>CV_PI*3./4.-theta_range)){
                sum_horizon++;
#if (DEBUG!=0)
                cv::Point pt1(h_x_1, h_y_1);
                cv::Point pt2(h_x_2, h_y_2);
                cv::line(result, pt1, pt2, cv::Scalar(0,0,255), 1);
#endif
            }
		}
		const int sum=max(sum_horizon,sum_vertical);
#if (DEBUG==0)
		if(direction>0){
		    if(sum_horizon>3){
		        return sum;
		    }
//		    cout << "sum_horizon " << sum_horizon << endl;
		}
		else{
		    if(sum_vertical>3){
		        return sum;
		    }
//		    cout << "sum_vertical " << sum_vertical << endl;
		}
#endif
		++it;
	}
	return INT_MIN;
}

//通过霍夫变换计算角度
int CalcDegree(const Mat &srcImage, double &degree)
{
	Mat midImage,dstImage;
	Mat tmpImage;

	vector<Vec2f> lines(poolSize);

	double low_thresh = 0.0;
	double high_thresh = 0.0;

#if (DEBUG<200)
	try{
		AdaptiveFindThreshold(&srcImage, &low_thresh, &high_thresh);
	}catch (Exception e) {
        cout << "[CalcDegree]:" << e.msg << endl;
    }

	if (high_thresh < 2)
    {
      Canny(srcImage, midImage, low_thresh, high_thresh * 200, 3);
    }
    else if (high_thresh < 4)
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
#else
    midImage=srcImage;
#endif
    //imshow("Black white image", midImage);
    //waitKey(0);

    const Mat hough_img=midImage;
    //通过霍夫变换检测直线
    lines.clear();
    //通过逼近法求合适的值
    //获得图片大小
    //CvSize size=midImage.size();
    const int width=image_width;
    const int height=image_height;
    //cout << "w " << width << " h " << height << endl;
    int curmax=min(width,height),lineCnt=0,lastLineCnt=-1,touch=0,touchSame=0,touchZero=0,cur=curmax,curmin=0,lastCur=cur;
    while(lineCnt<poolSize*2||(touchSame+touchZero<5)){
        int tempCur=cur;
        HoughLines(hough_img, lines, 1, CV_PI / 180, abs(cur), 0, 0);//第5个参数就是阈值，阈值越小，检测精度越高
        //由于图像不同，阈值不好设定，因为阈值设定过高导致无法检测直线，阈值过低直线太多，速度很慢
        lineCnt=lines.size();
#if (DEBUG!=0)
        cout << "lineCnt " << lineCnt << ", cur " << cur << ", min " << curmin << ", max " << curmax << endl;
#endif
        if(lineCnt<=3){ //要缩很小
            curmax=min(curmax,cur);
            cur=max(curmin++,(int)sqrt(cur));
            touchZero++;
            touch=0;
        }
        else if(lineCnt<poolSize){  //要缩小
            curmax=min(curmax,cur);
            cur=max(cur/2,curmin);

        }
        else if(lineCnt>poolSize){  //要增大
            curmin=max(curmin,cur);
            cur=min(curmax,(int)(cur*sqrt(cur)));
            touch++;
            touchSame=0;
            touchZero=0;
        }
        else{
            break;
        }

        if(curmin++>=curmax--){
            break;
        }
        else if(lineCnt==lastLineCnt){
            touchSame++;
            touchZero=0;
            touch--;
        }

        if(touch>1){
            int last=abs(poolSize-lastLineCnt);
            int now=abs(poolSize-lineCnt);
            if(last<now){
                HoughLines(hough_img, lines, 1, CV_PI / 180, lastCur, 0, 0);
                cout << "return lineCnt " << lines.size() << ", cur " << lastCur << endl;
            }
            break;
        }

        if(touchSame>2||touchZero>2){
            break;
        }

        lastLineCnt=lineCnt;
        lastCur=tempCur;
    }
    //下面找到文字行所代表的横线
    //显示测试图片
    Mat hough_img_line=origImg;
    const int sum=drawDetectedLines(hough_img_line,lines);

#if (DEBUG!=0)
    namedWindow("Lines",0);
    imshow("Lines", hough_img_line);
    waitKey(0);
#endif

    if(sum>3){
        return 0;
    }

    lineCnt=lineCnt/100*100;
    return max(sum+lineCnt,1);
}

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
        src = imread(srcpath);
        origImg=src;
	}catch (Exception e) {
        cout << "[file error]:" << e.msg << endl;
        return -1;
    }
    cvtColor(src,sourceImage, COLOR_BGR2GRAY);
#if (DEBUG<100)
    threshold(sourceImage, sourceImage, 127, 255, THRESH_BINARY);
#endif
    Rect rect(100, 60/*srcImg.rows /4*/, sourceImage.cols - 200, sourceImage.rows - 200);
    src = sourceImage(rect);

    try{
        result = CalcDegree(src, degree);
	}catch (Exception e) {
        cout << "[Java_com_BlankPageDetectDLL_BlankPageDetect]:" << e.msg << endl;
    }

    return  result;
}
