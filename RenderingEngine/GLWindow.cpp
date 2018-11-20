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
//Convex Hull
#include "ConvexHull.h"

//Random
#include <random>
#include <ctime>

#include <limits>

#include <glm/gtc/matrix_transform.hpp>


using namespace renderlib;

TrackballCamera cam(
	vec3(0, 0, -1), vec3(0, 0, 2),
	glm::perspective(90.f*3.14159f/180.f, 1.f, 0.1f, 100.f));

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
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
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

float laplacian(float left, float center, float right) { return -2.f*center + left + right; }

void WindowManager::waveSimulationLoop(int numSegments, float dt) {
	glfwSetKeyCallback(window, keyCallback);
	glfwSetCursorPosCallback(window, cursorPositionCallback);
	glfwSetWindowSizeCallback(window, windowResizeCallback);

	vector<float> pressuresT0(numSegments, 0);
	vector<float> pressuresT1(numSegments, 0);
	vector<float> pressuresT2(numSegments, 0);

	float height = 10.f;
	float leftEdge = -1.f;
	float width = 2.f;

	for (int i = 0; i < pressuresT0.size(); i++) {
		float u = float(i) / float(pressuresT0.size()-1);
		//pressuresT1[i] = pressuresT0[i] = sin(M_PI*6.f*u)*0.01f;
	}

	auto geometry = make_shared<SimpleGeometry>(GL_LINE_STRIP);
	Drawable waveDrawable(geometry, make_shared<ColorMat>(vec3(1, 0, 0)));
	SimpleShader simpleShader;

	float C = 0.5f;
	float ALPHA = 0.000001f;

	//pressuresT0[pressuresT0.size()/2] = pressuresT1[pressuresT0.size()/2] = 0.1f;

	float timeElapsed = 0.f;

	glClearColor(0.f, 0.f, 0.f, 1.f);

	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (windowResized) {
			window_width = windowWidth;
			window_height = windowHeight;
			glViewport(0, 0, window_width, window_height);
			cam.projection = glm::perspective(90.f*3.14159f / 180.f, float(window_width)/float(window_height), 0.1f, 100.f);
		}


		float height = 10.f;
		float leftEdge = -1.f;
		float width = 2.f;
		float step = width / float(numSegments - 1);

		
		float frameTime = 0.f;
		while (frameTime < 1.f / 60.f) {
			float leftBoundary = 0.f;
			if (C*C*timeElapsed*timeElapsed*5.f < M_PI*10.f)
				leftBoundary = (sin(C*C*timeElapsed*timeElapsed*5.f)) / (C*timeElapsed + 0.1f)*0.01f;
			float rightBoundary = 0.f;	//-(pressuresT1[pressuresT1.size() - 1] - pressuresT0[pressuresT0.size() - 1])+pressuresT1[pressuresT1.size()-1]; //0.f;
			timeElapsed += dt;
			frameTime += dt;

			for (int i = 0; i < numSegments; i++) {

				//Laplacian
				float p0 = pressuresT0[i];
				float p1 = pressuresT1[i];

				//if (p0 > 0)
					//p0 += 0.00001f;

				float l0 = laplacian(
					(i > 0) ? pressuresT0[i - 1] : leftBoundary,
					p0,
					(i < numSegments - 1) ? pressuresT0[i + 1] : rightBoundary
				)/(step*step);
				float l1 = laplacian(
					(i > 0) ? pressuresT1[i - 1] : leftBoundary,
					p1,
					(i < numSegments - 1) ? pressuresT1[i + 1] : rightBoundary
				)/(step*step);

				pressuresT2[i] =
					2.f*p1 + (C*C*dt*dt + C*ALPHA*dt)*l1
					- p0 - C*ALPHA*dt*l0;
					
			}

			pressuresT0 = pressuresT1;
			pressuresT1 = pressuresT2;
		}
		//Update past pressures
		//pressuresT2[0] += sin(timeElapsed)*0.1f;
		//pressuresT0[pressuresT0.size() / 2] = pressuresT1[pressuresT0.size() / 2] = 0.1f;

		vector<vec3> points;
		for (int i = 0; i < numSegments; i++) {
			points.push_back(vec3(leftEdge + float(i)*step, pressuresT2[i] * height, 0.f));
		}
		geometry->loadGeometry(points.data(), points.size());

		simpleShader.draw(cam, waveDrawable);

		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	glfwTerminate();
}

float floatRand() {
	return float(rand()) / float(RAND_MAX);
}

int intRand(int range) {
	return rand() % range;
}

