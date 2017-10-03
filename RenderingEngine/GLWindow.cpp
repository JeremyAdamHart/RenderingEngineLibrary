#include "GLWindow.h"

#include <iostream>

using namespace glm;
using namespace std;

#include "ModelLoader.h"
#include "Drawable.h"
#include "SimpleGeometry.h"
#include "SimpleShader.h"
#include "ColorMat.h"
#include "TrackballCamera.h"
#include "SimpleTexManager.h"
#include "simpleTexShader.h"
#include "TextureCreation.h"
#include "TextureMat.h"
#include "MeshInfoLoader.h"
#include "ShadedMat.h"
#include "TorranceSparrowShader.h"
#include "Framebuffer.h"
//Ambient occlusion
#include "AOShader.h"
#include "PosNormalShader.h"
#include "PerlinNoise.h"
#include <sstream>

//Particle system
#include "HeatParticleGeometry.h"
#include "HeatParticleMat.h"
#include "HeatParticleShader.h"
#include "HeatParticleSystem.h"


#include <glm/gtc/matrix_transform.hpp>

using namespace renderlib;

TrackballCamera cam(
	vec3(0, 0, -1), vec3(0, 0, 1),
	glm::perspective(90.f*3.14159f/180.f, 1.f, 0.1f, 3.f));

void cursorPositionCallback(GLFWwindow *window, double xpos, double ypos) {
	static vec2 lastPos = vec2(0.f, 0.f);
	
	int vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);
	vec2 mousePos = vec2(float(xpos) / float(vp[2]), 
		float(-ypos) / float(vp[3]))*2.f
		- vec2(1.f, 1.f);

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		vec2 diff = mousePos - lastPos;
		cam.trackballRight(-diff.x*3.14159f);
		cam.trackballUp(-diff.y*3.14159f);
	}

	lastPos = mousePos;
}

WindowManager::WindowManager() :
window_width(800), window_height(800)
{

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);	
	glfwInit();
	window = createWindow(window_width, window_height, 
		"You really should rename this");
	initGLExtensions();

	glfwSwapInterval(1);
	glfwSetCursorPosCallback(window, cursorPositionCallback);

	glClearColor(1.f, 1.f, 1.f, 1.f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glViewport(0, 0, window_width, window_height);
}

WindowManager::WindowManager(int width, int height, std::string name, glm::vec4 color) :
	window_width(width), window_height(height) 
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);	
	glfwInit();
	glfwSetCursorPosCallback(window, cursorPositionCallback);
	window = createWindow(window_width, window_height, name);
	initGLExtensions();

	glClearColor(color.r, color.g, color.b, color.a);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glViewport(0, 0, window_width, window_height);
}

#define M_PI 3.1415926535897932384626433832795
#define MOD_MAX 8388608

uint lcg(uint seed) {
	int value = (65793 * seed + 4282663) % MOD_MAX;
	return value;
}

vec2 gradient(uint i, uint j, uint seed) {
	uint rand = lcg(lcg(seed + i) + j);
	float theta = float(rand) / float(MOD_MAX);
	return vec2(sin(theta), cos(theta));
}

void WindowManager::noiseLoop() {
	vec3 points[6] = {
		//First triangle
		vec3(-0.5f, 0.5f, 0.f)*2.f,
		vec3(0.5f, 0.5f, 0.f)*2.f,
		vec3(0.5f, -0.5f, 0.f)*2.f,
		//Second triangle
		vec3(0.5f, -0.5f, 0.f)*2.f,
		vec3(-0.5f, -0.5f, 0.f)*2.f,
		vec3(-0.5f, 0.5f, 0.f)*2.f
	};

	vec2 coords[6] = {
		//First triangle
		vec2(0, 1.f),
		vec2(1.f, 1.f),
		vec2(1.f, 0.f),
		//Second triangle
		vec2(1.f, 0.f),
		vec2(0.f, 0.f),
		vec2(0.f, 1.f)
	};

	gradient(0, 0, 1);
	gradient(0, 1, 1);
	gradient(1, 0, 1);
	gradient(1, 1, 1);

	int value = 1;
	for (int i = 0; i < 10; i++) {
		value = lcg(value);
		printf("Value = %d\n", value);
	}

	SimpleTexManager tm;
	PerlinNoiseShader2D perlinShader;
	Drawable texSquare(
		new TextureMat(createTexture2D(1, 1, &tm)),
		new SimpleTexGeometry(points, coords, 6, GL_TRIANGLES));

	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		perlinShader.draw(cam, texSquare);


		glfwSwapBuffers(window);
		glfwWaitEvents();
	}

	glfwTerminate();
}

