#pragma once

#ifdef _WIN32
#define APIENTRY __stdcall
#endif

// GLAD
#ifndef USING_GLEW
#include <glad/glad.h>
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

// confirm that GLAD didn't include windows.h
#ifdef _WINDOWS_
#error windows.h was included!
#endif

// GLFW
#include <GLFW/glfw3.h>

#include <string>
#include <glm/glm.hpp>

class WindowManager {
protected:
	GLFWwindow *window;
	
	int window_width, window_height;


public:
	WindowManager();
	WindowManager(int width, int height, std::string name, 
		glm::vec4 color = glm::vec4(1.f));

	void mainLoop();
	void noiseLoop();
};

void initGLExtensions();
GLFWwindow *createWindow(int width, int height, std::string name);