void loadGeometryWithHalfEdgeMesh(SimpleGeometry* geom, HalfEdgeMesh<vec3>& mesh) {
	vector<vec3> points;

	for (auto f : mesh.faces) {
		vec3 center = mesh.center(f);
		vec3 normal = mesh.normal(f);
		auto e = f.edge;

		int count = 0;
		do {
//			points.push_back(center + normal*0.01f);
//			points.push_back(mesh[mesh[e].head].pos + normal*0.01f);
			points.push_back(mesh[mesh[e].head].pos);
			points.push_back(mesh[mesh[mesh[e].next].head].pos);
			points.push_back(mesh[mesh[mesh[e].next].head].pos);
			points.push_back(mesh[mesh[mesh[mesh[e].next].next].head].pos);
			points.push_back(mesh[mesh[mesh[mesh[e].next].next].head].pos);
			points.push_back(mesh[mesh[e].head].pos);

			points.push_back(center + normal*0.01f);
			points.push_back(center + normal*0.5f);
			e = mesh[e].next;
			count++;
		} while (e != f.edge && count < 10);

		if (count >= 10) printf("Face with > 10 sides detected\n");
	}

	geom->loadGeometry(points.data(), points.size());
}

template<typename T>
bool allDistinct(T* arr, size_t size) {
	for (int i = 0; i < size-1; i++) {
		for (int j = i+1; j < size; j++) {
			if (arr[i] == arr[j])
				return false;
		}
	}

	return true;
}


