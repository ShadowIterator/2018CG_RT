#ifndef BMP_H
#define BMP_H

#include "Global.h"
#include"color.h"
#include<string>

extern const double EPS;

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned int dword;

struct IMAGEDATA {
	byte blue;
	byte green;
	byte red;
	Color GetColor() {
		return Color( red , green , blue ) / 256;
	}
};

class Bmp {
	int H, W;
	bool ima_created;
	IMAGEDATA** ima;

	void Release();
	
public:
	Bmp( int H = 0 , int W = 0 );
	~Bmp();

	int GetH() { return H; } 
	int GetW() { return W; }
	Color GetColor( int i , int j ) { return Color( ima[i][j].red , ima[i][j].green , ima[i][j].blue ) / 256; }
	void SetColor( int i , int j , Color );

	void Initialize( int H , int W );
	void Input( std::string file );
	void Output( std::string file );
	Color GetSmoothColor( double u , double v );
};

#endif
