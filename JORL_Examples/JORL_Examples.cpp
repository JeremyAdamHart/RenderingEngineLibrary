// RenderingEngine.cpp : Defines the entry point for the console application.
//
#include "GLWindow.h"
#include <tuple>
#include <vector>

#include "AdaptiveNoise.h"
#include "noiseTest.h"
#include <unordered_map>
#include <cctype>

#include <iostream>

int main()
{	
	srand(time(0));

	WindowManager wm(800, 800, "Test Window");

	std::tuple<float, int, char> tp;

	using namespace adaptive;

	//runAdaptiveNoiseTests();

	wm.shadowLoop();
//	wm.simpleModelLoop();
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
//	wm.growthLoop2D();
//	wm.adaptiveNoiseLoop();
//	wm.laplacianSmoothing();
//	wm.laplacianSmoothingMeshLoop();
//	wm.growthLoop2D();

}

