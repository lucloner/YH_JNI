#define _CRT_SECURE_NO_WARNINGS
#include"com_IdentityCard.h"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
//#include "cv.h"
//#include "highgui.h"
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
#include <fstream>  
#include <string>  
#include <iostream>
#include <opencv2\imgproc\types_c.h>
#include <com_IdentityCard.h>


//#pragma warning(disable:4113)
//
//jmp_buf mark;
//int fperr;
//void __cdecl  fphandler(int num);
//void fpcheck(void);


using namespace cv;
using namespace std;


jstring str2jstring(JNIEnv* env, const char* pat)
{
    jstring result=(jstring)"";
	//定义java String类 strClass
	jclass strClass = (env)->FindClass("Ljava/lang/String;");
	//获取String(byte[],String)的构造器,用于将本地byte[]数组转换为一个新String
	jmethodID ctorID = (env)->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
	//建立byte数组
	jbyteArray bytes = (env)->NewByteArray(strlen(pat));
	//将char* 转换为byte数组
	(env)->SetByteArrayRegion(bytes, 0, strlen(pat), (jbyte*)pat);
	// 设置String, 保存语言类型,用于byte数组转换至String时的参数
	jstring encoding = (env)->NewStringUTF("GB2312");
	//将byte数组转换为java String,并输出
	result=(jstring)(env)->NewObject(strClass, ctorID, bytes, encoding);
	return result;
}


std::string jstring2str(JNIEnv* env, jstring jstr)
{
	char*   rtn = NULL;
	jclass   clsstring = env->FindClass("java/lang/String");
	jstring   strencode = env->NewStringUTF("GB2312");
	jmethodID   mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");
	jbyteArray   barr = (jbyteArray)env->CallObjectMethod(jstr, mid, strencode);
	jsize   alen = env->GetArrayLength(barr);
	jbyte*   ba = env->GetByteArrayElements(barr, JNI_FALSE);
	if (alen   >   0)
	{
		rtn = (char*)malloc(alen + 1);
		memcpy(rtn, ba, alen);
		rtn[alen] = 0;
	}
	env->ReleaseByteArrayElements(barr, ba, 0);
	std::string stemp(rtn);
	free(rtn);
	return   stemp;
}
std::string toStr(JNIEnv* env, const char* chs) { std::string s(chs); return s; }
std::string toStr(JNIEnv* env, const jstring& jstr) { return toStr(env, env->GetStringUTFChars(jstr, 0)); }



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


JNIEXPORT void JNICALL Java_com_IdentityCard_IdentityCardTrans
(JNIEnv *env, jclass cls, jstring SrcPath, jstring DstPath)
{
	//string srcdir = toStr(env, env->GetStringUTFChars(SrcPath, 0));
	//string dstdir = toStr(env, env->GetStringUTFChars(DstPath, 0));
	cout << SrcPath << DstPath << endl;

	string srcpath = toStr(env, env->GetStringUTFChars(SrcPath, 0));
	string dstpath = toStr(env, env->GetStringUTFChars(DstPath, 0));


	//ofstream out1("e:\\srcpath.txt");
	//out1 << srcdir << endl;
	//out1.close();
	////srcpath = "E:/测试/image0000041A.jpg";
	////dstpath = "E:/测试/image0000041Ares.jpg";
	//ofstream out2("e:\\dstpath.txt");
	//out2 << dstdir << endl;
	//out2.close();


	//string srcpath = "";
	//string dstpath = "";
	//ifstream in1("e:\\srcpath.txt");
	//getline(in1,srcpath);
	//out << srcpath << endl;
    //in1.close();

	//ifstream in2("e:\\dstpath.txt");
	//getline(in2, dstpath);
	//out << srcpath << endl;
	//in2.close();
	//ofstream out1("e:\\1.txt");
	//out1 << srcpath << endl;
	//string str = "he is@ a@ good boy";
	//srcpath = srcpath.replace(srcpath.find("//"), 1, "/");  //从第一个a位置开始的两个字符替换成#
	//cout << str << endl;
	//ofstream out2("e:\\2.txt");
	//out2 << dstpath << endl;

	//ifstream in1("e:\\srcpath.txt");
	//in1 >> srcpath;
	//ifstream in2("e:\\dstpath.txt");
	//in1 >> dstpath;
	//srcpath = "E:\测试\image0000041A.jpg";
	//srcpath = srcpath.replace;
   // setlocale(LC_ALL, "Chinese-simplified");//设置中文环境
	Mat srcImage = imread(srcpath);

	//printf("1");
	if (srcImage.cols > srcImage.rows)
	{
		transpose(srcImage, srcImage);
		flip(srcImage, srcImage, 1);
	}
	//setlocale(LC_ALL, "C");//还原
	//printf("2");
	Mat GrayImage;
	Mat BinaryImage;
	Mat dst;
	//创建与原图同类型和同大小的矩阵
	GrayImage.create(srcImage.size(), srcImage.type());
	dst.create(srcImage.size(), srcImage.type());
	//将原图转换为灰度图像
	//printf("3");
	cvtColor(srcImage, GrayImage, CV_BGR2GRAY);

	threshold(GrayImage, BinaryImage, 180, 255, CV_THRESH_BINARY);
	//printf("4");
	int height = srcImage.rows;//获取RGB图像的行
	int width = srcImage.cols;//获取RGB图像的列
	int blue,green,red;
	for (int row = 0; row < height; row++) {
		for (int col = 0; col < width; col++) {
			//获取RGB图像各通道像素值
			blue = srcImage.at<Vec3b>(row, col)[0];    //获取b通道像素值
			green = srcImage.at<Vec3b>(row, col)[1];    //获取g通道像素值
			red = srcImage.at<Vec3b>(row, col)[2];    //获取r通道像素值

			int tmp = BinaryImage.at<uchar>(row, col);

			if (tmp<180)
			{
				dst.at<Vec3b>(row, col)[0] = blue;
				dst.at<Vec3b>(row, col)[1] = green;
				dst.at<Vec3b>(row, col)[2] = red;
			}
			else
			{
				dst.at<Vec3b>(row, col)[0] = 255;
				dst.at<Vec3b>(row, col)[1] = 255;
				dst.at<Vec3b>(row, col)[2] = 255;
			}
		}
	}
	//printf("5");
	imwrite(dstpath, dst);
	//printf("6");
	const char*  ch;
	ch = dstpath.data();
	SetResolution(ch, 300);


}


