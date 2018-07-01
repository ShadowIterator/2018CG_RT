#include"primitive.h"
#include<sstream>
#include<cstdio>
#include<string>
#include<cmath>
#include<iostream>
#include<cstdlib>

Material::Material() {
	color = absor = Color();
	refl = refr = 0;
	diff = spec = 0;
	rindex = 0;
	drefl = 0;
	texture = NULL;
}

void Material::Input( std::string var , std::stringstream& fin ) {
	if ( var == "color=" ) color.Input( fin );
	if ( var == "absor=" ) absor.Input( fin );
	if ( var == "refl=" ) fin >> refl;
	if ( var == "refr=" ) fin >> refr;
	if ( var == "diff=" ) fin >> diff;
	if ( var == "spec=" ) fin >> spec;
	if ( var == "drefl=" ) fin >> drefl;
	if ( var == "rindex=" ) fin >> rindex;
	if ( var == "texture=" ) {
		std::string file; fin >> file;
		texture = new Bmp;
		texture->Input( file );
	}
}

Primitive::Primitive() {
	sample = rand();
	material = new Material;
	next = NULL;
}

Primitive::Primitive( const Primitive& primitive ) {
	*this = primitive;
	material = new Material;
	*material = *primitive.material;
}

Primitive::~Primitive() {
	delete material;
}

void Primitive::Input( std::string var , std::stringstream& fin ) {
	material->Input( var , fin );
}

Sphere::Sphere() : Primitive() {
	De = Vector3lf( 0 , 0 , 1 );
	Dc = Vector3lf( 0 , 1 , 0 );
}

void Sphere::Input( std::string var , std::stringstream& fin ) {
	if ( var == "O=" ) O.Input( fin );
	if ( var == "R=" ) fin >> R;
	if ( var == "De=" ) De.Input( fin );
	if ( var == "Dc=" ) Dc.Input( fin );
	Primitive::Input( var , fin );
}

bool Sphere::Collide( Vector3lf ray_O , Vector3lf ray_V )
{
	ray_V ._normalize();
	Vector3lf V = ray_O - O;
	double proj = -(V % ray_V );
	double det = proj * proj - V.len2() + R * R;

	if (DBcmp(det, 0) > 0)
	{
		det = sqrt( det );
		double x1 = proj - det  , x2 = proj + det;

		if ( x2 < EPS ) return false;
		if ( x1 > EPS ) 
		{
			crash.dist = x1;
			crash.front = true;
		} else 
		{
			crash.dist = x2;
			crash.front = false;
		} 
	} else 
		return false;

	crash.C = ray_O + ray_V * crash.dist;
	crash.N = ( crash.C - O ).normalize();
	if ( crash.front == false ) crash.N = -crash.N;
	return true;
}

Color Sphere::GetTexture() 
{
	Vector3lf I = ( crash.C - O ).normalize();
	double a = acos( -(I% De ) );
	double b = acos( std::min( std::max( (I% Dc ) / sin( a ) , -1.0 ) , 1.0 ) );
	double u = a / PI , v = b / 2 / PI;
	if ( I%( Dc * De ) < 0 ) v = 1 - v;
	return material->texture->GetSmoothColor( u , v );
}

Primitive* Sphere::PrimitiveCopy() {
	Sphere* ret = new Sphere( *this );
	return ret;
}

void Plane::Input( std::string var , std::stringstream& fin ) {
	if ( var == "N=" ) N.Input( fin );
	if ( var == "R=" ) fin >> R;
	if ( var == "Dx=" ) Dx.Input( fin );
	if ( var == "Dy=" ) Dy.Input( fin );
	Primitive::Input( var , fin );
}

bool Plane::Collide( Vector3lf ray_O , Vector3lf ray_V )
{
	ray_V._normalize();
	N._normalize();
	double d = N % ray_V ;
	if ( fabs( d ) < EPS ) return false;
	double l = ( N * R - ray_O ) % N  / d;
	if ( l < EPS ) return false;

	crash.dist = l;
	crash.front = ( d < 0 );
	crash.C = ray_O + ray_V * crash.dist;
	crash.N = ( crash.front ) ? N : -N;
	return true;
}

Color Plane::GetTexture() 
{
	double u = (crash.C % Dx ) / Dx.len2();
	double v = (crash.C % Dy ) / Dy.len2();
	return material->texture->GetSmoothColor( u , v );
}

Primitive* Plane::PrimitiveCopy() 
{
	Plane* ret = new Plane( *this );
	return ret;
}

void Triangle::Input(std::string var, std::stringstream& fin) 
{
	if (var == "P1=") P1.Input(fin);
	if (var == "P2=") P2.Input(fin);
	if (var == "P3=") P3.Input(fin);
	Primitive::Input(var, fin);
}

void Triangle::calcN()
{
	N = ((P2 - P1) * (P3 - P1)).normalize();
}

double Abs(double x)
{
	return DBcmp(x, 0) < 0 ? -x : x;
}

bool Triangle::Collide(Vector3lf ray_O, Vector3lf ray_V) 
{
	ray_V._normalize();

	double cosTheta = -(N % ray_V);
	if (DBcmp(cosTheta, 0) == 0) return false;
	double d = fabs((P1 - ray_O) % N);
	double l;
	Point3lf C;
	if (DBcmp(d, 0) == 0)
	{
		return false;
	}
	else
	{
		l = d / cosTheta;
		C = ray_O + (ray_V * l);
	}
	Vector3lf V1 = (P1 - C) * (P2 - C);
	Vector3lf V2 = (P2 - C) * (P3 - C);
	Vector3lf V3 = (P3 - C) * (P1 - C);
	if (V1 % V2 < -EPS || V2 % V3 < -EPS) return false;

	crash.dist = l;
	crash.front = (cosTheta > 0);
	crash.C = C;
	crash.N = (crash.front) ? N : -N;
	return true;

}

Primitive* Triangle::PrimitiveCopy() {
	Triangle* ret = new Triangle(*this);
	return ret;
}

Color Triangle::GetTexture() 
{
	return Color(1, 1, 1);
}
