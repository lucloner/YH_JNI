#include "RemoveBlackBorder.h"

using namespace cv;
using namespace std;

JNIEXPORT void JNICALL Java_com_RemoveBlackBorderDll_RemoveBlackBorder
(JNIEnv *env, jclass cls, jstring SrcPath, jstring DstPath)
{
	const char *c_str = NULL;
	jboolean isCopy;	// 返回JNI_TRUE表示原字符串的拷贝，返回JNI_FALSE表示返回原字符串的指针

	c_str = env->GetStringUTFChars(SrcPath, &isCopy);
	string srcpath = c_str;

	c_str = env->GetStringUTFChars(DstPath, &isCopy);
	string dstpath = c_str;

	Mat sourceImage=imread(srcpath);
	Scalar color;

	double max = 0;                                 //最亮的像素特征值

	int xstart = 0, ystart = 0, xend = 0, yend = 0; //修复区域的起始和终止点坐标
	int xlength = 0, ylength = 0;                   //修复区域长宽
	int cols = 0, rows = 0;


	//1.左边缘
	for (int row = 0; row < sourceImage.rows; row++)
	{
		int a = sourceImage.at<Vec3b>(row, 0)(0);
		int b = sourceImage.at<Vec3b>(row, 0)(1);
		int c = sourceImage.at<Vec3b>(row, 0)(2);
		double tmp = pow(double(a), 2) + pow(double(b), 2) + pow(double(c), 2);

		if (tmp>max)
		{
			max = pow(double(a), 2) + pow(double(b), 2) + pow(double(c), 2);
			//color = Vec3b(src.at<Vec3b>(row, 0)(0), src.at<Vec3b>(row, 0)(1), src.at<Vec3b>(row, 0)(2));
			color(0) = sourceImage.at<Vec3b>(row, 0)(0);
			color(1) = sourceImage.at<Vec3b>(row, 0)(1);
			color(2) = sourceImage.at<Vec3b>(row, 0)(2);

		}
	}

	xstart = 0;
	xend = 0;
	ystart = sourceImage.rows - 1;
	cols = 0;
	for (int row = 0; row < sourceImage.rows; row++)
	{
		//if (sourceImage.at<Vec3b>(row, 0) == Vec3b(0, 0, 0))
		if (sourceImage.at<Vec3b>(row, 0)(0)<120 || sourceImage.at<Vec3b>(row, 0)(1)<120 || sourceImage.at<Vec3b>(row, 0)(2)<120)
		{
			if (row < ystart)
			{
				ystart = row;

			}
			for (int col = 0; col < sourceImage.cols; col++)
			{
				if (sourceImage.at<Vec3b>(row, col) == Vec3b(0, 0, 0))
				{
					sourceImage.at<Vec3b>(row, col) = Vec3b(color(0), color(1), color(2));

				}
				else if (sourceImage.at<Vec3b>(row, col)(0)>120 || sourceImage.at<Vec3b>(row, col)(1)>120 || sourceImage.at<Vec3b>(row, col)(2)>120)
				{
					Scalar color1 = sourceImage.at<Vec3b>(row, col);

					if (col > xend)
					{
						xend = col;
					}
					cols = col;
					break;
				}
				else
				{
					sourceImage.at<Vec3b>(row, col) = Vec3b(color(0), color(1), color(2));

				}
				if (row > yend)
				{
					yend = row;
				}
			}
			if (cols<sourceImage.cols - 20)
			{
				for (int col = cols; col < cols + 20; col++)
				{
					sourceImage.at<Vec3b>(row, col) = Vec3b(color(0), color(1), color(2));
				}
			}
		}
		//imwrite("e:\\scalar.jpg", sourceImage);
	}

	if (yend > ystart&&xend > xstart)
	{
		if (xend + 20 < sourceImage.cols)
		{
			xlength = xend - xstart + 20;
		}
		else
		{
			xlength = sourceImage.cols - xstart;
		}

		if (yend + 20 < sourceImage.rows)
		{
			ylength = yend - ystart + 20;
		}
		else
		{
			ylength = sourceImage.rows - ystart;
		}

		if (ylength > 100)
		{
			int xstart_tmp;
			int xlength_tmp;
			if (xlength > 100)
			{
				xlength_tmp = 100;
			}
			else
			{
				xlength_tmp = xlength;
			}


			Mat srcROT1(sourceImage, Rect(xstart, ystart, xlength_tmp, ylength));  //Mat srcROI = img(Rect(0,0,rows/2,cols/2));
			blur(srcROT1, srcROT1, Size(81, 81));

			ylength = 100;
		}
		Mat srcROT1(sourceImage, Rect(xstart, ystart, xlength, ylength));  //Mat srcROI = img(Rect(0,0,rows/2,cols/2));
		blur(srcROT1, srcROT1, Size(81, 81));
	}

	//2.右边缘
	for (int row = 0; row < sourceImage.rows; row++)
	{
		int a = sourceImage.at<Vec3b>(row, sourceImage.cols - 1)(0);
		int b = sourceImage.at<Vec3b>(row, sourceImage.cols - 1)(1);
		int c = sourceImage.at<Vec3b>(row, sourceImage.cols - 1)(2);
		double tmp = pow(double(a), 2) + pow(double(b), 2) + pow(double(c), 2);

		if (tmp>max)
		{
			max = pow(double(a), 2) + pow(double(b), 2) + pow(double(c), 2);
			//color = Vec3b(src.at<Vec3b>(row, 0)(0), src.at<Vec3b>(row, 0)(1), src.at<Vec3b>(row, 0)(2));
			color(0) = sourceImage.at<Vec3b>(row, sourceImage.cols - 1)(0);
			color(1) = sourceImage.at<Vec3b>(row, sourceImage.cols - 1)(1);
			color(2) = sourceImage.at<Vec3b>(row, sourceImage.cols - 1)(2);

		}
	}

	xend = sourceImage.cols - 1;
	xstart = sourceImage.cols - 1;
	ystart = sourceImage.rows - 1;
	cols = 0;
	for (int row = 0; row < sourceImage.rows; row++)
	{
		//if (sourceImage.at<Vec3b>(row, 0) == Vec3b(0, 0, 0))
		if (sourceImage.at<Vec3b>(row, sourceImage.cols - 1)(0)<120 || sourceImage.at<Vec3b>(row, sourceImage.cols - 1)(1)<120 || sourceImage.at<Vec3b>(row, sourceImage.cols - 1)(2)<120)
		{
			if (row < ystart)
			{
				ystart = row;

			}
			int count = 0;
			for (int col = sourceImage.cols - 1; col >0; col--)
			{
				if (sourceImage.at<Vec3b>(row, col) == Vec3b(0, 0, 0))
				{
					sourceImage.at<Vec3b>(row, col) = Vec3b(color(0), color(1), color(2));
					count++;
				}
				else if (sourceImage.at<Vec3b>(row, col)(0)>120 || sourceImage.at<Vec3b>(row, col)(1)>120 || sourceImage.at<Vec3b>(row, col)(2)>120)
				{
					Scalar color1 = sourceImage.at<Vec3b>(row, col);

					if (col < xstart)
					{
						xstart = col;
					}
					cols = col;
					break;
				}
				else
				{
					sourceImage.at<Vec3b>(row, col) = Vec3b(color(0), color(1), color(2));
					count++;
				}

				if (row > yend)
				{
					yend = row;
				}
			}
			if (cols > 20)
			{
				for (int col = cols; col > cols - 20; col--)
				{
					sourceImage.at<Vec3b>(row, col) = Vec3b(color(0), color(1), color(2));
				}
			}

		}
	}

	if (yend > ystart&&xend > xstart)
	{
		if (xstart - 20 >0)
		{
			xlength = xend - xstart + 20;
			xstart = xstart - 20;
		}
		else
		{
			xlength = sourceImage.cols - xstart;
		}

		if (yend + 20 < sourceImage.rows)
		{
			ylength = yend - ystart + 20;
		}
		else
		{
			ylength = sourceImage.rows - ystart;
		}

		if (ylength > 100)
		{
			int xstart_tmp;
			int xlength_tmp;
			if (xlength > 100)
			{
				xstart_tmp = sourceImage.cols - 100;
				xlength_tmp = 100;
			}
			else
			{
				xstart_tmp = xstart;
				xlength_tmp = xlength;
			}
			Mat srcROT1(sourceImage, Rect(xstart_tmp, ystart, xlength_tmp, ylength));  
			blur(srcROT1, srcROT1, Size(81, 81));

			ylength = 100;
		}
		Mat srcROT(sourceImage, Rect(xstart, ystart, xlength, ylength)); 
		blur(srcROT, srcROT, Size(81, 81));
	}

	//3.上边缘
	for (int col = 0; col < sourceImage.cols; col++)
	{
		int a = sourceImage.at<Vec3b>(0, col)(0);
		int b = sourceImage.at<Vec3b>(0, col)(1);
		int c = sourceImage.at<Vec3b>(0, col)(2);
		double tmp = pow(double(a), 2) + pow(double(b), 2) + pow(double(c), 2);

		if (tmp>max)
		{
			max = pow(double(a), 2) + pow(double(b), 2) + pow(double(c), 2);
			//color = Vec3b(src.at<Vec3b>(row, 0)(0), src.at<Vec3b>(row, 0)(1), src.at<Vec3b>(row, 0)(2));
			color(0) = sourceImage.at<Vec3b>(0, col)(0);
			color(1) = sourceImage.at<Vec3b>(0, col)(1);
			color(2) = sourceImage.at<Vec3b>(0, col)(2);

		}
	}
	xstart = sourceImage.cols - 1;
	xend = 0;
	ystart = 0;
	yend = 0;
	for (int col = 0; col < sourceImage.cols; col++)
	{
		//if (sourceImage.at<Vec3b>(row, 0) == Vec3b(0, 0, 0))
		if (sourceImage.at<Vec3b>(0, col)(0)<120 || sourceImage.at<Vec3b>(0, col)(1)<120 || sourceImage.at<Vec3b>(0, col)(2)<120)
		{
			if (col < xstart)
			{
				xstart = col;
			}
			int count = 0;
			for (int row = 0; row <sourceImage.rows - 1; row++)
			{
				if (sourceImage.at<Vec3b>(row, col) == Vec3b(0, 0, 0))
				{
					sourceImage.at<Vec3b>(row, col) = Vec3b(color(0), color(1), color(2));
					count++;
				}
				else if (sourceImage.at<Vec3b>(row, col)(0)>120 || sourceImage.at<Vec3b>(row, col)(1)>120 || sourceImage.at<Vec3b>(row, col)(2)>120)
				{
					Scalar color1 = sourceImage.at<Vec3b>(row, col);

					if (row > yend)
					{
						yend = row;
					}
					rows = row;
					break;
				}
				else
				{
					sourceImage.at<Vec3b>(row, col) = Vec3b(color(0), color(1), color(2));
					count++;
				}

				if (col > xend)
				{
					xend = col;
				}
			}
			if (rows <sourceImage.rows - 20)
			{
				for (int row = 0; row <rows + 20; row++)
				{
					sourceImage.at<Vec3b>(row, col) = Vec3b(color(0), color(1), color(2));
				}
			}
		}
	}
	if (yend > ystart&&xend > xstart)
	{
		if (yend <sourceImage.rows - 20)
		{
			ylength = yend - ystart + 20;
		}
		else
		{
			ylength = yend - ystart;
		}

		if (xend + 20 < sourceImage.cols)
		{
			xlength = xend - xstart + 20;
		}
		else
		{
			xlength = xend - xstart;
		}

		if (xlength > 100)
		{
			int ystart_tmp;
			int ylength_tmp;
			if (ylength > 100)
			{

				ylength_tmp = 100;
			}
			else
			{
				ylength_tmp = ylength;
			}
			Mat srcROT1(sourceImage, Rect(xstart, ystart, xlength, ylength_tmp));  //Mat srcROI = img(Rect(0,0,rows/2,cols/2));
			blur(srcROT1, srcROT1, Size(81, 81));

			xlength = 100;

		}

		Mat srcROT(sourceImage, Rect(xstart, ystart, xlength, ylength));  //Mat srcROI = img(Rect(0,0,rows/2,cols/2));
		blur(srcROT, srcROT, Size(81, 81));
	}

	//4.下边缘
	for (int col = 0; col < sourceImage.cols; col++)
	{
		int a = sourceImage.at<Vec3b>(sourceImage.rows - 1, col)(0);
		int b = sourceImage.at<Vec3b>(sourceImage.rows - 1, col)(1);
		int c = sourceImage.at<Vec3b>(sourceImage.rows - 1, col)(2);
		double tmp = pow(double(a), 2) + pow(double(b), 2) + pow(double(c), 2);

		if (tmp>max)
		{
			max = pow(double(a), 2) + pow(double(b), 2) + pow(double(c), 2);
			//color = Vec3b(src.at<Vec3b>(row, 0)(0), src.at<Vec3b>(row, 0)(1), src.at<Vec3b>(row, 0)(2));
			color(0) = sourceImage.at<Vec3b>(sourceImage.rows - 1, col)(0);
			color(1) = sourceImage.at<Vec3b>(sourceImage.rows - 1, col)(1);
			color(2) = sourceImage.at<Vec3b>(sourceImage.rows - 1, col)(2);

		}
	}
	xstart = 0;
	xend = 0;
	yend = sourceImage.rows - 1;
	ystart = sourceImage.rows - 1;
	for (int col = 0; col < sourceImage.cols; col++)
	{
		//if (sourceImage.at<Vec3b>(row, 0) == Vec3b(0, 0, 0))
		if (sourceImage.at<Vec3b>(0, col)(0)<120 || sourceImage.at<Vec3b>(0, col)(1)<120 || sourceImage.at<Vec3b>(0, col)(2)<120)
		{
			if (col <xstart)
			{
				xstart = col;
			}
			int count = 0;
			for (int row = sourceImage.rows - 1; row >0; row--)
			{
				if (sourceImage.at<Vec3b>(row, col) == Vec3b(0, 0, 0))
				{
					sourceImage.at<Vec3b>(row, col) = Vec3b(color(0), color(1), color(2));
					count++;
				}
				else if (sourceImage.at<Vec3b>(row, col)(0)>120 || sourceImage.at<Vec3b>(row, col)(1)>120 || sourceImage.at<Vec3b>(row, col)(2)>120)
				{
					Scalar color1 = sourceImage.at<Vec3b>(row, col);

					if (row <ystart)
					{
						ystart = row;
					}
					rows = row;
					break;
				}
				else
				{
					sourceImage.at<Vec3b>(row, col) = Vec3b(color(0), color(1), color(2));
					count++;
				}

				if (col > xend)
				{
					xend = col;
				}
			}
			if (rows <sourceImage.rows - 20)
			{
				for (int row = 0; row <rows + 20; row++)
				{
					sourceImage.at<Vec3b>(row, col) = Vec3b(color(0), color(1), color(2));
				}
			}
		}
	}
	if (yend > ystart&&xend > xstart)
	{
		if (yend <sourceImage.rows - 20)
		{
			ylength = yend - ystart + 20;
		}
		else
		{
			ylength = yend - ystart;
		}

		if (xend + 20 < sourceImage.cols)
		{
			xlength = xend - xstart + 20;
		}
		else
		{
			xlength = xend - xstart;
		}

		if (xend < sourceImage.rows - 20)
		{
			ystart = ystart - 20;
		}

		if (xlength > 100)
		{
			int ystart_tmp;
			int ylength_tmp;
			if (ylength > 100)
			{
				ystart_tmp = sourceImage.rows - 100;
				ylength_tmp = 100;
			}
			else
			{
				ystart_tmp = ystart;
				ylength_tmp = ylength;
			}
			Mat srcROT1(sourceImage, Rect(xstart, ystart_tmp, xlength, ylength_tmp));  //Mat srcROI = img(Rect(0,0,rows/2,cols/2));
			blur(srcROT1, srcROT1, Size(81, 81));

			xlength = 100;

		}

		Mat srcROT(sourceImage, Rect(xstart, ystart, xlength, ylength));  //Mat srcROI = img(Rect(0,0,rows/2,cols/2));
		blur(srcROT, srcROT, Size(81, 81));
	}

	Mat resultImage=sourceImage;
	imwrite(dstpath, resultImage);

	const char*  ch;
	ch = dstpath.data();
}
