#include"raytracer.h"
#include<cstdlib>
#include<iostream>

using std::cout;
using std::endl;

inline double randD()
{
	return double(rand()) / 32768;
}

Color Raytracer::CalnDiffusion( Primitive* pri , int* hash ) 
{
	Color clr = pri->GetMaterial()->color;
	if ( pri->GetMaterial()->texture) clr = clr * pri->GetTexture();
	
	Color ret = clr * scene.GetBackgroundColor() * pri->GetMaterial()->diff;

	for ( Light* light = scene.GetLightHead() ; light != NULL ; light = light->GetNext() ) 
	{
		double shade = light->CalnShade( pri->crash.C , scene.GetPrimitiveHead() , scene.GetCamera()->GetShadeQuality() );
		if ( shade < EPS ) continue;
		
		Vector3lf CO = ( light->GetO() - pri->crash.C ).normalize();
		double proj = CO % pri->crash.N ;
		if ( proj > EPS ) 
		{
			if ( hash != NULL && light->IsPointLight() ) *hash = ( *hash + light->GetSample() ) & HASH_MOD;

			if ( pri->GetMaterial()->diff > EPS ) 
			{
				double diff = pri->GetMaterial()->diff * proj * shade;
				ret += clr * light->GetColor() * diff;
			}
			if ( pri->GetMaterial()->spec > EPS )
			{
				double spec = pri->GetMaterial()->spec * pow( proj , SPEC_POWER ) * shade;
				ret += clr * light->GetColor() * spec;
			}
		}
	}

	return ret;
}

Color Raytracer::CalnReflection( Primitive* pri , Vector3lf ray_V , int dep , int* hash ) 
{
	ray_V = ray_V.Reflect( pri->crash.N );

	if ( pri->GetMaterial()->drefl < EPS || dep > MAX_DREFL_DEP )
		return RayTracing( pri->crash.C , ray_V , dep + 1 , hash ) * pri->GetMaterial()->color * pri->GetMaterial()->refl;

	Vector3lf Dx = ray_V * Vector3lf( 1 , 0 , 0 );
	if ( Dx.IsZeroVector() ) Dx = Vector3lf( 1 , 0 , 0 );
	Vector3lf Dy = ray_V * Dx;
	Dx = Dx.normalize() * pri->GetMaterial()->drefl;
	Dy = Dy.normalize() * pri->GetMaterial()->drefl;

	Color ret;
	for ( int k = 0 ; k < 16 * scene.GetCamera()->GetDreflQuality() ; k++ )
	{
		double x , y;
		do
		{
			x = randD() * 2 - 1;
			y = randD() * 2 - 1;
		} while ( x * x + y * y > 1 );
		x *= pri->GetMaterial()->drefl;
		y *= pri->GetMaterial()->drefl;
		ret += RayTracing( pri->crash.C , ray_V + Dx * x + Dy * y , dep + MAX_DREFL_DEP , NULL );
	}
	ret = ret * pri->GetMaterial()->color * pri->GetMaterial()->refl / ( 16 * scene.GetCamera()->GetDreflQuality() );
	return ret;
}

Color Raytracer::CalnRefraction( Primitive* pri , Vector3lf ray_V , int dep , int* hash ) {
	double n = pri->GetMaterial()->rindex;
	if ( pri->crash.front ) n = 1 / n;

	ray_V = ray_V.Refract( pri->crash.N , n );
	Color rcol = RayTracing( pri->crash.C , ray_V , dep + 1 , hash );

	if ( pri->crash.front ) return rcol * pri->GetMaterial()->refr;

	Color absor = pri->GetMaterial()->absor * -pri->crash.dist;
	Color trans = Color( exp( absor.r ) , exp( absor.g ) , exp( absor.b ) );

	return rcol * trans * pri->GetMaterial()->refr;
}