//Temporary testing
void WindowManager::mainLoop() {

	noiseLoop();

	return;

	/*HeatParticleShader pShader;
	
	HeatParticleSystem pSystem;
	HeatParticleGeometry pGeometry;
	HeatParticleMat pMat(0.07f);

	Drawable pDrawable(&pMat, &pGeometry);

#define PI 3.14159265359f
	
	float initialVelocity = 7.0f;
	float lifespan = 0.2f;
	float heat = 0.2f;
	float divergenceAngle = PI/8.f;
	const int particlesPerStep = 250;
	
	Disk particleSpawner(0.05f, vec3(0.f, 0.f, 0.f), vec3(0, 1.f, 0));

	for (int i = 0; i < 500; i++) {
		pSystem.addParticleFromDisk(particleSpawner, initialVelocity, 
			heat, lifespan, divergenceAngle);
	}

	float timeElapsed = 0.f;

	float thrust = 0.f;

	glClearColor(0.f, 0.f, 0.0f, 0.f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	while (!glfwWindowShouldClose(window)) {
		glDepthMask(GL_TRUE);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDepthMask(GL_FALSE);

		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
			thrust = std::min(thrust + 0.02f, 1.f);
		else if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
			thrust = std::max(thrust - 0.02f, 0.f);

		float currentTime = glfwGetTime();
		float timeOffset = 0.f;
		for(int i=0; i<int((0.f+2.f*thrust)*float(particlesPerStep)/2.f); i++){
			float newHeat = heat*(float(rand()) / float(RAND_MAX))*(0.75f + thrust/4.f);
			pSystem.addParticleFromDisk(particleSpawner, initialVelocity*(0.5f+thrust/2.f),
				newHeat, lifespan, divergenceAngle*(1.f - thrust)*2.f, timeOffset);
			timeOffset += (currentTime - timeElapsed) / float(particlesPerStep);
		}

		pSystem.runSimulation((currentTime - timeElapsed)*0.5f);

		pGeometry.loadParticles(pSystem.particles.data(), pSystem.particles.size());

		pShader.draw(cam, pDrawable);

		timeElapsed = currentTime;

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	return;*/

	//Original main loop

	//Test
	std::stringstream ss;
	ss.str("Sample characters\nWith spaces\n");
	string arg;
	while (ss.good()) {
		ss >> arg;
		cout << arg << endl;
	}

	//glfwSetCursorPosCallback(window, cursorPositionCallback);

	vec3 points [6] = {
		//First triangle
		vec3(-0.5f, 0.5f, 0.f)*2.f,
		vec3(0.5f, 0.5f, 0.f)*2.f,
		vec3(0.5f, -0.5f, 0.f)*2.f,
		//Second triangle
		vec3(0.5f, -0.5f, 0.f)*2.f,
		vec3(-0.5f, -0.5f, 0.f)*2.f,
		vec3(-0.5f, 0.5f, 0.f)*2.f
	};

	vec2 coords[6] = {
		//First triangle
		vec2(0, 1.f),
		vec2(1.f, 1.f),
		vec2(1.f, 0.f),
		//Second triangle
		vec2(1.f, 0.f),
		vec2(0.f, 0.f),
		vec2(0.f, 1.f)
	};
	SimpleTexManager tm;

	//AO framebuffer
	Framebuffer pnFbo = createNewFramebuffer(window_width, window_height);
	pnFbo.addTexture(createTexture2D(
		TexInfo(GL_TEXTURE_2D, {window_width, window_height}, 0, GL_RGB, GL_RGB32F, GL_FLOAT), &tm), 
		GL_COLOR_ATTACHMENT0);

	pnFbo.addTexture(createTexture2D(
		TexInfo(GL_TEXTURE_2D, {window_width, window_height}, 0, GL_RGB, GL_RGB32F, GL_FLOAT), &tm), 
		GL_COLOR_ATTACHMENT1);
	pnFbo.addTexture(createDepthTexture(window_width, window_height, &tm), GL_DEPTH_ATTACHMENT);

	Framebuffer fbWindow (window_width, window_height);
	const int TEX_WIDTH = 160;
	const int TEX_HEIGHT = 160;
	Framebuffer fbTex = createNewFramebuffer(TEX_WIDTH, TEX_HEIGHT);
	
	if (!fbTex.addTexture(createTexture2D(TEX_WIDTH, TEX_HEIGHT, &tm),
		GL_COLOR_ATTACHMENT0) ||
		!fbTex.addTexture(createDepthTexture(TEX_WIDTH, TEX_HEIGHT, &tm),
			GL_DEPTH_ATTACHMENT)) {
		std::cout << "FBO creation failed" << endl;
	}

	//Dragon
	ElementGeometry dragonGeom = objToElementGeometry("models/dragon.obj");
	Drawable dragon(
		new ColorMat(vec3(0.75f, 0.1f, 0.3f)),
		&dragonGeom);
	dragon.addMaterial(new ShadedMat(0.2f, 0.5f, 0.3f, 10.f));

	Drawable texSquare(
		new TextureMat(fbTex.getTexture(GL_COLOR_ATTACHMENT0)),
		new SimpleTexGeometry(points, coords, 6, GL_TRIANGLES));

	texSquare.addMaterial(new TextureMat(pnFbo.getTexture(GL_COLOR_ATTACHMENT0), TextureMat::POSITION));
	texSquare.addMaterial(new TextureMat(pnFbo.getTexture(GL_COLOR_ATTACHMENT1), TextureMat::NORMAL));

	SimpleTexShader texShader;
	SimpleShader shader;
	TorranceSparrowShader tsShader;
	TorranceSparrowShader tsTexShader(
	{ { GL_VERTEX_SHADER, "#define USING_TEXTURE\n" },
	{ GL_FRAGMENT_SHADER, "#define USING_TEXTURE\n"} });

	fbTex.use();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	tsShader.draw(cam, vec3(10.f, 10.f, 10.f), dragon);

	fbWindow.use();

	PosNormalShader pnShader;
	AOShader aoShader;


	vector<Drawable> drawables;
//	loadWavefront("untrackedmodels/SciFiCenter/CenterCity/", "Center_City_Sci-Fi", &drawables, &tm);
//	loadWavefront("untrackedmodels/OrganodronCity2/", "OrganodronCity", &drawables, &tm);

/*	Drawable debugSquare(
		new TextureMat(dynamic_cast<TextureMat*>(drawables[2].getMaterial(TextureMat::id))->tex),
		new SimpleTexGeometry(points, coords, 6, GL_TRIANGLES));
*/
	vec3 lightPos(10.f, 10.f, 10.f);

	for (int i = 0; i < drawables.size(); i++) {
		drawables[i].setScale(vec3(0.5f));
	}

	while (!glfwWindowShouldClose(window)) {

		//texShader.draw(cam, texSquare);

		//Render dragon with Ambient Occlusion
		pnFbo.use();
		glClearColor(0.f, 0.f, 0.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		pnShader.draw(cam, vec3(0, 0, 0), dragon);
		fbWindow.use();
		glClearColor(1.f, 1.f, 1.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		aoShader.draw(cam, vec3(10.f, 10.f, 10.f), texSquare);

		texShader.draw(cam, texSquare);

/*		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		for (int i = 0; i < drawables.size(); i++) {
			if (drawables[i].getMaterial(TextureMat::id) != nullptr) {
				tsTexShader.draw(cam, lightPos, drawables[i]);
			}
			else
				tsShader.draw(cam, lightPos, drawables[i]);
		}
		//texShader.draw(cam, debugSquare);
*/
		

//		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//		tsShader.draw(cam, vec3(10.f, 10.f, 10.f), dragon);

		glfwSwapBuffers(window);
		glfwWaitEvents();
	}

	delete texSquare.getMaterial(TextureMat::id);
	delete texSquare.getGeometryPtr();

	delete dragon.getMaterial(ColorMat::id);
	delete dragon.getMaterial(ShadedMat::id);

	fbTex.deleteFramebuffer();
	fbTex.deleteTextures();

	glfwTerminate();
}

void initGLExtensions() {
#ifndef USING_GLEW
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "GLAD initialization failed" << std::endl;
	}
#else
	glewExperimental = true;
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		std::cout << "GLEW initialization failed" << std::endl;
	}
#endif
}

GLFWwindow *createWindow(int width, int height, std::string name)
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	GLFWwindow *window = glfwCreateWindow(
		width, height, name.c_str(), nullptr, nullptr);
	
	if (window == NULL) {
		glfwTerminate();
		return nullptr;
	}
	else {
		glfwMakeContextCurrent(window);
		return window;
	}
}

