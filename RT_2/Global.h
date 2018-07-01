#ifndef Global_H
#define Global_H

#include<map>
#include<list>
#include<ctime>
#include<cmath>
#include<vector>
#include<string>
#include<cstdio>
#include<cstring>
#include<cstdlib>
#include<iostream>
#include<algorithm>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp>  


typedef unsigned char uchar;

typedef double DB;

const double EPS = 1e-6;
const double PI = 3.14159265358979323846;
const double SPEC_POWER = 20;
const int MAX_DREFL_DEP = 2;
const int MAX_RAYTRACING_DEP = 10;
const int HASH_FAC = 7;
const int HASH_MOD = 10000007;

inline char DBcmp(double a, double b)
{
	if (a - b > EPS) return 1;
	if (b - a > EPS) return -1;
	return 0;
}

#endif // !Global_H
