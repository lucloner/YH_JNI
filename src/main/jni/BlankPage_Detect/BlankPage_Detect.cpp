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

//调试开关 0正常 1调试显示窗口 2调试不显示窗口
#define DEBUG 0

using namespace cv;
using namespace std;

extern int image_height,image_width;
extern double theta_offset;
extern Mat origImg;
static int image_height,image_width;
static double theta_offset;
static Mat origImg;
//此处控制阈值
const int poolSize=100;
const double divideRate=1000;
const double theta_range=CV_PI*2/divideRate;
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
    //判断页面方向
    const int direction=image_height-image_width;
    //计算页面直线区域,同区域的执行不可以重复记录
    const double v_grid_length=image_width/poolSize/2;
    const double h_grid_length=image_height/poolSize/2;
    int v_grid[poolSize*2+1];
    int h_grid[poolSize*2+1];

    //计算角度修正
    const double theta_fix_length=theta_range/2;
    int theta_fix[(int)(divideRate*2+1)];
    double v_theta_offset=0;
    double h_theta_offset=0;
    int v_theta_fix_max=0;
    int h_theta_fix_max=0;

	while (it != lines.end())
	{
		// 以下两个参数用来检测直线属于垂直线还是水平线
		const double rho = (*it)[0];
		double theta = (*it)[1];
		const int theta_sum=++theta_fix[(int)(theta/theta_fix_length)];

		//求画直线的点
		const double v_x_1=rho / cos(theta);
		const double v_y_1=0;
		const double v_x_2=(rho - result.rows*sin(theta)) / cos(theta);
		const double v_y_2=result.rows;
		const double h_x_1=0;
		const double h_y_1=rho / sin(theta);
		const double h_x_2=result.cols;
		const double h_y_2=(rho - result.cols*cos(theta)) / sin(theta);
#if (DEBUG!=0)
		//求与极坐标垂直的直线
		cv::Point l1(v_x_1, v_y_1);
		cv::Point l2(h_x_1, h_y_1);
		cv::line(result, l1, l2, cv::Scalar(0,255,0,127), 1);
		cout << theta << " theta,rho " << rho << " direction " << direction << endl;
#endif
        //整理角度 使范围在[0,pi]之间
        theta=fmod(theta,CV_PI*2.);
        theta=theta<0?CV_PI+theta:theta;
        theta=theta>CV_PI?theta-CV_PI:theta;

        //根据页面方向计算修正
        double temp_offset;
        int temp_max;
        if(direction<0){//如果是竖线
            temp_offset=v_theta_offset;
            temp_max=v_theta_fix_max;
    		if(theta_sum>=temp_max){
        	    temp_offset=theta;
        	    temp_max=theta_sum;
        	}
       		if(abs(temp_offset)<CV_PI/8.){
       		    theta-=temp_offset;
       		    v_theta_offset=temp_offset;
       		    v_theta_fix_max=temp_max;
       		    theta_offset=temp_offset;
       		}
        }
        else {//视为横线
            temp_offset=h_theta_offset;
            temp_max=h_theta_fix_max;
    		if(theta_sum>=temp_max){
        	    temp_offset=CV_PI/2.-theta;
        	    temp_max=theta_sum;
        	}
       		if(abs(temp_offset)<CV_PI/8.){
       		    theta+=temp_offset;
       		    h_theta_offset=temp_offset;
       		    h_theta_fix_max=temp_max;
       		    theta_offset=temp_offset;
       		}
        }

	    if (theta <CV_PI/4. || theta > 3.*CV_PI/4.)
		{  // 若检测为垂直线,直线交于图片的上下两边,先找交点
            if(theta<theta_range||theta>2.*CV_PI-theta_range||(theta<CV_PI+theta_range&&theta>CV_PI-theta_range)){
                int sum_v_grid=max(v_grid[(int)(v_x_1/v_grid_length)]++,v_grid[(int)(v_x_2/v_grid_length)]++);
                if(sum_v_grid<3){
                    sum_vertical++;
                }
#if (DEBUG!=0)
                cout << sum_vertical << " sum_vertical,sum_v_grid " << sum_v_grid << endl;
                cv::Point pt1(v_x_1, v_y_1);
                cv::Point pt2(v_x_2, v_y_2);
                cv::line(result, pt1, pt2, cv::Scalar(0,0,255), 1);
#endif
            }
		}
		else // 若检测为水平线,直线交于图片的左右两边,先找交点
		{
            if((theta<CV_PI/2.+theta_range&&theta>CV_PI/2.-theta_range)||(theta<CV_PI*3./4.+theta_range&&theta>CV_PI*3./4.-theta_range)){
                int sum_h_grid=max(h_grid[(int)(h_y_1/h_grid_length)]++,h_grid[(int)(h_y_2/h_grid_length)]++);
                if(sum_h_grid<3){
                    sum_horizon++;
                }
#if (DEBUG!=0)
                cout << sum_horizon << " sum_horizon,sum_h_grid " << sum_h_grid << endl;
                cv::Point pt1(h_x_1, h_y_1);
                cv::Point pt2(h_x_2, h_y_2);
                cv::line(result, pt1, pt2, cv::Scalar(255,0,0), 1);
#endif
            }
		}
		const int sum=max(sum_horizon,sum_vertical);
#if (DEBUG==0)
		if(direction>0){
		    if(sum_horizon>3){
		        return sum;
		    }
		}
		else{
		    if(sum_vertical>3){
		        return sum;
		    }
		}
#else
        cout << theta << " theta_fix,temp_offset " << temp_offset << " temp_max " << temp_max << endl;
#endif
		++it;
	}
	return INT_MIN;
}

