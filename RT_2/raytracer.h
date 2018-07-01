#ifndef RAYTRACER_H
#define RAYTRACER_H

#include "Global.h"
#include"scene.h"
#include"bmp.h"
#include<string>

extern const double SPEC_POWER;
extern const int MAX_DREFL_DEP;
extern const int MAX_RAYTRACING_DEP;
extern const int HASH_FAC;
extern const int HASH_MOD;

class Raytracer {
	std::string input , output;
	Scene scene;
	Color CalnDiffusion( Primitive* pri , int* hash );	//¬˛∑¥…‰
	Color CalnReflection( Primitive* pri , Vector3lf ray_V , int dep , int* hash );	//∑¥…‰
	Color CalnRefraction( Primitive* pri , Vector3lf ray_V , int dep , int* hash );	//’€…‰
	Color RayTracing( Vector3lf ray_O , Vector3lf ray_V , int dep , int* hash );

public:
	Raytracer() {}
	~Raytracer() {}
	
	void SetInput( std::string file ) { input = file; }
	void SetOutput( std::string file ) { output = file; }
	void Run();
};

#endif