void WindowManager::convexTestLoop() {
	srand(13);	//time(0));

	SlotMap<int> testMap;
	testMap.add(1);
	testMap.add(2);
	testMap.add(3);
	testMap.add(4);
	auto last = testMap.add(5);
	testMap.remove(last);

	MeshInfoLoader modelMinfo("models/dragon.obj");
	shared_ptr<ElementGeometry> modelGeom = make_shared<ElementGeometry>(modelMinfo.vertices.data(), modelMinfo.normals.data(), nullptr, 
		modelMinfo.indices.data(), modelMinfo.vertices.size(), modelMinfo.indices.size());

	Drawable modelDrawable(modelGeom, make_shared<ColorMat>(vec3(0, 1, 0)));
	modelDrawable.addMaterial(new ShadedMat(0.3f, 0.4f, 0.4f, 5.f));

	for (auto it = testMap.begin(); it != testMap.end(); ++it) {
		//auto ending = testMap.end();
  		printf("%d\n", *it);
	}

	std::vector<int>::iterator it;

	cam = TrackballCamera(
		vec3(0, 0, -1), vec3(0, 0, 5),
		glm::perspective(90.f*3.14159f / 180.f, 1.f, 0.1f, 150.f));

	//Find starting tetrahedron for model
	vec3 tetPoints[4];
	do {
		tetPoints[0] = modelMinfo.vertices[rand() % modelMinfo.vertices.size()];
		tetPoints[1] = modelMinfo.vertices[rand() % modelMinfo.vertices.size()];
		tetPoints[2] = modelMinfo.vertices[rand() % modelMinfo.vertices.size()];
		tetPoints[3] = modelMinfo.vertices[rand() % modelMinfo.vertices.size()];
	} while (!allDistinct(tetPoints, 4));

	HalfEdgeMesh<glm::vec3> mesh;
	//auto vert = generateTetrahedron(mesh, glm::vec3(0, 0, 0), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
	auto vert = generateTetrahedron(mesh, tetPoints[0], tetPoints[1], tetPoints[2], tetPoints[3]);

	const int POINT_NUM = 1000;
	std::vector<glm::vec3> hullPoints;
	for (int i = 0; i < POINT_NUM; i++) {
		hullPoints.push_back(
			(glm::vec3(
			floatRand(),
			floatRand(),
			floatRand()) + glm::vec3(-0.5, -0.5, -0.5)
				)*4.f);
	}

	//convexHullIteration(mesh, hullPoints);

	std::vector<glm::vec3> points;
	std::vector<unsigned int> indices;
	halfEdgeToFaceList(&points, &indices, mesh);
	//faceListToHalfEdge(&mesh, points, indices);
	//points.clear(); indices.clear();
	//halfEdgeToFaceList(&points, &indices, mesh);
	std::vector<glm::vec3> normals = calculateNormalsImp(&points, &indices);

	glfwSetKeyCallback(window, keyCallback);
	glfwSetWindowSizeCallback(window, windowResizeCallback);

	SimpleTexManager tm;
	SimpleShader simpleShader;
	BlinnPhongShader bpShader;
	shared_ptr<ElementGeometry> geom = make_shared<ElementGeometry>(points.data(), normals.data(), nullptr, indices.data(), points.size(), indices.size());
	//enum {POSITIONS=0, NORMALS};
	//StreamGeometry<vec3, vec3> geom({ false, false })

	Drawable heObject(geom, make_shared<ColorMat>(vec3(1, 0, 0)));
	heObject.addMaterial(new ShadedMat(0.3, 0.5, 0.4, 10.f));



	glPointSize(3.f);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	Drawable hullObject(make_shared<SimpleGeometry>(hullPoints.data(), hullPoints.size(), GL_POINTS), 
		make_shared<ColorMat>(vec3(1, 1, 1)));

	vector<vec3> debugPoints;
	shared_ptr<SimpleGeometry> debugGeometry = make_shared<SimpleGeometry>(GL_POINTS);
	Drawable debugObject(debugGeometry, make_shared<ColorMat>(vec3(0.2, 0.5, 1)));

	shared_ptr<SimpleGeometry> halfEdgeDebugGeometry = make_shared<SimpleGeometry>(GL_LINES);
	Drawable halfEdgeDebugObject(halfEdgeDebugGeometry, make_shared<ColorMat>(vec3(1.f, 0.f, 1.f)));

	shared_ptr<SimpleGeometry> halfEdgeTrackingGeometry = make_shared<SimpleGeometry>(GL_LINES);
	Drawable halfEdgeTrackingObject(halfEdgeTrackingGeometry, make_shared<ColorMat>(vec3(0, 1.f, 0.f)));
	SlotMap<HalfEdge<vec3>>::Index trackingEdge = mesh.edges.begin().toIndex();

	glClearColor(0.f, 0.f, 0.f, 0.f);

	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		static int iterationStep = 1;
		static bool iterationPressed = false;
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
			
			if (!iterationPressed) {
				iterationStep = (iterationStep + 1)%2;

				using namespace glm;
				convexHullIteration(mesh, modelMinfo.vertices);//*/		hullPoints);
				/*
				//Expanded function here vvvvvvvvvvvvvvvvvvvvvvv
				{
					static SlotMap<HalfEdge<vec3>>::Index boundaryEdge2;
					static vec3 furthestPoint = vec3(0);

					if (iterationStep == 0) {
						debugPoints.clear();

						const float MINIMUM_PROJECTED_DISTANCE = 0.2f;
						float maxProjectedDistance = 0.f;
						const int MIN_FACES_CHECKED = 3;
						const int MAX_FACES_CHECKED = 10;
						int facesChecked = 0;
						do {
							SlotMap<Face<vec3>>::Index extrudedFace = mesh.faces.random();
							vec3 faceCenter = mesh.center(mesh[extrudedFace]);
							vec3 faceNormal = mesh.normal(mesh[extrudedFace]);
							for (vec3 point : hullPoints) {
								float projectedDistance = dot(faceNormal, point - faceCenter);
								if (projectedDistance > maxProjectedDistance) {
									maxProjectedDistance = projectedDistance;
									furthestPoint = point;
								}
							}

							facesChecked++;
						} while (
							(maxProjectedDistance <= MINIMUM_PROJECTED_DISTANCE || facesChecked >= MIN_FACES_CHECKED) 
							&& facesChecked <= MAX_FACES_CHECKED);
						
						//printf("Distance %f\n------------\n", faceNormal.x, faceNormal.y, faceNormal.z, maxProjectedDistance);

						debugPoints.push_back(furthestPoint);

						std::vector<SlotMap<Face<vec3>>::Index> faceDeleteList;
						for (auto face = mesh.faces.begin(); face != mesh.faces.end(); ++face) {
							if (dot(mesh.normal(*face), furthestPoint - mesh.center(*face)) > 0.f) {
								faceDeleteList.push_back(face.toIndex());	// boundaryEdge = mesh.deleteFace(face.toIndex());
								debugPoints.push_back(mesh.center(*face));
							}
						}
						printf("");
						//SlotMap<HalfEdge<vec3>>::Index boundaryEdge2;	// = SlotMap<HalfEdge<vec3>>::Index();
						for (auto f : faceDeleteList) {
							boundaryEdge2 = mesh.deleteFace(f);
						}
						printf("");
					}
					else if (iterationStep == 1) {
						if (boundaryEdge2) {
							fillBoundary(mesh, boundaryEdge2, furthestPoint);
						}
					}

					trackingEdge = mesh.edges.begin().toIndex();
					vec3 endPoints[2] = { mesh[mesh[trackingEdge].head].pos, mesh[mesh[mesh.prev(trackingEdge)].head].pos };
					halfEdgeTrackingGeometry.loadGeometry(endPoints, 2);
				}
				//Expanded function here ^^^^^^^
				*/

				points.clear(); indices.clear();
				halfEdgeToFaceList(&points, &indices, mesh);
				normals = calculateNormalsImp(&points, &indices);
				vector<vec2> texCoords(normals.size());
				geom->loadGeometry(points.data(), normals.data(), texCoords.data(), &indices[0], points.size(), indices.size());
				debugGeometry->loadGeometry(debugPoints.data(), debugPoints.size());
				
			}
			
			iterationPressed = true;
		}
		else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE){
			iterationPressed = false;
		}

		static bool saveButtonPressed = false;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && !saveButtonPressed) {
			saveButtonPressed = true;
			writeToPly("convexhull.ply", &points, &indices);
		}
		else if (glfwGetKey(window, GLFW_KEY_S) != GLFW_PRESS && saveButtonPressed)
			saveButtonPressed = false;

		static bool pKeyPressed = false;
		if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !pKeyPressed) {
			pKeyPressed = true;
			vec3 normal = (!mesh.isBoundary(trackingEdge)) ? mesh.normal(mesh[mesh[trackingEdge].face]) : vec3(0.f);
			trackingEdge = mesh[trackingEdge].pair;
			vec3 endPoints[2] = { mesh[mesh[trackingEdge].head].pos + normal*0.01f, mesh[mesh[mesh.prev(trackingEdge)].head].pos + normal*0.01f };
			halfEdgeTrackingGeometry->loadGeometry(endPoints, 2);

			//Temporary testing
			vec3 furthestPoint;
			float maxProjectedDistance = 0.f;
			SlotMap<Face<vec3>>::Index extrudedFace = mesh[trackingEdge].face;	//mesh.faces.random();
			vec3 faceCenter = mesh.center(mesh[extrudedFace]);
			vec3 faceNormal = mesh.normal(mesh[extrudedFace]);
			for (vec3 point : hullPoints) {
				float projectedDistance = dot(faceNormal, point - faceCenter);
				if (projectedDistance > maxProjectedDistance) {
					maxProjectedDistance = projectedDistance;
					furthestPoint = point;
				}
			}

			vec3 arr[2] = { furthestPoint, faceCenter };
			debugGeometry->loadGeometry(arr, 2);
		}
		else if(glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE){
			pKeyPressed = false;
		}

		static bool nKeyPressed = false;
		if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS && !nKeyPressed) {
			nKeyPressed = true;
			vec3 normal = (!mesh.isBoundary(trackingEdge)) ? mesh.normal(mesh[mesh[trackingEdge].face]) : vec3(0.f);
			trackingEdge = mesh[trackingEdge].next;
			vec3 endPoints[2] = { mesh[mesh[trackingEdge].head].pos + normal*0.01f, mesh[mesh[mesh.prev(trackingEdge)].head].pos + normal*0.01f };
			halfEdgeTrackingGeometry->loadGeometry(endPoints, 2);
		}
		else if(glfwGetKey(window, GLFW_KEY_N) == GLFW_RELEASE){
			nKeyPressed = false;
		}

		static bool sKeyPressed = false;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && !sKeyPressed) {


		}

		if (windowResized) {
			window_width = windowWidth;
			window_height = windowHeight;
			glViewport(0, 0, window_width, window_height);
		}

		loadGeometryWithHalfEdgeMesh(halfEdgeDebugGeometry.get(), mesh);

		simpleShader.draw(cam, halfEdgeTrackingObject);