//通过霍夫变换计算角度
int CalcDegree(const Mat &srcImage, double &degree)
{
	Mat tmpImage,grayImage,binImage;
    //去除边上下左右5%
    const int h_boarder=srcImage.rows*0.1;
    const int w_boarder=srcImage.cols*0.1;
    //裁切
    Rect rect(w_boarder, h_boarder, srcImage.cols-w_boarder, srcImage.rows-h_boarder);
    const Mat orig = srcImage(rect);
    origImg=origImg(rect);
    binImage=orig;

	vector<Vec2f> lines(poolSize);

	double low_thresh = 0.0;
	double high_thresh = 0.0;

	try{
		AdaptiveFindThreshold(&binImage, &low_thresh, &high_thresh);
	}catch (Exception e) {
        cout << "[CalcDegree]:" << e.msg << endl;
    }

    //获得图片大小
    const int width=image_width;
    const int height=image_height;

	if (high_thresh < 2)
    {
      Canny(binImage, tmpImage, low_thresh, high_thresh * 200, 3);
    }
    else if (high_thresh < 4)
    {
      Canny(binImage, tmpImage, low_thresh, high_thresh * 100, 3);
    }
    else if (high_thresh < 10)
    {
      Canny(binImage, tmpImage, low_thresh, high_thresh * 40, 3);
    }
    else if (high_thresh < 20)
    {
      Canny(binImage, tmpImage, low_thresh * 10, high_thresh * 30, 3);
    }
    else if (high_thresh < 30)
    {
      Canny(binImage, tmpImage, low_thresh * 10, high_thresh * 20, 3);
    }
    else if (high_thresh < 40)
    {
      Canny(binImage, tmpImage, low_thresh * 10, high_thresh * 10, 3);
    }
    else if (high_thresh < 50)
    {
      Canny(binImage, tmpImage, low_thresh * 10, high_thresh * 2, 3);
    }
    else if (high_thresh < 60)
    {
      Canny(binImage, tmpImage, low_thresh * 10, high_thresh, 3);
    }
    else if (high_thresh < 70)
    {
      Canny(binImage, tmpImage, low_thresh, high_thresh, 3);
    }
    else
    {
      Canny(binImage, tmpImage, 10, 60, 3);
    }

    //imshow("Black white image", tmpImage);
    //waitKey(0);

    const Mat hough_img=tmpImage;
    //通过霍夫变换检测直线
    lines.clear();
    //通过逼近法求合适的值
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
        if(cur<=1&&lineCnt<=3){
            return INT_MAX;
        }
        else if(lineCnt<=3){ //要缩很小
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
            touchSame=0;
            if(touchZero>0){
                cur=min((curmax+curmin)/2,(int)(cur*1.5));
            }
            touchZero=0;
            if(lineCnt/poolSize<10+pow(10,touch)/abs(curmax-curmin)){
                touch++;
            }
        }
        else{
            break;
        }

        if(curmin++>=curmax--){
            if(lineCnt<=3){
                return INT_MAX;
            }
            break;
        }
        else if(lineCnt==lastLineCnt){
            touchSame++;
            touchZero=0;
            touch--;
        }

        if(touch>1){
            if((lastLineCnt<lineCnt&&lastLineCnt>poolSize/2)||lineCnt>poolSize*3){
                HoughLines(hough_img, lines, 1, CV_PI / 180, lastCur, 0, 0);
            }
            touchSame=0;
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

#if (DEBUG==1)
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

char* jstringToChar(JNIEnv *env, jstring jstr)
{
    char * rtn = NULL;
    jclass clsstring = env->FindClass("java/lang/String");
    jstring strencode = env->NewStringUTF("GBK");
    jmethodID mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");
    jbyteArray barr= (jbyteArray)env->CallObjectMethod(jstr,mid,strencode);
    jsize alen = env->GetArrayLength(barr);
    jbyte * ba = env->GetByteArrayElements(barr,JNI_FALSE);
    if(alen > 0)
    {
        rtn = (char*)malloc(alen+1); //new char[alen+1];
        memcpy(rtn,ba,alen);
        rtn[alen]=0;
    }
    env->ReleaseByteArrayElements(barr,ba,0);

    return rtn;
}

JNIEXPORT jint JNICALL Java_com_BlankPageDetectDLL_BlankPageDetect
(JNIEnv *env, jclass cls, jstring SrcPath)
{
    int result = 0;
    double degree=0.0;
    Mat src,tmpImg,dstImg;
    char *c_str;
    try{
        c_str=jstringToChar(env,SrcPath);
        string srcpath = c_str;
        src = imread(srcpath);
        origImg=src.clone();
        cvtColor(src,tmpImg,COLOR_BGR2GRAY);
        threshold(tmpImg,dstImg,127,255,THRESH_BINARY);
	}catch (Exception e) {
        cout << "[file error]:" << e.msg << endl;
        return -1;
    }

    try{
        result = CalcDegree(dstImg, degree);
        cout << result << "\t<-BlankPageDetect result(>0 isBlank),image[\t" << c_str << "\t](\t" << image_width << "\t,\t"
            << image_height << "\t)theta offset(deg):\t" << theta_offset/CV_PI*180 << endl;
	}catch (Exception e) {
        cout << "[Java_com_BlankPageDetectDLL_BlankPageDetect]:" << e.msg << endl;
    }

    return  result;
}
