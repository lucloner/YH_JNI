#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <vector>
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc_c.h"

#include "com_RemoveBlackBorderDll.h"
#include <fcntl.h>
#if defined(__linux__) || defined(__linux)
#  include <sys/io.h>
#  include <opencv2/photo/photo.hpp>
#else
#  include <io.h>
#  include <opencv2/photo.hpp>
#endif

using namespace cv;
using namespace std;


#define V_PROJECT 1  //垂直投影（vertical）
#define H_PROJECT 2  //水平投影（horizational）

typedef struct
{
	int begin;
	int end;

}char_range_t;
