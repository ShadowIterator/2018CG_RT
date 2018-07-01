#include"light.h"
#include<sstream>
#include<string>
#include<cmath>
#include<cstdlib>
#define ran() ( double( rand() % 32768 ) / 32768 )

Light::Light() {
	sample = rand();
	next = NULL;
}

void Light::Input( std::string var , std::stringstream& fin ) {
	if ( var == "color=" ) color.Input( fin );
}

void AreaLight::Input( std::string var , std::stringstream& fin ) 
{
	if ( var == "O=" ) O.Input( fin );
	if ( var == "Dx=" ) Dx.Input( fin );
	if ( var == "Dy=" ) Dy.Input( fin );
	Light::Input( var , fin );
}

bool AreaLight::Collide( Vector3lf ray_O , Vector3lf ray_V ) 
{
	ray_V._normalize();
	Vector3lf N = ( Dx * Dy ).normalize();
	double d = N % ray_V;
	if ( fabs( d ) < EPS ) return false;
	double l = ( N * ( O % N ) - ray_O ) % N / d;
	if ( l < EPS ) return false;

	Vector3lf C = ( ray_O + ray_V * l ) - O;
	if ( fabs( Dx % C ) > Dx % Dx ) return false;
	if ( fabs( Dy % C ) > Dy % Dy ) return false;

	crash_dist = l;
	return true;
}

double AreaLight::CalnShade( Vector3lf C , Primitive* primitive_head , int shade_quality ) 
{
	int shade = 0;
	
	for ( int i = -2 ; i < 2 ; i++ )
		for ( int j = -2 ; j < 2 ; j++ )
			for ( int k = 0 ; k < shade_quality ; k++ )
			{
				Vector3lf V = O - C + Dx * ( ( ran() + i ) / 2 ) + Dy * ( ( ran() + j ) / 2 );
				double dist = V.len();

				for ( Primitive* now = primitive_head ; now != NULL ; now = now->GetNext() )
					if ( now->Collide( C , V ) && ( now->crash.dist < dist ) )
					{
						shade++;
						break;
					}
			}
	
	return 1 - ( double ) shade / ( 16.0 * shade_quality );
}
