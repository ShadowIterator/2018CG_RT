#define _CRT_SECURE_NO_WARNINGS

#include"bmp.h"
#include<cstdio>
#include<fstream>
#include<iostream>
#include<string>
#include<cmath>

using namespace std;

Bmp::Bmp( int H , int W ) {
	Initialize( H , W );
}

Bmp::~Bmp() {
	Release();
}

void Bmp::Initialize( int H , int W ) 
{ }

void Bmp::Release() 
{
	for ( int i = 0 ; i < H ; i++ )
		delete[] ima[i];
	delete[] ima;
}

void Bmp::Input(std::string file)
{
	cout << "loading " << file << endl;

	cv::Mat timg = cv::imread(file, CV_LOAD_IMAGE_COLOR);
	H = timg.rows;
	W = timg.cols;

	cout << H << " " << W << endl;

	ima = new IMAGEDATA*[H];
	for ( int i = 0 ; i < H ;++i )
		ima[i] = new IMAGEDATA[W];

	for(int i=0;i<H;++i)
		for (int j = 0; j < W; ++j)
		{
			byte* bs = (byte*)&(timg.at<cv::Vec3b>(i, j));
			ima[i][j].blue = bs[0];
			ima[i][j].green = bs[1];
			ima[i][j].red = bs[2];
		}
}

void Bmp::Output(std::string file)
{
	cout<<"Output " << file << endl;
	cv::Mat timg(H, W, CV_8UC3, cv::Scalar(0, 0, 0));
	for (int i = 0; i<H; ++i)
		for (int j = 0; j < W; ++j)
		{
			byte* bs = &(timg.at<byte>(j, i * 3));
			bs[0] = ima[i][j].blue;
			bs[1] = ima[i][j].green;
			bs[2] = ima[i][j].red;

		}
	cv::imwrite(file, timg);
}

void Bmp::SetColor( int i , int j , Color col ) {
	ima[i][j].red = ( int ) ( col.r * 255 );
	ima[i][j].green = ( int ) ( col.g * 255 );
	ima[i][j].blue = ( int ) ( col.b * 255 );
}

Color Bmp::GetSmoothColor( double u , double v ) {
	double U = (u - floor(u)) * H;
	double V = (v - floor(v)) * W;
	int U1 = ( int ) floor( U + EPS ) , U2 = U1 + 1;
	int V1 = ( int ) floor( V + EPS ) , V2 = V1 + 1;
	double rat_U = U2 - U;
	double rat_V = V2 - V;
	if ( U1 < 0 ) U1 = H - 1; if ( U2 == H ) U2 = 0;
	if ( V1 < 0 ) V1 = W - 1; if ( V2 == W ) V2 = 0;
	U1 %= H;
	U2 %= H;
	V1 %= W;
	V2 %= W;

	Color ret;
	ret = ret + ima[U1][V1].GetColor() * rat_U * rat_V;
	ret = ret + ima[U1][V2].GetColor() * rat_U * ( 1 - rat_V );
	ret = ret + ima[U2][V1].GetColor() * ( 1 - rat_U ) * rat_V;
	ret = ret + ima[U2][V2].GetColor() * ( 1 - rat_U ) * ( 1 - rat_V );
	return ret;
}
