#include"engine.h"
#include<cstdlib>
#include<iostream>
#define ran() ( double( rand() % 32768 ) / 32768 )

//const double SPEC_POWER = 20;
//const int MAX_DREFL_DEP = 2;
//const int MAX_RAYTRACING_DEP = 10;
//const int HASH_FAC = 7;
//const int HASH_MOD = 10000007;

Color Engine::CalnDiffusion( Primitive* pri , int* hash ) {
	Color color = pri->GetMaterial()->color;
	if ( pri->GetMaterial()->texture != NULL ) color = color * pri->GetTexture();
	
	Color ret = color * scene.GetBackgroundColor() * pri->GetMaterial()->diff;

	for ( Light* light = scene.GetLightHead() ; light != NULL ; light = light->GetNext() ) {
		double shade = light->CalnShade( pri->crash.C , scene.GetPrimitiveHead() , scene.GetCamera()->GetShadeQuality() );
		if ( shade < EPS ) continue;
		
		Vector3lf R = ( light->GetO() - pri->crash.C ).normalize();
		double dot = R % pri->crash.N ;
		if ( dot > EPS ) {
			if ( hash != NULL && light->IsPointLight() ) *hash = ( *hash + light->GetSample() ) & HASH_MOD;

			if ( pri->GetMaterial()->diff > EPS ) {
				double diff = pri->GetMaterial()->diff * dot * shade;
				ret += color * light->GetColor() * diff;
			}
			if ( pri->GetMaterial()->spec > EPS ) {
				double spec = pri->GetMaterial()->spec * pow( dot , SPEC_POWER ) * shade;
				ret += color * light->GetColor() * spec;
			}
		}
	}

	return ret;
}

Color Engine::CalnReflection( Primitive* pri , Vector3lf ray_V , int dep , int* hash ) {
	ray_V = ray_V - pri->crash.N * ( 2 * (ray_V% pri->crash.N ) );

	if ( pri->GetMaterial()->drefl < EPS || dep > MAX_DREFL_DEP )
		return RayTracing( pri->crash.C , ray_V , dep + 1 , hash ) * pri->GetMaterial()->color * pri->GetMaterial()->refl;

	Vector3lf Dx = ray_V * Vector3lf( 1 , 0 , 0 );
	if ( Dx.IsZeroVector() ) Dx = Vector3lf( 1 , 0 , 0 );
	Vector3lf Dy = ray_V * Dx;
	Dx = Dx.normalize() * pri->GetMaterial()->drefl;
	Dy = Dy.normalize() * pri->GetMaterial()->drefl;

	Color ret;
	for ( int k = 0 ; k < 16 * scene.GetCamera()->GetDreflQuality() ; k++ ) {
		double x , y;
		do {
			x = ran() * 2 - 1;
			y = ran() * 2 - 1;
		} while ( x * x + y * y > 1 );
		x *= pri->GetMaterial()->drefl;
		y *= pri->GetMaterial()->drefl;

		ret += RayTracing( pri->crash.C , ray_V + Dx * x + Dy * y , dep + MAX_DREFL_DEP , NULL );
	}

	ret = ret * pri->GetMaterial()->color * pri->GetMaterial()->refl / ( 16 * scene.GetCamera()->GetDreflQuality() );
	return ret;
}

Color Engine::CalnRefraction( Primitive* pri , Vector3lf ray_V , int dep , int* hash ) {
	double n = pri->GetMaterial()->rindex;
	if ( pri->crash.front ) n = 1 / n;

	ray_V._normalize();
	double cosI = -pri->crash.N%ray_V  , cosT2 = 1 - ( n * n ) * ( 1 - cosI * cosI ); 
	if ( cosT2 > EPS ) ray_V = ray_V * n + pri->crash.N * ( n * cosI - sqrt( cosT2 ) );
		else ray_V = ray_V - pri->crash.N * ( 2 * (ray_V% pri->crash.N ) );
	
	Color rcol = RayTracing( pri->crash.C , ray_V , dep + 1 , hash );
	if ( pri->crash.front ) return rcol * pri->GetMaterial()->refr;
	Color absor = pri->GetMaterial()->absor * -pri->crash.dist;
	Color trans = Color( exp( absor.r ) , exp( absor.g ) , exp( absor.b ) );
	return rcol * trans * pri->GetMaterial()->refr;
}

