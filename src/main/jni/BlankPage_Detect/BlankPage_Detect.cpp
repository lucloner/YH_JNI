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

using namespace cv;
using namespace std;

extern int image_height,image_width;
extern double theta_range;
static int image_height,image_width;
static double theta_range=CV_PI*0.001;
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
	while (it != lines.end())
	{
		// 以下两个参数用来检测直线属于垂直线还是水平线
		float rho = (*it)[0];
		float theta = (*it)[1];
		if (theta < CV_PI / 4. || theta > 3. * CV_PI / 4.)
		{  // 若检测为垂直线,直线交于图片的上下两边,先找交点
            cv::Point pt1(rho / cos(theta), 0);
            cv::Point pt2((rho - result.rows*sin(theta)) / cos(theta), result.rows);
            cv::line(result, pt1, pt2, cv::Scalar(0,0,255), 5); //
//            cout << "theta vertical " << lines_theta << endl;
            if(theta<theta_range||theta>2.*CV_PI-theta_range||(theta<CV_PI+theta_range&&theta>CV_PI-theta_range)){
                sum_vertical++;
            }
		}
		else // 若检测为水平线,直线交于图片的左右两边,先找交点
		{
            cv::Point pt1(0, rho / sin(theta));
            cv::Point pt2(result.cols, (rho - result.cols*cos(theta)) / sin(theta));
            cv::line(result, pt1, pt2, cv::Scalar(0,0,255), 5);
//            cout << "theta horizon " << theta << endl;
            if((theta<CV_PI/2.+theta_range&&theta>CV_PI/2.-theta_range)||(theta<CV_PI*3./4.+theta_range&&theta>CV_PI*3./4.-theta_range)){
                sum_horizon++;
            }
		}
		const int sum=max(sum_horizon,sum_vertical);
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
		++it;
	}
	return INT_MIN;
}

//通过霍夫变换计算角度
int CalcDegree(const Mat &srcImage, double &degree)
{
	Mat midImage,dstImage;
	Mat tmpImage;
	//此处控制阈值
    const int poolSize=100;

	vector<Vec2f> lines(poolSize);

	double low_thresh = 0.0;
	double high_thresh = 0.0;

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

    //imshow("Black white image", midImage);
    //waitKey(0);

    const Mat hough_img=midImage.clone();
    //通过霍夫变换检测直线
    lines.clear();
    //通过逼近法求合适的值
    //获得图片大小
    //CvSize size=midImage.size();
    const int width=image_width;
    const int height=image_height;
    //cout << "w " << width << " h " << height << endl;
    int cur=max(width/4,height/4),lineCnt=0,lastLineCnt=-1,touch=0,touchSame=0,touchZero=0;
    while(lineCnt<poolSize*2||(touch+touchSame+touchZero<5)){
        HoughLines(hough_img, lines, 1, CV_PI / 180, cur, 0, 0);//第5个参数就是阈值，阈值越小，检测精度越高
        //由于图像不同，阈值不好设定，因为阈值设定过高导致无法检测直线，阈值过低直线太多，速度很慢
        lineCnt=lines.size();
        //cout << "lineCnt " << lineCnt << ", cur " << cur << endl;
        if(lineCnt>poolSize){
            cur=cur*sqrt(cur)+1;
            touch++;
        }
        else if(lineCnt<=1&&cur<=1){
            return max(1,lastLineCnt);
        }
        else{
            if(lastLineCnt==lineCnt&&lineCnt!=0){
                touch--;
                touchZero=0;
                touchSame++;
            }
            else if(lineCnt==0){
                touch--;
                touchSame--;
                touchZero++;
            }
            cur=sqrt(cur)+touch;
        }
        if(touch>1){
             break;
        }
        else if(touchSame>2){
            break;
        }
        else if(touchZero>2){
            return INT_MAX;
        }

        lastLineCnt=lineCnt;
    }
    //下面找到文字行所代表的横线
    //显示测试图片
    Mat hough_img_line=srcImage.clone();
    const int sum=drawDetectedLines(hough_img_line,lines);
//    imshow("Lines", hough_img_line);
//    waitKey(0);

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
        sourceImage = imread(srcpath);
	}catch (Exception e) {
        cout << "[file error]:" << e.msg << endl;
        return -1;
    }

    cvtColor(sourceImage,src, COLOR_BGR2GRAY);
    threshold(src, sourceImage, 127, 255, THRESH_BINARY);

    Rect rect(100, 60/*srcImg.rows /4*/, sourceImage.cols - 200, sourceImage.rows - 200);
    src = sourceImage(rect);

    try{
        result = CalcDegree(src, degree);
	}catch (Exception e) {
        cout << "[Java_com_BlankPageDetectDLL_BlankPageDetect]:" << e.msg << endl;
    }

    return  result;
}