//		simpleShader.draw(cam, hullObject);
		simpleShader.draw(cam, debugObject);
		simpleShader.draw(cam, halfEdgeDebugObject);
		bpShader.draw(cam, glm::vec3(10, 10, 10), heObject);
		bpShader.draw(cam, glm::vec3(10, 10, 10), modelDrawable);

		glfwSwapBuffers(window);
		glfwWaitEvents();
	}

	glfwTerminate();
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
	if (!lightFramebuffer.addTexture(createTexture2D(TexInfo(GL_TEXTURE_2D, { window_width, window_height }, 0,
		GL_RGBA, GL_RGB32F, GL_FLOAT), &tm), GL_COLOR_ATTACHMENT0)
		|| !lightFramebuffer.addTexture(createDepthTexture(window_width, window_height, &tm), GL_DEPTH_ATTACHMENT)) {
		//){
		printf("Failed to initialize lightFramebuffer\n");
	}

	Framebuffer gaussianFramebuffer = createNewFramebuffer(window_width, window_height);
	if (!gaussianFramebuffer.addTexture(createTexture2D(TexInfo(GL_TEXTURE_2D, { window_width, window_height }, 0,
		GL_RGBA, GL_RGB32F, GL_FLOAT), &tm), GL_COLOR_ATTACHMENT0)
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

	Drawable glowDragon(
		objToElementGeometry("models/dragon.obj"),
		new ColorMat(vec3(0.75f, 0.1f, 0.3f)*40.f));
	//dragon.addMaterial(new ShadedMat(0.2f, 0.5f, 0.3f, 10.f));

	const int N = 301;
	const float SIGMA = 0.4f*float(N + 1);

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
		flatShader.draw(cam, glowDragon);

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

const float TROPISM = 0.0f;
const float SEARCH_RADIUS = 0.2f;
const float SEARCH_FALLOFF = 0.f;
const float GROWTH_SPEED = 0.02f;
const float HEADING_WEIGHT = 1.f;
const float BRANCHING_WEIGHT = 4.f;
const float SECOND_BRANCH_WEIGHT = 0.5f;
const float BRANCHING_DELAY = 4.f;

struct TreeNode {
	vec3 pos;
	vec3 normal;
	float radius;
	shared_ptr<TreeNode> prev;

	TreeNode(vec3 _pos, float _radius, shared_ptr<TreeNode> _prev) :pos(_pos), normal(vec3(0)), radius(_radius), prev(_prev) {}
};

vec3 getCylinderJoint(TreeNode current) {
	if (length(current.prev->normal) > 0)
		return current.prev->normal;

	vec3 v2 = normalize(current.pos - current.prev->pos);
	vec3 v1 = normalize(current.prev->prev->pos - current.prev->pos);

	return normalize(-v1 - v2)*current.prev->radius;
}

vec3 getCapVector(TreeNode current, vec3 prevVector) {
	vec3 cylinderDirection = normalize(current.pos - current.prev->pos);
	vec3 projVector = dot(prevVector, cylinderDirection)*cylinderDirection;
	vec3 capVector = normalize(prevVector - projVector)*current.radius;

	return capVector;
}

vec3 getCylinderDirection(TreeNode current) {
	return normalize(current.pos - current.prev->pos);
}

vector<vec3> generateSpacePoints(int numPoints) {
	vector<vec3> spacePoints;
	srand(time(0));
	//mat3 rotateMatrix(cos(M_PI / 4.f), -sin(M_PI / 4.f), 0.f, sin(M_PI / 4.f), cos(M_PI / 4.f), 0.f, 0.f, 0.f, 1.f);
	for (int i = 0; i < numPoints; i++) {
		spacePoints.push_back(vec3(
			1.f*(floatRand() - 0.5f),
			2.f*(floatRand() - 0.5f),
			1.f*(floatRand() - 0.5f)));
	}

	return spacePoints;
}

vector<vec3> removeNearbyPoints(const vector<vec3>& removedVector, const vector<shared_ptr<TreeNode>>& points, float radius) {
	vector<vec3> returnVector;
	for (vec3 vectorPoint : removedVector) {
		bool remove = false;
		for (shared_ptr<TreeNode> point : points) {
			if (dot(point->pos - vectorPoint, point->pos - vectorPoint) < 4.f*point->radius*point->radius){//radius*radius*) {
				remove = true;
				break;
			}
		}

		if (!remove)
			returnVector.push_back(vectorPoint);
	}

	return returnVector;
}

vector<vec3> getDirectionVectors(const vector<vec3>& points, vec3 origin, float radius, float falloff) {
	vector<vec3> directionVectors;

	for (vec3 point : points) {
		vec3 vec = point - origin;
		float vec_length_squared = dot(vec, vec);
		if (vec_length_squared < radius*radius) {
			float vec_length = sqrt(vec_length_squared);
			vec = vec * pow((radius - vec_length) / radius, falloff)/ vec_length;
			directionVectors.push_back(vec);
		}
	}

	return directionVectors;
}

vec3 powerIteration(mat3 matrix, vec3 startVector, vector<vec3> eigenvectors, int iterations) {
	vec3 previousEigenvector = vec3(0);
	vec3 currentEigenvector = startVector;

	for (vec3& eigenvector : eigenvectors) {
		eigenvector = normalize(eigenvector);
	}

	for (int i = 0; i < iterations; i++) {
		vec3 newEigenvector = matrix*normalize(currentEigenvector);
		previousEigenvector = normalize(currentEigenvector);

		//Remove other eigenvectors
		for (vec3 eigenvector : eigenvectors) {
			newEigenvector -= dot(eigenvector, newEigenvector)*eigenvector;
		}
		currentEigenvector = newEigenvector;
	}

	return normalize(currentEigenvector)*length(currentEigenvector) / length(previousEigenvector);
}

mat3 calculateCovariance(const vector<vec3>& data) {
	mat3 covariance (0);

	for (int i = 0; i < 3; i++) {
		for (int j = i; j < 3; j++) {
			for (vec3 point : data) {
				covariance[i][j] += point[i] * point[j];
				covariance[j][i] += point[i] * point[j];
			}
		}
	}

	return covariance / float(data.size() - 1);
}

mat3 getEigenvectors(const vector<vec3>& data, vec3 point, float radius) {
	vector<vec3> directionVectors = getDirectionVectors(data, point, radius, SEARCH_FALLOFF);
	if (directionVectors.size() < 10)
		return mat3(0);
	mat3 covariance = calculateCovariance(directionVectors);
	vec3 axis0 = powerIteration(covariance, vec3(1, 0, 0), {}, 30);
	vec3 axis1 = powerIteration(covariance, vec3(0, 1, 0), { axis0 }, 30);
	vec3 axis2 = powerIteration(covariance, vec3(0, 0, 1), { axis0, axis1 }, 30);

	vec3 average(0);
	for (vec3 vec : directionVectors) {
		average += vec;
	}
	average /= float(directionVectors.size());
	if (dot(axis0, average) < 0)
		axis0 *= -1.f;
	if (dot(axis1, average) < 0)
		axis1 *= -1.f;
	if (dot(axis2, average) < 0)
		axis2 *= -1.f;

	if (length(axis0) < length(axis1)) {
		vec3 temp = axis1;
		axis1 = axis0;
		axis0 = temp;
	}
	if (length(axis0) < length(axis2)) {
		vec3 temp = axis2;
		axis2 = axis0;
		axis0 = temp;
	}
	if (length(axis1) < length(axis2)) {
		vec3 temp = axis2;
		axis2 = axis1;
		axis1 = temp;
	}

	return mat3(axis0, axis1, axis2);
}	


vector<vec3> traceParticle(const vector<vec3>& points, vec3 particle, vec3 heading, float radius, float speed, bool canSplit) {
	mat3 eigenvectors = getEigenvectors(points, particle+heading*speed, radius);
	vec3 eigenvalues(length(eigenvectors[0]), length(eigenvectors[1]), length(eigenvectors[2]));

	vector<vec3> newParticle; 

	if (eigenvalues[0] < 0.00001f)
		return{};

	vec3 tropism = vec3(0, 1.f, 0)*eigenvalues[0]*TROPISM;

	if (canSplit && eigenvalues[0] > eigenvalues[1]) {
		newParticle.push_back(particle + speed*normalize(
			HEADING_WEIGHT*heading*length(eigenvectors[0]) + eigenvectors[0] + BRANCHING_WEIGHT*eigenvectors[1] + tropism));
		newParticle.push_back(particle + speed*normalize(
			HEADING_WEIGHT*heading*length(eigenvectors[0]) + eigenvectors[0] - SECOND_BRANCH_WEIGHT*BRANCHING_WEIGHT*eigenvectors[1] + tropism));
	}
	else {
		newParticle.push_back(particle + speed*normalize(
			HEADING_WEIGHT*heading*length(eigenvectors[0]) + eigenvectors[0] + tropism));
	}

	return newParticle;
}

vector<vec3> reallyBasicSpaceColonization() {
	vector<vec3> spacePoints = generateSpacePoints(1000);


	return{};

}

vector<pair<GLenum, string>> shaders{
	{GL_VERTEX_SHADER, "shaders/cylinder.vert"},
	{GL_FRAGMENT_SHADER, "shaders/cylinder.frag"},
	{GL_TESS_CONTROL_SHADER, "shaders/cylinder.tesc"},
	{GL_TESS_EVALUATION_SHADER, "shaders/cylinder.tese"}
};

constexpr int cMax(int a, int b) {
	return (a > b) ? a : b;
}

void WindowManager::testLoop() {

	glfwSetKeyCallback(window, keyCallback);
	glfwSetWindowSizeCallback(window, windowResizeCallback);
	//glfwSetCursorPosCallback(window, cursorPositionCallback)

	enum {
		VP_MATRIX_LOCATION = ShadedMat::COUNT
		+ cMax(int(TextureMat::COUNT), int(ColorMat::COUNT)),
		M_MATRIX_LOCATION,
		CAMERA_POS_LOCATION,
		LIGHT_POS_LOCATION,
		COUNT
	};

	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	class CylinderShader : public ShaderT<ColorMat, ShadedMat> {
	public:
		CylinderShader() :
			ShaderT<ColorMat, ShadedMat>(shaders, {},
			{ "color", "ka", "kd", "ks", "alpha",
				"view_projection_matrix", "model_matrix",
				"camera_position", "lightPos" })
		{}

		void draw(const Camera &cam, vec3 lightPos, Drawable& obj) {
			glUseProgram(programID);
			mat4 vp_matrix = cam.getProjectionMatrix()*cam.getCameraMatrix();
			mat4 m_matrix = obj.getTransform();
			vec3 camera_pos = cam.getPosition();

			loadMaterialUniforms(obj);
			glUniformMatrix4fv(uniformLocations[VP_MATRIX_LOCATION], 1, false, &vp_matrix[0][0]);
			glUniformMatrix4fv(uniformLocations[M_MATRIX_LOCATION], 1, false, &m_matrix[0][0]);
			glUniform3f(uniformLocations[CAMERA_POS_LOCATION],
				camera_pos.x, camera_pos.y, camera_pos.z);
			glUniform3f(uniformLocations[LIGHT_POS_LOCATION],
				lightPos.x, lightPos.y, lightPos.z);
			obj.getGeometry().drawGeometry();
			glUseProgram(0);
		}
	};



	vector<vec3> points{
		vec3(0.f, 0.f, 0.f),
		vec3(0.f, 3.f, 0.f),
		vec3(0.f, 4.f, 1.f),
		vec3(1.f, 4.f, 2.f),
		vec3(2.f, 4.f, 2.f)
	};

	/*vector<vec3> normals{
		vec3(2.f, 0.f, 0.f),
		normalize(normalize(points[1] - points[0]) + normalize(points[1] - points[2])),
		normalize(vec3(0, 1, -1))*0.5f
	};*/

	vector<vec3> normals;

	normals.push_back(vec3(1, 0, 0));

	for (int i = 1; i < points.size() - 1; i++) {
		normals.push_back(normalize(
			normalize(points[i] - points[i - 1])
			+ normalize(points[i] - points[i + 1]))
		);

	}

	normals.push_back(normalize(cross(normals.back(), points.back() - points[points.size() - 2])));

	vector<unsigned int> indices{ 0, 1, 1, 2, 2, 3, 3, 4 };

	CylinderShader cShader;
	auto geometry = make_shared<ElementGeometry>(points.data(), normals.data(), nullptr, indices.data(), points.size(), indices.size(), GL_PATCHES);

	Drawable cylinderDrawable(geometry, make_shared<ShadedMat>(0.3, 0.5, 0.4, 10.f));
	cylinderDrawable.addMaterial(new ColorMat(vec3(1, 1, 1)));

	//Terrible code to solve temporary problem
	indices.clear();

	//Space colonization visualization
	vector<vec3> spacePoints = generateSpacePoints(50000);
	/*vec3 meanPoint(0);
	for (vec3 point : spacePoints) {
		meanPoint += point;
	}
	meanPoint /= float(spacePoints.size());

	vector<vec3> nearbyPoints = getDirectionVectors(spacePoints, meanPoint, 2.f, 0.f);
	mat3 covariance = calculateCovariance(nearbyPoints);
	vec3 axis0 = powerIteration(covariance, vec3(1, 0, 0), {}, 30);
	vec3 axis1 = powerIteration(covariance, vec3(0, 1, 0), { axis0 }, 30);
	vec3 axis2 = powerIteration(covariance, vec3(0, 0, 1), { axis0, axis1 }, 30);

	vector<vec3> lines = { meanPoint, axis0, meanPoint, axis1, meanPoint, axis2 };*/

	float START_RADIUS = 0.05f;
	vector<vec3> treePoints = { vec3(0, -1.2, 0), vec3(0, -1, 0) };
	vector<vec3> treeNormals = { vec3(1, 0, 0), vec3(1, 0, 0) };
	auto treeRoot = make_shared<TreeNode>(vec3(0, -1.2, 0), START_RADIUS, nullptr);
	auto firstNode = make_shared<TreeNode>(treePoints[0], START_RADIUS, treeRoot);
	indices = { 0, 1 };
	vector<shared_ptr<TreeNode>> treeEndpoints = { make_shared<TreeNode>(vec3(0, -0.96, 0),  START_RADIUS, firstNode)};
	//vector<vec3> treeHeadings = { vec3(0, 1, 0) };
	vector<float> branchingCountdown = { BRANCHING_DELAY };

	auto spacePointGeometry = make_shared<SimpleGeometry>(spacePoints.data(), spacePoints.size(), GL_POINTS);
	Drawable spacePointDrawable(spacePointGeometry, make_shared<ColorMat>(vec3(1.f, 0.f, 0.f)));
	//Drawable eigenvectorDrawable(new SimpleGeometry(lines.data(), lines.size(), GL_LINES), new ColorMat(vec3(0.f, 1.f, 0.f)));

	auto treeGeometry = make_shared<SimpleGeometry>(treePoints.data(), treePoints.size(), GL_LINES);
	Drawable treeDrawable(treeGeometry, make_shared<ColorMat>(vec3(0, 1, 0)));

	SimpleShader simpleShader;

	//Dragon
	Drawable dragon(
		objToElementGeometry("models/dragon.obj"),
		new ColorMat(vec3(0.75f, 0.1f, 0.3f)));
	dragon.addMaterial(new ShadedMat(0.2f, 0.5f, 0.3f, 10.f));

	vec3 lightPos(10.f, 10.f, 10.f);


	BlinnPhongShaderT bpShader;

	glPatchParameteri(GL_PATCH_VERTICES, 2);

	while (!glfwWindowShouldClose(window)) {
		if (windowResized) {
			window_width = windowWidth;
			window_height = windowHeight;
		}
		glClearColor(0.f, 0.f, 0.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		static bool spacePressed = false;
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !spacePressed) {
			spacePressed = true;
		}
		else if (spacePressed && glfwGetKey(window, GLFW_KEY_SPACE) != GLFW_PRESS) {
			//Space colonization
			float radius = SEARCH_RADIUS;
			float speed = GROWTH_SPEED;
			vector<shared_ptr<TreeNode>> treeNewEndpoints;
			//vector<vec3> treeNewHeadings;
			vector<float> newBranchingCountdown;
			for (int i = 0; i < treeEndpoints.size(); i++) {
				vector<vec3> newParticles = traceParticle(spacePoints, treeEndpoints[i]->pos, getCylinderDirection(*treeEndpoints[i]), 
					radius, speed, branchingCountdown[i] <= 0.f);
				for (int j = 0; j < newParticles.size(); j++) {
					vec3 particle = newParticles[j];
					float newRadius = (newParticles.size() == 2 && (j == 0 || SECOND_BRANCH_WEIGHT > 0.f)) 
						? treeEndpoints[i]->radius*0.5f : treeEndpoints[i]->radius*0.97f;
					treeNewEndpoints.push_back(make_shared<TreeNode>(particle, newRadius, treeEndpoints[i]));
					//treeNewHeadings.push_back(normalize(particle - treeEndpoints[i]));
					treePoints.push_back(treeEndpoints[i]->prev->pos);
					treeNormals.push_back(getCylinderJoint(*treeEndpoints[i]));
					treePoints.push_back(treeEndpoints[i]->pos);
					if (newParticles.size() > 2)
						treeEndpoints[i]->normal = treeNormals.back();
					treeNormals.push_back(getCylinderJoint(*treeNewEndpoints.back()));
					indices.push_back(indices.size());
					indices.push_back(indices.size());
					//treePoints.push_back(particle);
					newBranchingCountdown.push_back((newParticles.size() == 1) ? branchingCountdown[i] - floatRand()*2.f : BRANCHING_DELAY);
				}
			}

			branchingCountdown.swap(newBranchingCountdown);
			treeEndpoints.swap(treeNewEndpoints);
			//treeHeadings.swap(treeNewHeadings);
			//treeGeometry->loadGeometry(treePoints.data(), treePoints.size());
 			geometry->loadGeometry(treePoints.data(), treeNormals.data(), nullptr, indices.data(), treePoints.size(), indices.size());

			vector<vec3> treeEndpointsVec3;
			for (shared_ptr<TreeNode> node : treeEndpoints) {
				treeEndpointsVec3.push_back(node->pos);
			}
			
			spacePoints = removeNearbyPoints(spacePoints, treeEndpoints, speed*3.f);
			spacePointGeometry->loadGeometry(spacePoints.data(), spacePoints.size());
			printf("spacePoints.size() = %d\n", spacePoints.size());
			spacePressed = false;
		}

		//bpShader.draw(cam, lightPos, dragon);
		cShader.draw(cam, lightPos, cylinderDrawable);
		simpleShader.draw(cam, spacePointDrawable);
		//simpleShader.draw(cam, eigenvectorDrawable);
		//simpleShader.draw(cam, treeDrawable);

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
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
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