Color Engine::RayTracing( Vector3lf ray_O , Vector3lf ray_V , int dep , int* hash ) {
	if ( dep > MAX_RAYTRACING_DEP ) return Color();

	Color ret;
	Primitive* nearest_primitive = scene.FindNearestPrimitive( ray_O , ray_V );
	Light* nearest_light = scene.FindNearestLight( ray_O , ray_V );

	if ( nearest_light != NULL && ( nearest_primitive == NULL || nearest_light->crash_dist < nearest_primitive->crash.dist ) ) {
		if ( hash != NULL ) *hash = ( *hash + nearest_light->GetSample() ) % HASH_MOD;
		ret += nearest_light->GetColor();
	}
	
	if ( nearest_primitive != NULL ) {
		if ( hash != NULL ) *hash = ( *hash + nearest_primitive->GetSample() ) % HASH_MOD;
		Primitive* primitive = nearest_primitive->PrimitiveCopy();
		if ( primitive->GetMaterial()->diff > EPS || primitive->GetMaterial()->spec > EPS ) ret += CalnDiffusion( primitive , hash );
		if ( primitive->GetMaterial()->refl > EPS ) ret += CalnReflection( primitive , ray_V , dep , hash );
		if ( primitive->GetMaterial()->refr > EPS ) ret += CalnRefraction( primitive , ray_V , dep , hash );
		delete primitive;
	}

	if ( hash != NULL ) *hash = ( *hash * HASH_FAC ) % HASH_MOD;
	ret.Confine();
	return ret;
}

void Engine::Run() {
	Camera* camera = scene.GetCamera();
	scene.CreateScene( input );

	Vector3lf ray_O = camera->GetO();
	int H = camera->GetH() , W = camera->GetW();
	int** sample = new int*[H];
	for ( int i = 0 ; i < H ; i++ ) {
		sample[i] = new int[W];
		for ( int j = 0 ; j < W ; j++ )
			sample[i][j] = 0;
	}

	for ( int i = 0 ; i < H ; std::cout << "Sampling:   " << ++i << "/" << H << std::endl )
		for ( int j = 0 ; j < W ; j++ ) {
			Vector3lf ray_V = camera->Emit( i , j );
			Color color = RayTracing( ray_O , ray_V , 1 , &sample[i][j] );
			camera->SetColor( i , j , color );
		}

	for ( int i = 0 ; i < H ; std::cout << "Resampling: " << ++i << "/" << H << std::endl )
		for ( int j = 0 ; j < W ; j++ ) {
			if ( ( i == 0 || sample[i][j] == sample[i - 1][j] ) && ( i == H - 1 || sample[i][j] == sample[i + 1][j] ) &&
			     ( j == 0 || sample[i][j] == sample[i][j - 1] ) && ( j == W - 1 || sample[i][j] == sample[i][j + 1] ) ) continue;

			Color color;
			for ( int r = -1 ; r <= 1 ; r++ )
				for ( int c = -1 ; c <= 1 ; c++ ) {
					Vector3lf ray_V = camera->Emit( i + ( double ) r / 3 , j + ( double ) c / 3 );
					color += RayTracing( ray_O , ray_V , 1 , NULL ) / 9;
				}
			camera->SetColor( i , j , color );
		}
	
	for ( int i = 0 ; i < H ; i++ )
		delete[] sample[i];
	delete[] sample;

	Bmp* bmp = new Bmp( H , W );
	camera->Output( bmp );
	bmp->Output( output );
	delete bmp;
}
