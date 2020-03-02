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

#include "MultiThreadedResource.h"

void pushToVec(const int& v, std::vector<int>* vec) {
	vec->push_back(v);
}

void testMultiBufferStager() {
	const size_t BUFFER_SIZE = 20;
	const size_t N = 3;
	std::vector<int> buffers[N];

	int counter = 0;

	MultiBufferStager<int, N> stager (BUFFER_SIZE*N);

	std::vector<int> newValues;
	for (size_t i = 0; i < BUFFER_SIZE*10; i++)
		newValues.push_back(counter++);
	{
		int start = 0;
		int end = 10;
		int buffer = 1;
		stager.add(newValues.begin() + start, newValues.begin() + end);
		stager.processStagedElements(buffer, pushToVec, &buffers[buffer]);
	}

	{
		int start = 10;
		int end = 20;
		int buffer = 0;
		stager.add(newValues.begin() + start, newValues.begin() + end);
		stager.processStagedElements(buffer, pushToVec, &buffers[buffer]);
	}

	{
		int start = 20;
		int end = 40;
		int buffer = 2;
		stager.add(newValues.begin() + start, newValues.begin() + end);
		stager.processStagedElements(buffer, pushToVec, &buffers[buffer]);
	}

	{
		int start = 40;
		int end = 50;
		int buffer = 2;
		stager.add(newValues.begin() + start, newValues.begin() + end);
		stager.processStagedElements(buffer, pushToVec, &buffers[buffer]);
	}

	{
		int start = 50;
		int end = 70;
		int buffer = 0;
		stager.add(newValues.begin() + start, newValues.begin() + end);
		stager.processStagedElements(buffer, pushToVec, &buffers[buffer]);
	}

	{
		int start = 70;
		int end = 90;
		int buffer = 0;
		stager.add(newValues.begin() + start, newValues.begin() + end);
		stager.processStagedElements(buffer, pushToVec, &buffers[buffer]);
	}

	{
		int start = 90;
		int end = 100;
		int buffer = 2;
		stager.add(newValues.begin() + start, newValues.begin() + end);
		stager.processStagedElements(buffer, pushToVec, &buffers[buffer]);
	}

	stager.processStagedElements(0, pushToVec, &buffers[0]);
	stager.processStagedElements(1, pushToVec, &buffers[1]);
	stager.processStagedElements(2, pushToVec, &buffers[2]);


	for (int i = 0; i < N; i++) {
		printf("---Buffer %d------\n", i);
		for (int& v : buffers[i])
			printf("\t%d\n", v);
	}
}

int main()
{	
	testMultiBufferStager();

	srand(time(0));

	WindowManager wm(800, 800, "Test Window");

	std::tuple<float, int, char> tp;

	using namespace adaptive;

	//runAdaptiveNoiseTests();

	wm.petioleAlignmentLoop();
//	wm.shadowLoop();
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

