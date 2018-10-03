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
#include "BlinnPhongShader.h"
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

//Other
#include "StreamGeometry.h"
#include "ColorShader.h"
#include "ColorSetMat.h"

//Rigid body test
#include "Physics.h"

//Glow test
#include "GlowShader.h"

#include <glm/gtc/matrix_transform.hpp>


using namespace renderlib;

TrackballCamera cam(
	vec3(0, 0, -1), vec3(0, 0, 1),
	glm::perspective(90.f*3.14159f/180.f, 1.f, 0.1f, 3.f));

bool reloadShaders = false;
bool windowResized = false;
int windowWidth, windowHeight;

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
	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
		vec2 diff = mousePos - lastPos;
		cam.zoom(pow(2.f, diff.y));
	}

	lastPos = mousePos;
}

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	windowResized = true;
	windowWidth = width;
	windowHeight = height;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE)
		reloadShaders = true;
}

GLFWwindow* glfwSetup(int window_width, int window_height, string window_name) {
	if (!glfwInit()) {
		printf("GLFW failed to initialize\n");
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
//	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);	

	GLFWwindow *window = glfwCreateWindow(
		window_width, window_height, window_name.c_str(), nullptr, nullptr);

	if (window == NULL) {
		glfwTerminate();
		return nullptr;
	}

	glfwMakeContextCurrent(window);

	glfwSwapInterval(1);
	glfwSetCursorPosCallback(window, cursorPositionCallback);

	return window;
}

WindowManager::WindowManager() :
window_width(800), window_height(800)
{
	window = glfwSetup(window_width, window_height, "Time to rename the window");
	initGLExtensions();
	printf("%s\n", glGetString(GL_VERSION));

	glClearColor(1.f, 1.f, 1.f, 1.f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glViewport(0, 0, window_width, window_height);
}

WindowManager::WindowManager(int width, int height, std::string name, glm::vec4 color) :
	window_width(width), window_height(height) 
{
	window = glfwSetup(window_width, window_height, name);
	initGLExtensions();
	printf("%s\n", glGetString(GL_VERSION));

	glClearColor(color.r, color.g, color.b, color.a);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glViewport(0, 0, window_width, window_height);
}

#define M_PI 3.1415926535897932384626433832795
#define MOD_MAX 8388608


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

	glfwSetKeyCallback(window, keyCallback);
	glfwSetWindowSizeCallback(window, windowResizeCallback);

	SimpleTexManager tm;
	PerlinNoiseShader2D perlinShader;
	Drawable texSquare(
		new SimpleTexGeometry(points, coords, 6, GL_TRIANGLES),
		new TextureMat(createTexture2D(1, 1, &tm)));

	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (reloadShaders) {
			perlinShader.createProgram();
			reloadShaders = false;
		}
		if (windowResized) {
			window_width = windowWidth;
			window_height = windowHeight;
			glViewport(0, 0, window_width, window_height);
		}

		perlinShader.draw(cam, texSquare);


		glfwSwapBuffers(window);
		glfwWaitEvents();
	}

	glfwTerminate();
}

void WindowManager::glowTest() {
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

	glfwSetKeyCallback(window, keyCallback);
	glfwSetWindowSizeCallback(window, windowResizeCallback);


	SimpleTexManager tm;

	Framebuffer lightFramebuffer = createNewFramebuffer(window_width, window_height);
	if (!lightFramebuffer.addTexture(createTexture2D(window_width, window_height, &tm), GL_COLOR_ATTACHMENT0)
		|| !lightFramebuffer.addTexture(createDepthTexture(window_width, window_height, &tm), GL_DEPTH_ATTACHMENT)) {
		//){
		printf("Failed to initialize lightFramebuffer\n");
	}

	Framebuffer gaussianFramebuffer = createNewFramebuffer(window_width, window_height);
	if (!gaussianFramebuffer.addTexture(createTexture2D(window_width, window_height, &tm), GL_COLOR_ATTACHMENT0)
		|| !gaussianFramebuffer.addTexture(createDepthTexture(window_width, window_height, &tm), GL_DEPTH_ATTACHMENT)) {
		printf("Failed to initialize gaussianFramebuffer\n");
	}
	
	Framebuffer fbWindow(window_width, window_height);

	FlatColorShader flatShader;
	GaussianBlurShader gaussianShader;

	Drawable texSquareHorizontal(
		new SimpleTexGeometry(points, coords, 6, GL_TRIANGLES),
		new TextureMat(lightFramebuffer.getTexture(GL_COLOR_ATTACHMENT0)));

	Drawable texSquareVertical(
		new SimpleTexGeometry(points, coords, 6, GL_TRIANGLES),
		new TextureMat(gaussianFramebuffer.getTexture(GL_COLOR_ATTACHMENT0)));


	BlinnPhongShader bpShader;

	//Dragon
	Drawable dragon(
		objToElementGeometry("models/dragon.obj"),
		new ColorMat(vec3(0.75f, 0.1f, 0.3f)));
	//dragon.addMaterial(new ShadedMat(0.2f, 0.5f, 0.3f, 10.f));

	const int N = 41;
	const float SIGMA = 0.4f*float(N + 1)*0.5f;

	glClearColor(0.f, 0.f, 0.f, 1.f);
	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (windowResized) {
			window_width = windowWidth;
			window_height = windowHeight;
			glViewport(0, 0, window_width, window_height);
		}

		lightFramebuffer.use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		flatShader.draw(cam, dragon);

		gaussianFramebuffer.use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		gaussianShader.draw(SIGMA, N, GaussianBlurShader::Direction::X, texSquareHorizontal);

		fbWindow.use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		gaussianShader.draw(SIGMA, N, GaussianBlurShader::Direction::Y, texSquareVertical);

		glClear(GL_DEPTH_BUFFER_BIT);
		bpShader.draw(cam, vec3(10, 10, 10), dragon);


		glfwSwapBuffers(window);
		glfwWaitEvents();
	}

	glfwTerminate();
}

