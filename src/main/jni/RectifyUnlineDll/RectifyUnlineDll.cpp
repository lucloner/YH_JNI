#include "com_RectifyUnlineDll.h"
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

int number = 0;

using namespace cv;
using namespace std;

#define V_PROJECT 1  //垂直投影（vertical）
#define H_PROJECT 2  //水平投影（horizational）

typedef struct
{
	int begin;
	int end;

}char_range_t;

vector<Vec2f> lines;
vector<Vec4i> lines_l(5000);
Vec4i ln;

//度数转换
double DegreeTrans(double theta)
{
	double res = theta / CV_PI * 180;
	return res;
}

// 仿照matlab，自适应求高低两个门限                                            
void _AdaptiveFindThreshold(Mat *dx, Mat *dy, double *low, double *high)
{
	CvSize size;
	IplImage *imge = 0;
	int i, j;
	CvHistogram *hist;
	int hist_size = 255;
	float range_0[] = { 0, 256 };
	float* ranges[] = { range_0 };
	double PercentOfPixelsNotEdges = 0.7;
	size = dx->size();
	imge = cvCreateImage(size, IPL_DEPTH_32F, 1);
	// 计算边缘的强度, 并存于图像中                                        
	float maxv = 0;
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
	_AdaptiveFindThreshold(&_dx, &_dy, low, high);

}

//逆时针旋转图像degree角度（原尺寸）    
void rotateImage(Mat src, Mat& img_rotate, double degree)
{
	//旋转中心为图像中心    
	Point2f center;
	center.x = float(src.cols / 2.0);
	center.y = float(src.rows / 2.0);
	int length = 0;
	length = sqrt(src.cols*src.cols + src.rows*src.rows);
	//计算二维旋转的仿射变换矩阵  
	Mat M = getRotationMatrix2D(center, degree, 1);
	warpAffine(src, img_rotate, M, Size(length, length), 1, 0, Scalar(255, 255, 255));//仿射变换，背景色填充为白色  
}

//通过霍夫变换计算角度
void CalcDegree(const Mat &srcImage, double &degree)
{
	Mat midImage, dstImage;

	IplImage*filtdst;

	double low_thresh = 0.0;
	double high_thresh = 0.0;

	float low[9] = { 1.0 / 16, 2.0 / 16, 1.0 / 16, 2.0 / 16, 4.0 / 16, 2.0 / 16, 1.0 / 16, 2.0 / 16, 1.0 / 16 };//低通滤波核
	float high[9] = { -1, -1, -1, -1, 9, -1, -1, -1, -1 };//高通滤波核
    Mat km;
    km = Mat(3, 3, CV_32FC1, high); //构造单通道浮点矩阵，将图像IplImage结构转换为图像数组
    IplImage src2img=IplImage(srcImage);
	IplImage* filtimg = &src2img;

	filtdst = cvCreateImage(srcImage.size(), IPL_DEPTH_8U, 3);

	//cvFilter2D(filtimg, filtdst, &km, cvPoint(-1, -1));  //设参考点为核的中心

	Mat tmpImage;
	tmpImage = cvarrToMat(filtimg);
	midImage = tmpImage;
	//imshow("边缘提取", tmpImage);
	//cvWaitKey(0);
	AdaptiveFindThreshold(&midImage, &low_thresh, &high_thresh);

	if (high_thresh < 10)
	{
		Canny(srcImage, midImage, low_thresh * 10, high_thresh * 180, 3);
	}
	else if (high_thresh < 20)
	{
		Canny(srcImage, midImage, low_thresh * 10, high_thresh * 40, 3);
	}
	else
	{
		Canny(srcImage, midImage, 20, 1000, 3);
	}

	cvtColor(midImage, dstImage, CV_GRAY2BGR);

	//通过霍夫变换检测直线
	lines.clear();
	HoughLines(midImage, lines, 1, CV_PI / 180, 660, 0, 0);//第5个参数就是阈值，阈值越大，检测精度越高
														   //由于图像不同，阈值不好设定，因为阈值设定过高导致无法检测直线，阈值过低直线太多，速度很慢
														   //所以根据阈值由大到小设置了三个阈值，如果经过大量试验后，可以固定一个适合的阈值。
	if (!lines.size())
	{
		HoughLines(midImage, lines, 1, CV_PI / 180, 200, 0, 0);
	}

	if (!lines.size())
	{
		HoughLines(midImage, lines, 1, CV_PI / 180, 150, 0, 0);
	}
	if (!lines.size())
	{
		HoughLines(midImage, lines, 1, CV_PI / 180, 100, 0, 0);
	}
	if (!lines.size())
	{
		HoughLines(midImage, lines, 1, CV_PI / 180, 50, 0, 0);
	}
	//cout << lines.size() << endl;
	if (!lines.size())
	{
		//cout << "没有检测到直线！" << endl;

	}
	float sum = 0;
	int linesizever = 0;
	//依次画出每条线段
	for (size_t i = 0; i < lines.size(); i++)
	{
		float rho = lines[i][0];
		float theta = lines[i][1];
		Point pt1, pt2;
		//cout << theta << endl;
		double a = cos(theta), b = sin(theta);
		double x0 = a*rho, y0 = b*rho;
		//pt1.x = cvRound(x0 + 1000 * (-b));
		//pt1.y = cvRound(y0 + 1000 * (a));
		//pt2.x = cvRound(x0 - 1000 * (-b));
		//pt2.y = cvRound(y0 - 1000 * (a));
		//只选角度最小的作为旋转角度
		if (theta < 2.156 &&theta>0.8853)
		{
			sum += theta;
			linesizever++;
		}

		//line(dstImage, pt1, pt2, Scalar(55, 100, 195), 1, CV_AA); //Scalar函数用于调节线段颜色
	}
	//float average = sum / lines.size(); //对所有角度求平均，这样做旋转效果会更好
	float average;
	if (linesizever > 0)
	{
		average = sum / linesizever; //对所有角度求平均，这样做旋转效果会更好
									 //cout << "average theta:" << average << endl;
		double angle = DegreeTrans(average) - 90;
		degree = angle;
	}

}