Color Raytracer::RayTracing( Vector3lf ray_O , Vector3lf ray_V , int depth , int* hash ) 
{
	if ( depth > MAX_RAYTRACING_DEP ) return Color();

	Color ret;
	Primitive* nearest_primitive = scene.FindNearestPrimitive( ray_O , ray_V );
	Light* nearest_light = scene.FindNearestLight( ray_O , ray_V );

	if ( nearest_light != NULL && ( nearest_primitive == NULL || nearest_light->crash_dist < nearest_primitive->crash.dist ) ) 
	{
		if ( hash != NULL ) *hash = ( *hash + nearest_light->GetSample() ) % HASH_MOD;
		ret += nearest_light->GetColor();
	}
	
	if ( nearest_primitive != NULL ) 
	{
		if ( hash != NULL ) *hash = ( *hash + nearest_primitive->GetSample() ) % HASH_MOD;
		Primitive* primitive = nearest_primitive->PrimitiveCopy();

		if ( primitive->GetMaterial()->diff > EPS || primitive->GetMaterial()->spec > EPS ) 
			ret += CalnDiffusion( primitive , hash );
		if ( primitive->GetMaterial()->refl > EPS ) ret += CalnReflection( primitive , ray_V , depth , hash );
		if ( primitive->GetMaterial()->refr > EPS ) ret += CalnRefraction( primitive , ray_V , depth , hash );
		delete primitive; 
	}

	if ( hash != NULL ) *hash = ( *hash * HASH_FAC ) % HASH_MOD;
	ret.Confine();
	return ret;
}