void WindowManager::rigidBodyTest() {

	glfwSetKeyCallback(window, keyCallback);
	glfwSetCursorPosCallback(window, cursorPositionCallback);
	glfwSetWindowSizeCallback(window, windowResizeCallback);

	glClearColor(0.f, 0.f, 0.f, 1.f);

	float width = 5.f;
	float height = -4.f;
	vec3 planePoints[4] = {
		vec3(-width, height, -width),
		vec3(width, height, -width),
		vec3(width, height, width),
		vec3(-width, height, width)
	};
	vec3 planeNormals[4] = {
		vec3(0, 1, 0),
		vec3(0, 1, 0),
		vec3(0, 1, 0),
		vec3(0, 1, 0)
	};

	unsigned int planeIndices[6] = { 0, 1, 2, 0, 2, 3 };

	Drawable plane(new ElementGeometry(planePoints, planeNormals, nullptr, planeIndices, 4, 6), 
		new ShadedMat(0.2, 0.4f, 0.4f, 1.f));
	plane.addMaterial(new ColorMat(vec3(1.f, 0, 0)));

	SimpleTexManager tm;
	MeshInfoLoader minfo("models/coryPrism.obj");

	Drawable rectPrism(new ElementGeometry(
		minfo.vertices.data(), minfo.normals.data(), minfo.uvs.data(), minfo.indices.data(),
		minfo.vertices.size(), minfo.indices.size()), 
		new ShadedMat(0.2f, 0.4f, 0.4f, 10.f));
	rectPrism.addMaterial(new ColorMat(vec3(1.f)));

	BlinnPhongShader bpShader;

	cam = TrackballCamera(
		vec3(0, 0, -1), vec3(0, 0, 5),
		glm::perspective(90.f*3.14159f / 180.f, 1.f, 0.1f, 20.f));


	//Dimensions 0.5 x 2 x 0.5
//	RigidBody body(1.f, glm::mat3(0.354167f, 0, 0, 0, 0.0416667, 0, 0, 0, 0.354167));	//Cory's values
	RigidBody body(1.f, glm::mat3((4.f+0.25f)/12.f, 0, 0, 0, (4.f+0.25f)/12.f, 0, 0, 0, (4.f+4.f)/12.f));	//My values
//	RigidBody body(1.f, glm::mat3((4.f + 0.25f) / 12.f, 0, 0, 0, (0.25f + 0.25f) / 12.f, 0, 0, 0, (4.f + 0.25f) / 12.f));	//Flat
	body.position = vec3(0, -3.11926, 0);
	body.v = vec3(0, -0.789059, 0);
	quat rx = angleAxis(0.785398f, vec3(1, 0, 0));
	quat ry = angleAxis(0.785398f, vec3(0, 1, 0));
	body.orientation = ry*rx;
	rectPrism.position = body.position;
	rectPrism.orientation = body.orientation;
	body.addForce(vec3(1, 0, 0), vec3(0, 50, 1));
	bool animating = false;
	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (windowResized) {
			window_width = windowWidth;
			window_height = windowHeight;
			glViewport(0, 0, window_width, window_height);
		}

		static bool animationKeyPressed = false;
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !animationKeyPressed) {
			animating = !animating;
			animationKeyPressed = true;
		}else if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE){
			animationKeyPressed = false;
		}

		if (animating) {
			mat4 transform = body.getTransform();
			for (const vec3 &point : minfo.vertices) {
				vec3 transformedPoint = vec3(transform*vec4(point, 1.f));
				if (transformedPoint.y <= -4.f) {
					float fMagnitude = (-4.f - transformedPoint.y);
					body.addForce(vec3(0, 0.5f, 0)*fMagnitude, transformedPoint);
				}
			}
			
			body.addForce(vec3(0, -9.81f, 0), body.position);
//			body.addDampingForces(0.5f, 0.1f);
			body.resolveForces(1.f / 64.f);

			rectPrism.position = body.position;
			rectPrism.orientation = body.orientation;
		}

		const vec3 lightPos(20.f, 20.f, 20.f);
		bpShader.draw(cam, lightPos, rectPrism);
		bpShader.draw(cam, lightPos, plane);

