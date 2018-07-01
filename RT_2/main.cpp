#include"raytracer.h"

using std::cout;
using std::endl;

int main() {
	Raytracer* raytracer = new Raytracer;
	raytracer->SetInput( "scene-back2.txt" );
	raytracer->SetOutput( "picture-back2-8.bmp" );
	cout << "start run" << endl;
	raytracer->Run();
	return 0;
}