JNIEXPORT jdouble JNICALL Java_com_JniDemo_ImageRecify
(JNIEnv *env, jclass cls, jstring SrcPath, jstring DstPath)
{
	const char *c_str = NULL;
	jboolean isCopy;	// 返回JNI_TRUE表示原字符串的拷贝，返回JNI_FALSE表示返回原字符串的指针


	c_str = env->GetStringUTFChars(SrcPath, &isCopy);
	string srcpath = c_str;

	c_str = env->GetStringUTFChars(DstPath, &isCopy);
	string dstpath = c_str;

	Mat sourceImage = imread(srcpath);

	double degree;
	Mat source = imread(srcpath);
	Mat dst;
	Mat src = source;

	if (source.cols > 800 && source.rows >600)
	{
		resize(source, src, Size(src.cols / 6, src.rows / 6), 0, 0, INTER_LINEAR);
	}
	//倾斜角度矫正

//    try{
	    CalcDegree(src, degree);
//	}catch (Exception e) {
//        cout << "[Java_com_JniDemo_ImageRecify]:" << e.msg << endl;
//    }

	if (degree > 0.5 || degree <-0.5)
	{
		rotateImage(source, dst, degree);
	}
	else
	{
		dst = source;
		degree = 0;
	}

	Mat resultImage = dst(Rect(0, 0, source.cols, source.rows)); //根据先验知识，估计好文本的长宽，再裁剪下来
	imwrite(dstpath, resultImage);

	const char*  ch;
	ch = dstpath.data();

	return degree;
}


//获取文本的投影以用于分割字符(垂直，水平),默认图片是白底黑色
int GetTextProjection(Mat &src, vector<int>& pos, int mode)
{
	if (mode == V_PROJECT)
	{
		for (int i = 0; i < src.rows; i++)
		{
			uchar* p = src.ptr<uchar>(i);
			for (int j = 0; j < src.cols; j++)
			{
				if (p[j] == 0)  //是黑色像素
				{
					pos[j]++;
				}
			}
		}
	}
	else if (mode == H_PROJECT)
	{
		for (int i = 0; i < src.cols; i++)
		{

			for (int j = 0; j < src.rows; j++)
			{
				if (src.at<uchar>(j, i) == 0)
				{
					pos[j]++;
				}
			}
		}
	}

	return 0;
}


//获取每个分割字符的范围，min_thresh：波峰的最小幅度，min_range：两个波峰的最小间隔
void GetPeekRange(vector<int> &vertical_pos, vector<char_range_t> &peek_range, int min_thresh, int min_range)
{
	int begin = 0;
	int end = 0;

	for (int i = 0; i < vertical_pos.size(); i++)
	{
		if (vertical_pos[i] > min_thresh && begin == 0)
		{
			begin = i;
		}
		else if (vertical_pos[i] > min_thresh && begin != 0)
		{
			continue;
		}
		else if (vertical_pos[i] < min_thresh && begin != 0)
		{
			end = i;
			if (end - begin >= min_range)
			{
				char_range_t tmp;
				tmp.begin = begin;
				tmp.end = end;
				peek_range.push_back(tmp);

				begin = 0;
				end = 0;
			}

		}
		else if (vertical_pos[i] < min_thresh || begin == 0)
		{
			continue;
		}
		else
		{
			//printf("raise error!\n");
		}
	}

}