//		static int count = 0;
//		printf("count %d\n", count);
//		count++;

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
}

//Temporary testing
void WindowManager::mainLoop() {
	//Original main loop

	//Test
	std::stringstream ss;
	ss.str("Sample characters\nWith spaces\n");
	string arg;
	while (ss.good()) {
		ss >> arg;
		cout << arg << endl;
	}

	glfwSetKeyCallback(window, keyCallback);
	glfwSetWindowSizeCallback(window, windowResizeCallback);
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
	Drawable dragon(
		objToElementGeometry("models/dragon.obj"),
		new ColorMat(vec3(0.75f, 0.1f, 0.3f)));
	dragon.addMaterial(new ShadedMat(0.2f, 0.5f, 0.3f, 10.f));

	Drawable texSquare(
		new SimpleTexGeometry(points, coords, 6, GL_TRIANGLES),
		new TextureMat(fbTex.getTexture(GL_COLOR_ATTACHMENT0)));

	texSquare.addMaterial(new TextureMat(pnFbo.getTexture(GL_COLOR_ATTACHMENT0), TextureMat::POSITION));
	texSquare.addMaterial(new TextureMat(pnFbo.getTexture(GL_COLOR_ATTACHMENT1), TextureMat::NORMAL));

	SimpleTexShader texShader;
	SimpleShader shader;
	BlinnPhongShader bpShader;
	BlinnPhongShader bpTexShader(
	{ { GL_VERTEX_SHADER, "#define USING_TEXTURE\n" },
	{ GL_FRAGMENT_SHADER, "#define USING_TEXTURE\n"} });

	fbTex.use();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	bpShader.draw(cam, vec3(10.f, 10.f, 10.f), dragon);

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

		if (reloadShaders) {
			aoShader.createProgram();
			reloadShaders = false;
		}
		if (windowResized) {
			window_width = windowWidth;
			window_height = windowHeight;
			pnFbo.resize(window_width, window_height);
			fbWindow.resize(window_width, window_height);
		}

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
				bpTexShader.draw(cam, lightPos, drawables[i]);
			}
			else
				bpShader.draw(cam, lightPos, drawables[i]);
		}
		//texShader.draw(cam, debugSquare);
*/
		

//		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//		bpShader.draw(cam, vec3(10.f, 10.f, 10.f), dragon);

		glfwSwapBuffers(window);
		glfwWaitEvents();
	}

	glfwTerminate();
}

