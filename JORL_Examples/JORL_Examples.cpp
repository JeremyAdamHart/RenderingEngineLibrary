// RenderingEngine.cpp : Defines the entry point for the console application.
//
#include "GLWindow.h"
#include <tuple>
#include <vector>
#include "AdaptiveNoise.h"

/*template<typename T>
optional{

}

template<typename Class, typename ...Args>
class PartialF{
	Class object;
	std::tuple<std::vector<Args>...> arguments;
	void (Class::*)(Args...) func;
	
};
*/
int main()
{
	WindowManager wm(800, 800, "Happy now Cory?");

	FaceQ<Quadrant::TL, float> face;
	face.edge<Side::Left>() = EdgeS<Side::Left, float>();

//	wm.testLoop();
//	wm.waveSimulationLoop(1000, 0.0001f);
//	wm.glowTest();
//	wm.testLoop();
//	wm.rigidBodyTest();
//	wm.mainLoop();
//	wm.noiseLoop();
//	wm.particleLoop();
	wm.convexTestLoop();
}

