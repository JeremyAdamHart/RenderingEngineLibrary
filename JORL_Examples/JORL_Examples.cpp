// RenderingEngine.cpp : Defines the entry point for the console application.
//
#include "GLWindow.h"
#include <tuple>
#include <vector>

#include "AdaptiveNoise.h"
#include "noiseTest.h"
#include <unordered_map>
#include <cctype>
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
	adaptive::VariableSizeGrid<adaptive::TopFace<adaptive::Noise>> grid;
	B(int x, int y) : grid(x, y) {}
};

int main()
{	
	srand(time(0));

	WindowManager wm(800, 800, "Happy now Cory?");

	std::tuple<float, int, char> tp;

	A<float, int> obj1;

	using namespace adaptive;

	/*TopFace<Counter> tf;
	tf.edge<Side::Left>().vertex = std::make_shared<Vertex<Counter>>();
	tf.edge<Side::Bottom>().vertex = std::make_shared<Vertex<Counter>>();
	tf.edge<Side::Right>().vertex = std::make_shared<Vertex<Counter>>();
	tf.edge<Side::Top>().vertex = std::make_shared<Vertex<Counter>>();

	printf("vertices = %d, %d, %d, %d\n", 
		tf.vertex<Quadrant::BL>().d.val,
		tf.vertex<Quadrant::BR>().d.val,
		tf.vertex<Quadrant::TR>().d.val,
		tf.vertex<Quadrant::TL>().d.val);

	VariableSizeGrid<TopFace<Counter>> g = initializeTopFaceGrid<Counter>(3, 3);
	*/

	runAdaptiveNoiseTests();


	/*
	FaceQ<Quadrant::TL, float> face;
	//face.edge<Side::Left>() = EdgeS<Side::Left, float>();
	//subdivideFace(face);
	TopFace<float> otherFace;
	subdivideFace(otherFace);

	VariableSizeGrid<TopFace<Noise>> grid(20, 20);
	
	initializeTopFaceGrid<float>(5, 5);
	*/
	//std::vector<TopFace<float>> topFaces(4);

//	wm.treeGrowthTest();
//	wm.testLoop();
//	wm.waveSimulationLoop(1000, 0.0001f);
//	wm.glowTest();
//	wm.testLoop();
//	wm.rigidBodyTest();
//	wm.mainLoop();
//	wm.noiseLoop();
//	wm.particleLoop();
//	wm.convexTestLoop();
//	wm.velocitySpringLoop();
	//wm.adaptiveNoiseLoop();
	//wm.laplacianSmoothing();
	wm.laplacianSmoothingMeshLoop();
	//wm.growthLoop2D();

}

