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

template<typename T1, typename T2>
struct A {
	std::tuple<T1, T2> t;
};

class B {
	VariableSizeGrid<TopFace<Noise>> grid;
	B(int x, int y) : grid(x, y) {}
};

int main()
{
	srand(time(0));

	WindowManager wm(800, 800, "Happy now Cory?");

	std::tuple<float, int, char> tp;

	A<float, int> obj1;

	FaceQ<Quadrant::TL, float> face;
	//face.edge<Side::Left>() = EdgeS<Side::Left, float>();
	//subdivideFace(face);
	TopFace<float> otherFace;
	subdivideFace(otherFace);

	VariableSizeGrid<TopFace<Noise>> grid(20, 20);
	
	initializeTopFaceGrid<float>(5, 5);

	//std::vector<TopFace<float>> topFaces(4);

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