void morhpologyLines(Mat &src, Mat &roiImage, vector<char_range_t> &peek_range, Mat &dst, int startX, int startY)
{
	//阈值化图像
	Mat binary_img, morhp_img;
	cvtColor(roiImage, morhp_img, COLOR_BGR2GRAY);
	threshold(morhp_img, binary_img, 0, 255, THRESH_BINARY_INV | THRESH_OTSU);
	//imshow("二值化结果", binary_img);

	//进行开运算操作
	Mat kernel = getStructuringElement(MORPH_RECT, Size(20, 1), Point(-1, -1));     //定义一个宽20、高1的一个矩形结构元
	morphologyEx(binary_img, morhp_img, MORPH_OPEN, kernel, Point(-1, -1));     //作开运算，只保留图像中的直线

																				// 进行膨胀操作
	kernel = getStructuringElement(MORPH_RECT, Size(3, 3), Point(-1, -1));      //定义一个宽3、高3的一个矩形结构元
	dilate(morhp_img, morhp_img, kernel);                             //作膨胀操作，图像中的直线增强
	dilate(morhp_img, morhp_img, kernel);                             //作膨胀操作，图像中的直线增强
																	  // 霍夫直线检测
	lines_l.clear();
	HoughLinesP(morhp_img, lines_l, 1, CV_PI / 180.0, 25.0, 20.0, 0);

	Mat result_img = src.clone();

	int LinesThr = 0;//下划线长度阈值

	for (size_t t = 0; t < lines_l.size(); ++t)
	{
		ln = lines_l[t];

		for (size_t i = 0; i<peek_range.size(); i++)
		{
			if ((ln[1]>(peek_range[i].begin - 6)) && (ln[1]<(peek_range[i].end + 6)))
			{
				LinesThr = 1.2 * (peek_range[i].end - peek_range[i].begin);
				break;
			}
		}

		if ((ln[2] - ln[0])>LinesThr)
		{
			line(result_img, Point(startX + ln[0], startY + ln[1]), Point(startX + ln[2], startY + ln[3]), Scalar(255, 255, 255), 4, 8, 0);
		}
	}
	lines_l.clear();
	dst = result_img;
}


JNIEXPORT void JNICALL Java_com_JniDemo_RemoveUnline(JNIEnv *env, jclass cls, jstring SrcPath, jstring DstPath, jint StartX, jint StartY, jint EndX, jint EndY)
{
	const char *c_str = NULL;
	jboolean isCopy;	// 返回JNI_TRUE表示原字符串的拷贝，返回JNI_FALSE表示返回原字符串的指针


	c_str = env->GetStringUTFChars(SrcPath, &isCopy);
	string srcpath = c_str;

	c_str = env->GetStringUTFChars(DstPath, &isCopy);
	string dstpath = c_str;

	Mat sourceImage = imread(srcpath);

	if (0 == EndX && 0 == EndY)
	{
		EndX = sourceImage.cols;
		EndY = sourceImage.rows;
	}
	if (StartX >EndX || StartY > EndY)
	{
		return;
	}

	Rect roiRect = Rect(StartX, StartY, (EndX - StartX), (EndY - StartY));

	Mat roiImage = sourceImage(roiRect);
	Mat resultImage;

	cvtColor(roiImage, roiImage, COLOR_BGR2GRAY);           //彩色图转为灰度图

	threshold(roiImage, roiImage, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);

	vector<int> hpos(roiImage.rows, 0);
	vector<char_range_t> h_peek_range;

	//计算文本横投影
	GetTextProjection(roiImage, hpos, H_PROJECT);

	//获取各行文本起止行坐标
	GetPeekRange(hpos, h_peek_range, 2, 10);

	roiImage = sourceImage(roiRect);

	morhpologyLines(sourceImage, roiImage, h_peek_range, resultImage, StartX, StartY);

	imwrite(dstpath, resultImage);

	const char*  ch;
	ch = dstpath.data();
}

JNIEXPORT void JNICALL Java_com_JniDemo_set
(JNIEnv *, jclass, jint i)
{
	number = i;
}

JNIEXPORT jint JNICALL Java_com_JniDemo_get
(JNIEnv *, jclass)
{
	//RemoveUnline();
	return number;
}
