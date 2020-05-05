// RenderingEngine.cpp : Defines the entry point for the console application.
//
#include "GLWindow.h"
#include <tuple>
#include <vector>

#include "AdaptiveNoise.h"
#include "noiseTest.h"
#include <unordered_map>
#include <sstream>
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
///*
void resourceTest() {
	const size_t N = 3;
	Resource<int, N> rm;
	
	std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

	std::thread writer = std::thread([](Resource<int, N>& rm, std::chrono::system_clock::time_point start) {
		for (int i = 0; i < 1000; i++) {
			auto value = rm.getWrite();
			*value = i;
			std::chrono::duration<double> timediff = std::chrono::system_clock::now() - start;
			std::printf("Write %d from %d, at %f\n", *value, value.id, timediff);
			std::this_thread::sleep_for(std::chrono::microseconds(100));
		}
	}, std::ref(rm), start);

	int value = -1;
	while (value < 999) {
		auto reader = rm.getRead();
		value = *reader;
		std::chrono::duration<double> timediff = std::chrono::system_clock::now() - start;
		std::printf("\t\tRead %d from %d, at %f\n", value, reader.id, timediff);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	writer.join();
}
//*/

void resourceAndMultiBufferTest() {

}

class Locked{
public:
	std::unique_lock<std::shared_mutex> lock;
	Locked(std::unique_lock<std::shared_mutex> lock) :lock(std::move(lock)) {}

};

class LockedShared {
public:
	std::shared_lock<std::shared_mutex> lock;
	LockedShared(std::shared_lock<std::shared_mutex> lock) :lock(std::move(lock)) {}
};

int main()
{	
	Resource<int, 3> intResource(0);

	intResource.getWrite().data = 7;

	//testMultiBufferStager();

	std::shared_mutex m;
	//auto lock = std::unique_lock(m);
	{
		Locked l(std::unique_lock<std::shared_mutex>(m));
	}

	LockedShared l(std::shared_lock<std::shared_mutex>(m));

	//resourceTest();

	srand(time(0));

	WindowManager wm(800, 800, "Test Window");

	std::tuple<float, int, char> tp;

	using namespace adaptive;

	//runAdaptiveNoiseTests();

//	wm.rationalBezierLoop();
//	wm.colorUpdatingLoop();
//	wm.petioleAlignmentLoop();
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
	wm.convexTestLoop();
//	wm.velocitySpringLoop();
//	wm.growthLoop2D();
//	wm.adaptiveNoiseLoop();
//	wm.laplacianSmoothing();
//	wm.laplacianSmoothingMeshLoop();
//	wm.growthLoop2D();

}

