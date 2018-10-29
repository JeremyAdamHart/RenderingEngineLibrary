// RenderingEngine.cpp : Defines the entry point for the console application.
//
#include "GLWindow.h"


int main()
{
	WindowManager wm;
	wm.waveSimulationLoop(1000, 0.0001f);
//	wm.testLoop();
//	wm.rigidBodyTest();
//	wm.mainLoop();
}