void WindowManager::particleLoop() {
	HeatParticleShader pShader;

	HeatParticleSystem pSystem;
	shared_ptr<HeatParticleGeometry> pGeometry (new HeatParticleGeometry());
	shared_ptr<HeatParticleMat> pMat(new HeatParticleMat(0.07f));

	Drawable pDrawable(pGeometry, pMat);

#define PI 3.14159265359f

	float initialVelocity = 7.0f;
	float lifespan = 0.2f;
	float heat = 0.2f;
	float divergenceAngle = PI/8.f;
	const int particlesPerStep = 250;

#undef PI

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

		pGeometry->loadParticles(pSystem.particles.data(), pSystem.particles.size());

		pShader.draw(cam, pDrawable);

		timeElapsed = currentTime;

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void WindowManager::testLoop() {

	glfwSetKeyCallback(window, keyCallback);
	glfwSetWindowSizeCallback(window, windowResizeCallback);
	//glfwSetCursorPosCallback(window, cursorPositionCallback);

	//Testing object creation
	GLTexture tex1 = createTextureID();
	tex1.print();

	{
		GLTexture tex4 = createTextureID();
		tex4 = tex1;
		tex4.print();
		tex1.print();
	}
	tex1.print();
	GLTexture tex2 = createTextureID();
	tex2.print();
	tex1 = tex2;
	tex1.print();
	tex2.print();
	GLTexture tex3(tex1);
	tex3.print();
	tex1.print();

	{
		GLTexture tex5 = createTextureID();
		tex5.print();
	}




	//Dragon
	Drawable dragon(
		objToElementGeometry("models/dragon.obj"),
		new ColorMat(vec3(0.75f, 0.1f, 0.3f)));
	dragon.addMaterial(new ShadedMat(0.2f, 0.5f, 0.3f, 10.f));

	vec3 lightPos(10.f, 10.f, 10.f);

	BlinnPhongShaderT bpShader;

	while (!glfwWindowShouldClose(window)) {
		if (windowResized) {
			window_width = windowWidth;
			window_height = windowHeight;
		}
		glClearColor(0.f, 0.f, 0.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		bpShader.draw(cam, lightPos, dragon);
			
		glfwSwapBuffers(window);
		glfwWaitEvents();
	}

	glfwTerminate();
}

void WindowManager::objLoadingLoop() {

	glfwSetKeyCallback(window, keyCallback);
	glfwSetWindowSizeCallback(window, windowResizeCallback);

	MeshInfoLoader minfo1;
	minfo1.loadModelPly("../tinyply/assets/icosahedron_ascii.ply");

	SimpleTexManager tm;

	vec3 lightPos(10.f, 10.f, 10.f);

	vector<Drawable> drawables;

	enum {
		POSITION = 0, NORMAL, COLOR
	};
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//
	MeshInfoLoader minfo("models/dragon.obj");
//	MeshInfoLoader minfo("untrackedmodels/riccoSurface/riccoSurface.obj");
	shared_ptr<StreamGeometry<vec3, vec3, unsigned char>> streamGeometry(
		new StreamGeometry<vec3, vec3, unsigned char>(minfo.vertices.size(), { false, false, true}));
	streamGeometry->loadElementArray(minfo.indices.size(), GL_STATIC_DRAW, minfo.indices.data());

	vector<unsigned char> colors(minfo.vertices.size(), 1);

	streamGeometry->loadBuffer<POSITION>(minfo.vertices.data());
	streamGeometry->loadBuffer<NORMAL>(minfo.normals.data());
	streamGeometry->loadBuffer<COLOR>(colors.data());

	drawables.push_back(Drawable(streamGeometry));
		drawables[0].addMaterial(new ShadedMat(0.3, 0.4, 0.4, 10.f));
	drawables[0].addMaterial(new ColorSetMat({ vec3(1, 0, 0), vec3(0, 0, 1) }));

	BlinnPhongShader bpShader;
	ColorShader colorShader(2);

	for (int i = 0; i < drawables.size(); i++) {
//		drawables[i].setPosition(vec3(0, 0, -2));
//		drawables[i].setScale(vec3(0.1f));
	}

	float transition = 0.f;
	vec3 startColor(1, 0, 0);
	vec3 endColor(0, 0, 1);
	float step = 0.01f;

	double frameTime = 0.f;
	int frameTimeSamples = 0;
	double lastTime = glfwGetTime();

	bool useColor1 = false;

	glfwSwapInterval(0);

	int interval = 0;
	int maxInterval = 20;

	int counter = 0;

	char drawColor = 0;

	while (!glfwWindowShouldClose(window)) {

		glClearColor(0.f, 0.f, 0.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		int writeIndex = streamGeometry->buffManager.getWrite();

		interval++;
		if (interval >= maxInterval) {
			useColor1 = !useColor1;
			interval = 0;
		}

		for (int i = 0; i < drawables.size(); i++) {
			colorShader.draw(cam, lightPos, drawables[i]);
//			bpShader.draw(cam, lightPos, drawables[i]);
		}
	
//		int bufferNum = streamGeometry.buffManager.getWrite();
		int offset = 0;
		unsigned char *color = streamGeometry->vboPointer<COLOR>();
		const int UPDATE_NUM = 10000;
		for (int i = 0; i < UPDATE_NUM; i++) {
			if (counter + i >= streamGeometry->getBufferSize() && offset == 0) {
				offset = -int(streamGeometry->getBufferSize());
				drawColor = (drawColor == 0) ? 1 : 0;
			}
			int index = (counter + i) + offset;
//			color[index] = (color[index] == 0)?  1 : 0;
			streamGeometry->modify<COLOR>(index, drawColor);
		}

		counter = counter+UPDATE_NUM+offset;
		
		streamGeometry->dump<COLOR>();
		streamGeometry->buffManager.endWrite();
		
		if (frameTimeSamples > 30) {
			cout << "Time per frame = " << frameTime / double(frameTimeSamples) << endl;
			frameTime = 0.0;
			frameTimeSamples = 0;
		}
		else {
			frameTimeSamples++;
			double currentTime = glfwGetTime();
			frameTime += currentTime - lastTime;
			lastTime = currentTime;
		}

		checkGLErrors("End draw: ");

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

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

