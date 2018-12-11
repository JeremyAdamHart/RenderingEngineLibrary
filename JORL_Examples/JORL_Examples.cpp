// RenderingEngine.cpp : Defines the entry point for the console application.
//
#include "GLWindow.h"

int main()
{
	WindowManager wm(800, 800, "Happy now Cory?");

	wm.testLoop();
//	wm.waveSimulationLoop(1000, 0.0001f);
//	wm.glowTest();
//	wm.testLoop();
//	wm.rigidBodyTest();
//	wm.mainLoop();
//	wm.noiseLoop();
//	wm.particleLoop();
}

