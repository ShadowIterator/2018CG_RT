#ifndef LIGHT_H
#define LIGHT_H

#include "Global.h"
#include"vector3.hpp"
#include"color.h"
#include"primitive.h"
#include<sstream>
#include<string>
#include<cmath>

extern const double EPS;

class Light {
protected:
	int sample;
	Color color;
	Light* next;

public:
	double crash_dist;

	Light();
	~Light() {}
	
	virtual bool IsPointLight() = 0;
	int GetSample() { return sample; }
	Color GetColor() { return color; }
	Light* GetNext() { return next; }
	void SetNext( Light* light ) { next = light; }

	virtual void Input( std::string , std::stringstream& );
	virtual Vector3lf GetO() = 0;
	virtual bool Collide( Vector3lf ray_O , Vector3lf ray_V ) = 0;
	virtual double CalnShade( Vector3lf C , Primitive* primitive_head , int shade_quality ) = 0;	//“ı”∞
};

class AreaLight : public Light {
	Vector3lf O , Dx , Dy;
public:
	AreaLight() : Light() {}
	~AreaLight() {}
	
	bool IsPointLight() { return false; }
	Vector3lf GetO() { return O; }
	void Input( std::string , std::stringstream& );
	bool Collide( Vector3lf ray_O , Vector3lf ray_V );
	double CalnShade( Vector3lf C , Primitive* primitive_head , int shade_quality );
};

#endif