void Raytracer::Run() {
	Camera* camera = scene.GetCamera();
	scene.CreateScene( input );


	Vector3lf ray_O = camera->GetO();
	int H = camera->GetH() , W = camera->GetW();
	int** sample = new int*[H];
	for ( int i = 0 ; i < H ; i++ ) 
	{
		sample[i] = new int[W];
		for ( int j = 0 ; j < W ; j++ )
			sample[i][j] = 0;
	}
	
	for (int i = 0; i < H; ++i)
	{
		std::cout << "Sampling:   " << i + 1 << "/" << H << std::endl;
		for (int j = 0; j < W; j++) 
		{
			Vector3lf ray_V = camera->Emit(i, j);
			Color color = RayTracing(ray_O, ray_V, 1, &sample[i][j]);
			camera->SetColor(i, j, color);
		}
	}

	for (int i = 0; i < H; std::cout << "Resampling: " << ++i << "/" << H << std::endl)
	{
		for (int j = 0; j < W; j++) 
		{
			if ((i == 0 || sample[i][j] == sample[i - 1][j]) && (i == H - 1 || sample[i][j] == sample[i + 1][j]) &&
				(j == 0 || sample[i][j] == sample[i][j - 1]) && (j == W - 1 || sample[i][j] == sample[i][j + 1])) continue;

			Color color;
			for (int r = -1; r <= 1; r++)
				for (int c = -1; c <= 1; c++) 
				{
					Vector3lf ray_V = camera->Emit(i + (double)r / 3, j + (double)c / 3);
					color += RayTracing(ray_O, ray_V, 1, NULL) / 9;
				}
			camera->SetColor(i, j, color);
		}
	}
	
	for ( int i = 0 ; i < H ; i++ )
		delete[] sample[i];
	delete[] sample;

	camera->Outputfile( output );

}
/*

background
color= 0.1 0.1 0.1
end

camera
O= 5 5 3.5
N= -0.5 -0.5 -0.35
shade_quality= 4
drefl_quality= 4
image_H= 512
image_W= 826
//	image_H= 760
//	image_W= 1240
lens_H= 0.6
lens_W= 1.0
end

light area
O= 3 3 7
Dx= 0.5 0 0
Dy= 0 0.5 0
color= 1 1 1
end

primitive triangle
P1= 1 1 1.3
P2= 0 1 1.3
P3= 1.414 1.414 2.3
refr= 1
rindex= 2.3
absor= 0 0 0
end

primitive triangle
P1= 1 1 1.3
P3= 0 1 1.3
P2= 1.414 1.414 0.3
refr= 1
rindex= 2.3
absor= 0 0 0
end

primitive triangle
P1= 1 1 1.3
P2= 1 0 1.3
P3= 1.414 1.414 0.3
refr= 1
rindex= 2.3
absor= 0 0 0
end

primitive triangle
P1= 1 1 1.3
P3= 1 0 1.3
P2= 1.414 1.414 2.3
refr= 1
rindex= 2.3
absor= 0 0 0
end

primitive triangle
P1= 0 0 1.3
P2= 1 0 1.3
P3= 1.414 1.414 2.3
refr= 1
rindex= 2.3
absor= 0 0 0
end

primitive triangle
P1= 0 0 1.3
P3= 1 0 1.3
P2= 1.414 1.414 0.3
refr= 1
rindex= 2.3
absor= 0 0 0
end

primitive triangle
P1= 0 0 1.3
P2= 0 1 1.3
P3= 1.414 1.414 0.3
refr= 1
rindex= 2.3
absor= 0 0 0
end

primitive triangle
P1= 0 0 1.3
P3= 0 1 1.3
P2= 1.414 1.414 2.3
refr= 1
rindex= 2.3
absor= 0 0 0
end

primitive sphere
O= 1 1 0.6
R= 0.4
color= 0.8 0.8 0.8
refl= 0.8
spec= 0.2
end

primitive sphere
O= 1.4142 1.4142 1.3
R= 0.3
color= 0 0 1
diff= 0.9
spec= 0.1
refl= 0.9
drefl= 0.45
end

//primitive sphere
//O= 1 1 2
//R= 0.4
//color= 1 1 1
//texture= marble.bmp
//De= 0 0 1
//Dc= 0 1 0
//diff= 0.8
//spec= 0.2
//refl= 0.3
//drefl= 0.95
//end

primitive sphere
O= 2 2 0.4
R= 0.3
refr= 1
rindex= 1.7
absor= 0.3 0.3 0.3
end

//primitive sphere
//O= 1 2.646 0.3
//R= 0.3
//refr= 1
//rindex= 1.7
//absor= 0 1 0
//end

//primitive sphere
//O= 2.646 1 0.3
//R= 0.3
//refr= 1
//rindex= 1.7
//absor= 1 0 0
//end


primitive plane
N= 0 0 1
R= 0
color= 1 1 1
diff= 0.1
spec= 0.9
refl= 0.9
drefl= 0.05
//	texture= floor.bmp
Dx= 1 0 0
Dy= 0 1 0
end


primitive plane
N= 1 0 0
R= -1.5
color= 1 1 1
diff= 0.9
spec= 0.1
refl= 0.9
drefl= 0.95
texture= wallpaper.bmp
Dx= 0 1 0
Dy= 0 0 1
end

primitive plane
N= 0 1 0
R= -1.5
color= 1 1 1
diff= 0.9
spec= 0.1
refl= 0.9
drefl= 0.95
texture= wallpaper.bmp
Dx= 1 0 0
Dy= 0 0 1
end

primitive plane
N= 0 0 -1
R= 10
color= 1 1 1
diff= 0.1
spec= 0.9
refl= 0.9
drefl= 0.05
texture= wallpaper.bmp
Dx= 1 0 0
Dy= 0 1 0
end







background
color= 0.1 0.1 0.1
end

camera
O= 5 5 5
N= -0.5 -0.5 -0.5
shade_quality= 4
drefl_quality= 4
image_H= 512
image_W= 826
//	image_H= 760
//	image_W= 1240
lens_H= 0.6
lens_W= 1.0
end

light area
O= 3 3 7
Dx= 10 0 0
Dy= 0 10 0
color= 1 1 1
end

primitive triangle
P1= 2 0 0
P2= 0 2 0
P3= 0 0 2
color= 1 1 1
diff= 0.1
spec= 0.9
refl= 0.9
drefl= 0
end

primitive plane
N= 1 1 1
R= 1.732
color= 1 1 1
diff= 0.1
spec= 0.9
refl= 0.9
drefl= 0
Dx= 1 0 0
Dy= 0 1 0
end

primitive sphere
O= 2 2 2
R= 0.3
refr= 1
rindex= 1.7
absor= 0 1 0
end


primitive plane
N= 1 0 0
R= 0
color= 1 1 1
diff= 0.9
spec= 0.1
refl= 0.9
drefl= 0.95
texture= wallpaper.bmp
Dx= 0 1 0
Dy= 0 0 1
end

primitive plane
N= 0 1 0
R= 0
color= 1 1 1
diff= 0.9
spec= 0.1
refl= 0.9
drefl= 0.95
texture= wallpaper.bmp
Dx= 1 0 0
Dy= 0 0 1
end

*/