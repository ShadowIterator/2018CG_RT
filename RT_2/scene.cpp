#include"scene.h"
#include<string>
#include<fstream>
#include<sstream>
#include<cstdlib>
#include<ctime>

Scene::Scene() {
	primitive_head = NULL;
	light_head = NULL;
	background_color = Color();
	camera = new Camera;
}

Scene::~Scene() {
	while ( primitive_head != NULL ) {
		Primitive* next_head = primitive_head->GetNext();
		if ( primitive_head->GetMaterial()->texture != NULL )
			delete primitive_head->GetMaterial()->texture;
		delete primitive_head;
		primitive_head = next_head;
	}

	while ( light_head != NULL ) {
		Light* next_head = light_head->GetNext();
		delete light_head;
		light_head = next_head;
	}

	delete camera;
}

void Scene::CreateScene( std::string file ) {
	srand( 1995 - 05 - 12 );
	std::ifstream fin( file.c_str() );

	std::string obj;
	while ( fin >> obj ) {
		Primitive* new_primitive = NULL;
		Light* new_light = NULL;
		std::string type;
		if ( obj == "primitive" ) {
			fin >> type;
			if ( type == "sphere" ) new_primitive = new Sphere;
			if ( type == "plane" ) new_primitive = new Plane;
			if (type == "triangle") new_primitive = new Triangle;
			if ( new_primitive != NULL ) {
				new_primitive->SetNext( primitive_head );
				primitive_head = new_primitive;
			}
		} else
		if ( obj == "light" ) {
			fin >> type;
			if ( type == "area" ) new_light = new AreaLight;
			if ( new_light != NULL ) {
				new_light->SetNext( light_head );
				light_head = new_light;
			}
		} else
		if ( obj != "background" && obj != "camera" ) continue;

		fin.ignore( 1024 , '\n' );
		
		std::string order;
		while ( getline( fin , order , '\n' ) ) {
			std::stringstream fin2( order );
			std::string var; fin2 >> var;
			if (var == "end")
			{
				if (obj == "primitive"&& type == "triangle")
					((Triangle*)(new_primitive))->calcN();
				break;
			}

			if ( obj == "background" && var == "color=" ) background_color.Input( fin2 );
			if ( obj == "primitive" && new_primitive != NULL ) new_primitive->Input( var , fin2 );
			if ( obj == "light" && new_light != NULL ) new_light->Input( var , fin2 );
			if ( obj == "camera" ) camera->Input( var , fin2 );
		}
	}

	camera->Initialize();
}

Primitive* Scene::FindNearestPrimitive( Vector3lf ray_O , Vector3lf ray_V ) {
	Primitive* ret = NULL;

	for ( Primitive* now = primitive_head ; now != NULL ; now = now->GetNext() )
		if ( now->Collide( ray_O , ray_V ) && ( ret == NULL || now->crash.dist < ret->crash.dist ) ) ret = now;

	return ret;
}

Light* Scene::FindNearestLight( Vector3lf ray_O , Vector3lf ray_V ) {
	Light* ret = NULL;

	for ( Light* now = light_head ; now != NULL ; now = now->GetNext() )
		if ( now->Collide( ray_O , ray_V ) && ( ret == NULL || now->crash_dist < ret->crash_dist ) ) ret = now;

	return ret;
}

/*
background
color= 0.1 0.1 0.1
end

camera
O= 4.5 6 4.5
N= -0.45 -0.6 -0.45
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

//primitive sphere
//	O= 1 1 0.6
//	R= 0.6
//	color= 0.9 0.2 0.3
//	refl= 0.8
//	spec= 0.2
//end

primitive triangle
P1= 1 1 1.3
P2= 0 1 1.3
P3= 1.414 1.414 2.3
color= 0.5 0.5 1
spec= 0.2
refl= 0.8
end

primitive triangle
P1= 1 1 1.3
P3= 0 1 1.3
P2= 1.414 1.414 0.3
color= 0.5 0.5 1
spec= 0.2
refl= 0.8
end

primitive triangle
P1= 1 1 1.3
P2= 1 0 1.3
P3= 1.414 1.414 0.3
color= 0.5 0.5 1
spec= 0.2
refl= 0.8
end

primitive triangle
P1= 1 1 1.3
P3= 1 0 1.3
P2= 1.414 1.414 2.3
color= 0.5 0.5 1
spec= 0.2
refl= 0.8
end

primitive triangle
P1= 0 0 1.3
P2= 1 0 1.3
P3= 1.414 1.414 2.3
color= 0.5 0.5 1
spec= 0.2
refl= 0.8
end

primitive triangle
P1= 0 0 1.3
P3= 1 0 1.3
P2= 1.414 1.414 0.3
color= 0.5 0.5 1
spec= 0.2
refl= 0.8
end

primitive triangle
P1= 0 0 1.3
P2= 0 1 1.3
P3= 1.414 1.414 0.3
color= 0.5 0.5 1
spec= 0.2
refl= 0.8
end

primitive triangle
P1= 0 0 1.3
P3= 0 1 1.3
P2= 1.414 1.414 2.3
color= 0.5 0.5 1
spec= 0.2
refl= 0.8
end

primitive sphere
O= 1 1 2.8
R= 0.3
color= 1 1 1
texture= marble.bmp
De= 0 0 1
Dc= 0 1 0
diff= 0.8
spec= 0.2
refl= 0.3
drefl= 0.95

end

primitive sphere
O= 2 2 0.5
R= 0.3
refr= 1
rindex= 1.7
absor= 0.3 0.3 0.3
end

primitive sphere
O= 1 2.646 0.3
R= 0.3
refr= 1
rindex= 1.7
absor= 0 1 0
end

primitive sphere
O= 2.646 1 0.3
R= 0.3
refr= 1
rindex= 1.7
absor= 1 0 0
end


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

//
//primitive plane
//N= 1 0 0
//R= -1.5
//color= 1 1 1
//diff= 0.9
//spec= 0.1
//refl= 0.9
//drefl= 0.95
//texture= wallpaper.bmp
//Dx= 0 1 0
//Dy= 0 0 1
//end
//
//primitive plane
//N= 0 1 0
//R= -1.5
//color= 1 1 1
//diff= 0.9
//spec= 0.1
//refl= 0.9
//drefl= 0.95
//texture= wallpaper.bmp
//Dx= 1 0 0
//Dy= 0 0 1
//end
//
//primitive plane
//N= 0 0 -1
//R= 10
//color= 1 1 1
//diff= 0.1
//spec= 0.9
//refl= 0.9
//drefl= 0.05
//texture= wallpaper.bmp
//Dx= 1 0 0
//Dy= 0 1 0
//end
*/