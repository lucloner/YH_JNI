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

#define V_PROJECT 1  //��ֱͶӰ��vertical��
#define H_PROJECT 2  //ˮƽͶӰ��horizational��

typedef struct
{
	int begin;
	int end;

}char_range_t;

vector<Vec2f> lines;
vector<Vec4i> lines_l(5000);
Vec4i ln;

//����ת��
double DegreeTrans(double theta)
{
	double res = theta / CV_PI * 180;
	return res;
}

// ����matlab������Ӧ��ߵ���������                                            
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
	// �����Ե��ǿ��, ������ͼ����                                        
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
	_AdaptiveFindThreshold(&_dx, &_dy, low, high);

}

//��ʱ����תͼ��degree�Ƕȣ�ԭ�ߴ磩    
void rotateImage(Mat src, Mat& img_rotate, double degree)
{
	//��ת����Ϊͼ������    
	Point2f center;
	center.x = float(src.cols / 2.0);
	center.y = float(src.rows / 2.0);
	int length = 0;
	length = sqrt(src.cols*src.cols + src.rows*src.rows);
	//�����ά��ת�ķ���任����  
	Mat M = getRotationMatrix2D(center, degree, 1);
	warpAffine(src, img_rotate, M, Size(length, length), 1, 0, Scalar(255, 255, 255));//����任������ɫ���Ϊ��ɫ  
}

//ͨ������任����Ƕ�
void CalcDegree(const Mat &srcImage, double &degree)
{
	Mat midImage, dstImage;

	IplImage*filtdst;

	double low_thresh = 0.0;
	double high_thresh = 0.0;

	float low[9] = { 1.0 / 16, 2.0 / 16, 1.0 / 16, 2.0 / 16, 4.0 / 16, 2.0 / 16, 1.0 / 16, 2.0 / 16, 1.0 / 16 };//��ͨ�˲���
	float high[9] = { -1, -1, -1, -1, 9, -1, -1, -1, -1 };//��ͨ�˲���
    Mat km;
    km = Mat(3, 3, CV_32FC1, high); //���쵥ͨ��������󣬽�ͼ��IplImage�ṹת��Ϊͼ������
    IplImage src2img=IplImage(srcImage);
	IplImage* filtimg = &src2img;

	filtdst = cvCreateImage(srcImage.size(), IPL_DEPTH_8U, 3);

	//cvFilter2D(filtimg, filtdst, &km, cvPoint(-1, -1));  //��ο���Ϊ�˵�����

	Mat tmpImage;
	tmpImage = cvarrToMat(filtimg);
	midImage = tmpImage;
	//imshow("��Ե��ȡ", tmpImage);
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

	//ͨ������任���ֱ��
	lines.clear();
	HoughLines(midImage, lines, 1, CV_PI / 180, 660, 0, 0);//��5������������ֵ����ֵԽ�󣬼�⾫��Խ��
														   //����ͼ��ͬ����ֵ�����趨����Ϊ��ֵ�趨���ߵ����޷����ֱ�ߣ���ֵ����ֱ��̫�࣬�ٶȺ���
														   //���Ը�����ֵ�ɴ�С������������ֵ�����������������󣬿��Թ̶�һ���ʺϵ���ֵ��
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
		//cout << "û�м�⵽ֱ�ߣ�" << endl;

	}
	float sum = 0;
	int linesizever = 0;
	//���λ���ÿ���߶�
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
		//ֻѡ�Ƕ���С����Ϊ��ת�Ƕ�
		if (theta < 2.156 &&theta>0.8853)
		{
			sum += theta;
			linesizever++;
		}

		//line(dstImage, pt1, pt2, Scalar(55, 100, 195), 1, CV_AA); //Scalar�������ڵ����߶���ɫ
	}
	//float average = sum / lines.size(); //�����нǶ���ƽ������������תЧ�������
	float average;
	if (linesizever > 0)
	{
		average = sum / linesizever; //�����нǶ���ƽ������������תЧ�������
									 //cout << "average theta:" << average << endl;
		double angle = DegreeTrans(average) - 90;
		degree = angle;
	}

}

JNIEXPORT jdouble JNICALL Java_com_JniDemo_ImageRecify
(JNIEnv *env, jclass cls, jstring SrcPath, jstring DstPath)
{
	const char *c_str = NULL;
	jboolean isCopy;	// ����JNI_TRUE��ʾԭ�ַ����Ŀ���������JNI_FALSE��ʾ����ԭ�ַ�����ָ��


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
	//��б�ǶȽ���

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

	Mat resultImage = dst(Rect(0, 0, source.cols, source.rows)); //��������֪ʶ�����ƺ��ı��ĳ����ٲü�����
	imwrite(dstpath, resultImage);

	const char*  ch;
	ch = dstpath.data();

	return degree;
}


//��ȡ�ı���ͶӰ�����ڷָ��ַ�(��ֱ��ˮƽ),Ĭ��ͼƬ�ǰ׵׺�ɫ
int GetTextProjection(Mat &src, vector<int>& pos, int mode)
{
	if (mode == V_PROJECT)
	{
		for (int i = 0; i < src.rows; i++)
		{
			uchar* p = src.ptr<uchar>(i);
			for (int j = 0; j < src.cols; j++)
			{
				if (p[j] == 0)  //�Ǻ�ɫ����
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


//��ȡÿ���ָ��ַ��ķ�Χ��min_thresh���������С���ȣ�min_range�������������С���
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
	//��ֵ��ͼ��
	Mat binary_img, morhp_img;
	cvtColor(roiImage, morhp_img, COLOR_BGR2GRAY);
	threshold(morhp_img, binary_img, 0, 255, THRESH_BINARY_INV | THRESH_OTSU);
	//imshow("��ֵ�����", binary_img);

	//���п��������
	Mat kernel = getStructuringElement(MORPH_RECT, Size(20, 1), Point(-1, -1));     //����һ����20����1��һ�����νṹԪ
	morphologyEx(binary_img, morhp_img, MORPH_OPEN, kernel, Point(-1, -1));     //�������㣬ֻ����ͼ���е�ֱ��

																				// �������Ͳ���
	kernel = getStructuringElement(MORPH_RECT, Size(3, 3), Point(-1, -1));      //����һ����3����3��һ�����νṹԪ
	dilate(morhp_img, morhp_img, kernel);                             //�����Ͳ�����ͼ���е�ֱ����ǿ
	dilate(morhp_img, morhp_img, kernel);                             //�����Ͳ�����ͼ���е�ֱ����ǿ
																	  // ����ֱ�߼��
	lines_l.clear();
	HoughLinesP(morhp_img, lines_l, 1, CV_PI / 180.0, 25.0, 20.0, 0);

	Mat result_img = src.clone();

	int LinesThr = 0;//�»��߳�����ֵ

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
	jboolean isCopy;	// ����JNI_TRUE��ʾԭ�ַ����Ŀ���������JNI_FALSE��ʾ����ԭ�ַ�����ָ��


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

	cvtColor(roiImage, roiImage, COLOR_BGR2GRAY);           //��ɫͼתΪ�Ҷ�ͼ

	threshold(roiImage, roiImage, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);

	vector<int> hpos(roiImage.rows, 0);
	vector<char_range_t> h_peek_range;

	//�����ı���ͶӰ
	GetTextProjection(roiImage, hpos, H_PROJECT);

	//��ȡ�����ı���ֹ������
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
