#include "GLWindow.h"

#include <iostream>

using namespace glm;
using namespace std;

#include "ModelLoader.h"
#include "Drawable.h"
#include "SimpleShader.h"
#include "ColorMat.h"
#include "TrackballCamera.h"
#include "SphericalCamera.h"
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
#include "TemplatedGeometry.h"
#include "CommonGeometry.h"

//Rigid body test
#include "Physics.h"

//Glow test
#include "GlowShader.h"
//Convex Hull
#include "ConvexHull.h"

#include "AdaptiveNoise.h"
#include "noiseTest.h"
#include "VertexVertex.h"
#include "CommonGeometry.h"
#include "FontToTexture.h"
#include "TextShader.h"

#include "ShadowShader.h"

#include "glmSupport.h"

//Random
#include <random>
#include <ctime>
#include <numeric>
#include <algorithm>

#include <limits>

#include <glm/gtc/matrix_transform.hpp>

#include "PetioleAlignment.h"

using namespace renderlib;

using PositionGeometry = GeometryT<attrib::Position>;
using PositionNormalGeometry = GeometryT<attrib::Position, attrib::Normal>;


/*TrackballCamera cam(
	vec3(0, 0, -1), vec3(0, 0, 2),
	glm::perspective(90.f*3.14159f/180.f, 1.f, 0.1f, 100.f));
	*/
SphericalCamera cam(
	glm::perspective(90.f*3.14159f / 180.f, 1.f, 0.1f, 100.f),
	0.f, 0.f, 1.f);

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
		cam.right(-diff.x*3.14159f);
		cam.up(-diff.y*3.14159f);
	}
	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
		vec2 diff = mousePos - lastPos;
		cam.zoom(pow(2.f, diff.y));
	}

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
		vec2 diff = mousePos - lastPos;
		cam.center -= diff.x*cam.right() + diff.y*cam.up();
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
	glfwWindowHint(GLFW_SAMPLES, 8);

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

void WindowManager::velocitySpringLoop() {
	//glfwSetCursorPosCallback(window, )
	

	BlinnPhongShaderT bpShader;
	Drawable test(
		createSphereGeometry(),
		make<ColorMat>(vec3(0, 1, 0)));
	test.addMaterial(make<ShadedMat>(0.2f, 0.4f, 0.5f, 20.f));
	test.setScale(glm::vec3(0.5f));

	const int smoothedsamples = 5;
	vec2 mouseVelocity (0.f);
	std::vector<vec2> mouseVelocityBuffer;
	mouseVelocityBuffer.resize(5, vec2(0.f));
	vec2 mousePosition;

	vec2 springPosition(0.f);
	vec2 springVelocity(0.f);
	vec2 springAcceleration(0.f);

	int vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);

	glfwSwapInterval(1);

	float dt = 1.f / 60.f;
	float k = 100.f;
	float damping = 1000.f;

	vec3 line[2] = { glm::vec3(0.f, 0.f, 0.f), vec3(0.f, 0.f, 0.f) };
	//auto velocityGeometry = std::make_shared<SimpleGeometry>(line, 2, GL_LINES);
	auto velocityGeometry = std::make_shared<PositionGeometry>(GL_LINES, line, 2);
	Drawable velocityLine(velocityGeometry, std::make_shared<ColorMat>(vec3(0.f, 1.f, 0.f)));

	vec3 position(0.f);
	auto springGeometry = std::make_shared<PositionGeometry>(GL_POINTS, &position, 1);
	Drawable spring(
		springGeometry, std::make_shared<ColorMat>(vec3(1, 0, 0)));
	
	FlatColorShader shader;

	Camera cam;

	glClearColor(0.f, 0.f, 1.f, 0.f);
	glPointSize(10.f);

	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		double x, y;
		glfwGetCursorPos(window, &x, &y);
		vec2 currentMousePosition = vec2(x / float(vp[2]), -y / float(vp[3]))*2.f - vec2(1, -1);
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
			mouseVelocityBuffer.push_back(currentMousePosition - mousePosition);
		}
		else
			mouseVelocityBuffer.push_back(vec2(0, 0));
		mousePosition = currentMousePosition;
		//Update velocity
		if(mouseVelocityBuffer.size() > smoothedsamples)
			mouseVelocityBuffer.erase(mouseVelocityBuffer.begin());
		mouseVelocity = std::accumulate(mouseVelocityBuffer.begin(), mouseVelocityBuffer.end(), vec2(0.f));
		mouseVelocity /= float(mouseVelocityBuffer.size());

		springAcceleration += dt*(k*(mouseVelocity - springVelocity) - springVelocity*damping);
		springVelocity += springAcceleration*dt;
		springPosition += springVelocity * dt;

		line[0] = vec3(springPosition, 0.f);
		line[1] = vec3(springPosition + springVelocity, 0.f);
		velocityGeometry->loadBuffers(line, 2);

		printf("Mouse velocity = %f %f\nSpring velocity = %f %f acceleration = %f %f\n", 
			mouseVelocity.x, mouseVelocity.y,  
			springVelocity.x, springVelocity.y, 
			springAcceleration.x, springAcceleration.y);

		spring.position = vec3(springPosition, 0.f);

		bpShader.draw(cam, vec3(10.f, 10.f, 10.f), test);
		shader.draw(cam, spring);
		shader.draw(cam, velocityLine);

		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	glfwTerminate();
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

	auto geometry = make_shared<PositionGeometry>(GL_LINE_STRIP);
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
		geometry->loadBuffers(points.data(), points.size());

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

void loadGeometryWithHalfEdgeMesh(GeometryT<attrib::Position>* geom, HalfEdgeMesh<vec3>& mesh) {
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

	geom->loadBuffers(points.data(), points.size());
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
	srand(time(0));

	SlotMap<int> testMap;
	testMap.add(1);
	testMap.add(2);
	testMap.add(3);
	testMap.add(4);
	auto last = testMap.add(5);
	testMap.remove(last);

	//MeshInfoLoader modelMinfo("models/dragon.obj");
	//MeshInfoLoader modelMinfo("untrackedmodels/181203_kale_01_use.obj");
	MeshInfoLoader modelMinfo;
	modelMinfo.loadModelPly("untrackedmodels/Helianthus.ply");
	auto modelGeom = make<IndexGeometryUint<attrib::Position, attrib::Normal>>(GL_TRIANGLES, modelMinfo.indices.data(), modelMinfo.indices.size(), modelMinfo.vertices.data(), modelMinfo.normals.data(), modelMinfo.vertices.size());

	Drawable modelDrawable(modelGeom, make_shared<ColorMat>(vec3(0, 1, 0)));
	modelDrawable.addMaterial(new ShadedMat(0.3f, 0.4f, 0.4f, 5.f));

	for (auto it = testMap.begin(); it != testMap.end(); ++it) {
		//auto ending = testMap.end();
  		printf("%d\n", *it);
	}

	std::vector<int>::iterator it;

	/*
	cam = TrackballCamera(
		vec3(0, 0, -1), vec3(0, 0, 5),
		glm::perspective(90.f*3.14159f / 180.f, 1.f, 0.1f, 1000.f));
		*/

	cam.projection = glm::perspective(90.f*3.14159f / 180.f, 1.f, 0.1f, 1000.f),
	cam.center = glm::vec3(0, 0, -1);
	cam.zoom(5.f);

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
	auto geom = make<IndexGeometryUint<attrib::Position, attrib::Normal>>(
		GL_TRIANGLES, indices.data(), indices.size(), points.data(), normals.data(), points.size());
	//enum {POSITIONS=0, NORMALS};
	//StreamGeometry<vec3, vec3> geom({ false, false })

	Drawable heObject(geom, make_shared<ColorMat>(vec3(1, 0, 0)));
	heObject.addMaterial(new ShadedMat(0.3, 0.5, 0.4, 10.f));



	glPointSize(3.f);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	Drawable hullObject(make<GeometryT<attrib::Position>>(GL_POINTS, hullPoints.data(), hullPoints.size()),
		make_shared<ColorMat>(vec3(1, 1, 1)));

	vector<vec3> debugPoints;
	auto debugGeometry = make<GeometryT<attrib::Position>>(GL_POINTS);
	Drawable debugObject(debugGeometry, make_shared<ColorMat>(vec3(0.2, 0.5, 1)));

	auto halfEdgeDebugGeometry = make<GeometryT<attrib::Position>>(GL_LINES);
	Drawable halfEdgeDebugObject(halfEdgeDebugGeometry, make_shared<ColorMat>(vec3(1.f, 0.f, 1.f)));

	auto halfEdgeTrackingGeometry = make<GeometryT<attrib::Position>>(GL_LINES);
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
				geom->loadIndices(indices.data(), indices.size());
				geom->loadBuffers(points.data(), normals.data(), points.size());
				debugGeometry->loadBuffers(debugPoints.data(), debugPoints.size());
				
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
			halfEdgeTrackingGeometry->loadBuffers(endPoints, 2);

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
			debugGeometry->loadBuffers(arr, 2);
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
			halfEdgeTrackingGeometry->loadBuffers(endPoints, 2);
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

constexpr int intMod(int x, int n) {
	return (x + n) % n;
}

struct BranchingStructureData {
	std::vector<vec3> points;
	std::vector<unsigned int> faces;
	std::vector<unsigned int> fixedPoints;
};

BranchingStructureData generateBranchingStructure(
	vec3 b0, float r0,
	vec3 b1, float r1,
	vec3 b2, float r2,
	vec3 junction, unsigned int divisions) {
	vec3 planeNormal = vec3(0);
	vec3 crossResult = cross(b0, b1);
	if (length(crossResult) > 0.0001f) planeNormal += crossResult;
	crossResult = cross(b1, b2);
	if (length(crossResult) > 0.0001f) planeNormal += crossResult;
	crossResult = cross(b2, b0);
	if (length(crossResult) > 0.0001f) planeNormal += crossResult;
	planeNormal = normalize(planeNormal);

	float radius = (r0 + r1 + r2)*0.33333f;

	vector<vec3> points;
	points.push_back(junction + planeNormal * radius);	//Front center
	points.push_back(junction - planeNormal * radius);	//Back center

	unsigned int b0BoundaryIndexMap[8];
	unsigned int b1BoundaryIndexMap[8];
	unsigned int b2BoundaryIndexMap[8];

	float lateralStep = 1.f / float(divisions+1);

	b0BoundaryIndexMap[0] = points.size();
	points.push_back(junction + planeNormal * radius + b0 * lateralStep);
	b0BoundaryIndexMap[4] = points.size();
	points.push_back(junction - planeNormal * radius + b0 * lateralStep);

	b1BoundaryIndexMap[0] = points.size();
	points.push_back(junction + planeNormal * radius + b1 * lateralStep);
	b1BoundaryIndexMap[4] = points.size();
	points.push_back(junction - planeNormal * radius + b1 * lateralStep);

	b2BoundaryIndexMap[0] = points.size();
	points.push_back(junction + planeNormal * radius + b2 * lateralStep);
	b2BoundaryIndexMap[4] = points.size();
	points.push_back(junction - planeNormal * radius + b2 * lateralStep);

	vec3 b0b1dir = 0.333f*(b0 + b1 - b2);
	b0BoundaryIndexMap[1] = b1BoundaryIndexMap[7] = points.size();
	points.push_back(junction + planeNormal * radius + b0b1dir * lateralStep);
	b0BoundaryIndexMap[2] = b1BoundaryIndexMap[6] = points.size();
	points.push_back(junction + b0b1dir * lateralStep);
	b0BoundaryIndexMap[3] = b1BoundaryIndexMap[5] = points.size();
	points.push_back(junction - planeNormal * radius + b0b1dir * lateralStep);

	vec3 b1b2dir = 0.333f*(b1 + b2 - b0);
	b1BoundaryIndexMap[1] = b2BoundaryIndexMap[7] = points.size();
	points.push_back(junction + planeNormal * radius + b1b2dir * lateralStep);
	b1BoundaryIndexMap[2] = b2BoundaryIndexMap[6] = points.size();
	points.push_back(junction + b1b2dir * lateralStep);
	b1BoundaryIndexMap[3] = b2BoundaryIndexMap[5] = points.size();
	points.push_back(junction - planeNormal * radius + b1b2dir * lateralStep);

	vec3 b2b0dir = 0.333f*(b2 + b0 - b1);
	b2BoundaryIndexMap[1] = b0BoundaryIndexMap[7] = points.size();
	points.push_back(junction + planeNormal * radius + b2b0dir * lateralStep);
	b2BoundaryIndexMap[2] = b0BoundaryIndexMap[6] = points.size();
	points.push_back(junction + b2b0dir * lateralStep);
	b2BoundaryIndexMap[3] = b0BoundaryIndexMap[5] = points.size();
	points.push_back(junction - planeNormal * radius + b2b0dir * lateralStep);

	//CENTER TRIANGLES
	//Front side
	std::vector<unsigned int> faces;
	faces.push_back(0);
	faces.push_back(b0BoundaryIndexMap[0]);
	faces.push_back(b0BoundaryIndexMap[1]);
	faces.push_back(0);
	faces.push_back(b0BoundaryIndexMap[7]);
	faces.push_back(b0BoundaryIndexMap[0]);

	faces.push_back(0);
	faces.push_back(b1BoundaryIndexMap[0]);
	faces.push_back(b1BoundaryIndexMap[1]);
	faces.push_back(0);
	faces.push_back(b1BoundaryIndexMap[7]);
	faces.push_back(b1BoundaryIndexMap[0]);
	
	faces.push_back(0);
	faces.push_back(b2BoundaryIndexMap[0]);
	faces.push_back(b2BoundaryIndexMap[1]);
	faces.push_back(0);
	faces.push_back(b2BoundaryIndexMap[7]);
	faces.push_back(b2BoundaryIndexMap[0]);

	//Back side
	faces.push_back(1);
	faces.push_back(b0BoundaryIndexMap[3]);
	faces.push_back(b0BoundaryIndexMap[4]);
	faces.push_back(1);
	faces.push_back(b0BoundaryIndexMap[4]);
	faces.push_back(b0BoundaryIndexMap[5]);

	faces.push_back(1);
	faces.push_back(b1BoundaryIndexMap[3]);
	faces.push_back(b1BoundaryIndexMap[4]);
	faces.push_back(1);
	faces.push_back(b1BoundaryIndexMap[4]);
	faces.push_back(b1BoundaryIndexMap[5]);

	faces.push_back(1);
	faces.push_back(b2BoundaryIndexMap[3]);
	faces.push_back(b2BoundaryIndexMap[4]);
	faces.push_back(1);
	faces.push_back(b2BoundaryIndexMap[4]);
	faces.push_back(b2BoundaryIndexMap[5]);

	std::vector<unsigned int> fixedPoints;
	const unsigned int RADIAL_DIVISIONS = 8;

	//B0 CYLINDER
	unsigned int b0Start = points.size();
	vec3 bx = normalize(cross(planeNormal, b0));
	vec3 by = normalize(cross(b0, bx));

	float lateral = lateralStep * 2.f;
	for (unsigned int i = 0; i < divisions; i++) {
		float theta = 0.f;
		float thetaStep = 2.f*M_PI / float(RADIAL_DIVISIONS);
		for (unsigned int j = 0; j < RADIAL_DIVISIONS; j++) {
			if (i > divisions - 3) fixedPoints.push_back(points.size());

			points.push_back(junction + b0*lateral + (by * cos(theta) + bx * sin(theta))*r0);

			if (i+1 < divisions) {
				unsigned int i00 = b0Start + i * RADIAL_DIVISIONS + j;
				unsigned int i10 = b0Start + (i + 1)* RADIAL_DIVISIONS + j;
				unsigned int i01 = b0Start + i * RADIAL_DIVISIONS + intMod(j + 1, RADIAL_DIVISIONS);
				unsigned int i11 = b0Start + (i + 1) * RADIAL_DIVISIONS + intMod(j + 1, RADIAL_DIVISIONS);
				
				if (j % 2 == 0){ //(j < 4) {
					faces.push_back(i00);
					faces.push_back(i10);
					faces.push_back(i11);
					faces.push_back(i00);
					faces.push_back(i11);
					faces.push_back(i01);
				}
				else {
					faces.push_back(i01);
					faces.push_back(i00);
					faces.push_back(i10);
					faces.push_back(i01);
					faces.push_back(i10);
					faces.push_back(i11);
				}
			}

			theta += thetaStep;
		}
		lateral += lateralStep;
	}

	//Connection between center and b0
	for (int j = 0; j < RADIAL_DIVISIONS; j++) {
		
		unsigned int i00 = b0BoundaryIndexMap[j];
		unsigned int i10 = b0Start + j;
		unsigned int i01 = b0BoundaryIndexMap[intMod(j + 1, RADIAL_DIVISIONS)];
		unsigned int i11 = b0Start + intMod(j+1, RADIAL_DIVISIONS);

		if (j % 2 == 0) { //(j < 4) {
			faces.push_back(i00);
			faces.push_back(i10);
			faces.push_back(i11);
			faces.push_back(i00);
			faces.push_back(i11);
			faces.push_back(i01);
		}
		else {
			faces.push_back(i01);
			faces.push_back(i00);
			faces.push_back(i10);
			faces.push_back(i01);
			faces.push_back(i10);
			faces.push_back(i11);
		}
	}

	//B1 CYLINDER
	unsigned int b1Start = points.size();
	bx = normalize(cross(planeNormal, b1));
	by = normalize(cross(b1, bx));

	lateral = lateralStep * 2.f;
	for (unsigned int i = 0; i < divisions; i++) {
		float theta = 0.f;
		float thetaStep = 2.f*M_PI / float(RADIAL_DIVISIONS);
		for (unsigned int j = 0; j < RADIAL_DIVISIONS; j++) {
			if (i > divisions - 3) fixedPoints.push_back(points.size());

			points.push_back(junction + b1 * lateral + (by * cos(theta) + bx * sin(theta))*r1);

			if (i + 1 < divisions) {
				unsigned int i00 = b1Start + i * RADIAL_DIVISIONS + j;
				unsigned int i10 = b1Start + (i + 1)* RADIAL_DIVISIONS + j;
				unsigned int i01 = b1Start + i * RADIAL_DIVISIONS + intMod(j + 1, RADIAL_DIVISIONS);
				unsigned int i11 = b1Start + (i + 1) * RADIAL_DIVISIONS + intMod(j + 1, RADIAL_DIVISIONS);

				if (j % 2 == 0) { //(j < 4) {
					faces.push_back(i00);
					faces.push_back(i10);
					faces.push_back(i11);
					faces.push_back(i00);
					faces.push_back(i11);
					faces.push_back(i01);
				}
				else {
					faces.push_back(i01);
					faces.push_back(i00);
					faces.push_back(i10);
					faces.push_back(i01);
					faces.push_back(i10);
					faces.push_back(i11);
				}
			}

			theta += thetaStep;
		}
		lateral += lateralStep;
	}

	//Connection between center and b1
	for (int j = 0; j < RADIAL_DIVISIONS; j++) {

		unsigned int i00 = b1BoundaryIndexMap[j];
		unsigned int i10 = b1Start + j;
		unsigned int i01 = b1BoundaryIndexMap[intMod(j + 1, RADIAL_DIVISIONS)];
		unsigned int i11 = b1Start + intMod(j + 1, RADIAL_DIVISIONS);

		if (j % 2 == 0) { //(j < 4) {
			faces.push_back(i00);
			faces.push_back(i10);
			faces.push_back(i11);
			faces.push_back(i00);
			faces.push_back(i11);
			faces.push_back(i01);
		}
		else {
			faces.push_back(i01);
			faces.push_back(i00);
			faces.push_back(i10);
			faces.push_back(i01);
			faces.push_back(i10);
			faces.push_back(i11);
		}
	}

	//B2 CYLINDER
	unsigned int b2Start = points.size();
	bx = normalize(cross(planeNormal, b2));
	by = normalize(cross(b2, bx));

	lateral = lateralStep * 2.f;
	for (unsigned int i = 0; i < divisions; i++) {
		float theta = 0.f;
		float thetaStep = 2.f*M_PI / float(RADIAL_DIVISIONS);
		for (unsigned int j = 0; j < RADIAL_DIVISIONS; j++) {
			if (i > divisions - 3) fixedPoints.push_back(points.size());

			points.push_back(junction + b2 * lateral + (by * cos(theta) + bx * sin(theta))*r2);

			if (i + 1 < divisions) {
				unsigned int i00 = b2Start + i * RADIAL_DIVISIONS + j;
				unsigned int i10 = b2Start + (i + 1)* RADIAL_DIVISIONS + j;
				unsigned int i01 = b2Start + i * RADIAL_DIVISIONS + intMod(j + 1, RADIAL_DIVISIONS);
				unsigned int i11 = b2Start + (i + 1) * RADIAL_DIVISIONS + intMod(j + 1, RADIAL_DIVISIONS);

				if (j % 2 == 0) { //(j < 4) {
					faces.push_back(i00);
					faces.push_back(i10);
					faces.push_back(i11);
					faces.push_back(i00);
					faces.push_back(i11);
					faces.push_back(i01);
					
				}
				else {
					faces.push_back(i01);
					faces.push_back(i00);
					faces.push_back(i10);
					faces.push_back(i01);
					faces.push_back(i10);
					faces.push_back(i11);
				}
			}

			theta += thetaStep;
		}
		lateral += lateralStep;
	}

	//Connection between center and b2
	for (int j = 0; j < RADIAL_DIVISIONS; j++) {

		unsigned int i00 = b2BoundaryIndexMap[j];
		unsigned int i10 = b2Start + j;
		unsigned int i01 = b2BoundaryIndexMap[intMod(j + 1, RADIAL_DIVISIONS)];
		unsigned int i11 = b2Start + intMod(j + 1, RADIAL_DIVISIONS);

		if (j % 2 == 0) { //(j < 4) {
			faces.push_back(i00);
			faces.push_back(i10);
			faces.push_back(i11);
			faces.push_back(i00);
			faces.push_back(i11);
			faces.push_back(i01);
		}
		else {
			faces.push_back(i01);
			faces.push_back(i00);
			faces.push_back(i10);
			faces.push_back(i01);
			faces.push_back(i10);
			faces.push_back(i11);
		}
	}

	BranchingStructureData ret;
	ret.points = points;
	ret.faces = faces;
	ret.fixedPoints = fixedPoints;

	return ret;
}

BranchingStructureData createBranchingStructure(const char* plyFile) {
	MeshInfoLoader minfo;
	minfo.loadModelPly(plyFile);

	BranchingStructureData ret;
	ret.points = minfo.vertices;
	ret.faces = minfo.indices;
	
	for (int i = 0; i < minfo.colors.size(); i++) {
		if (length(minfo.colors[i]) < 0.3f) {
			ret.fixedPoints.push_back(i);
		}
	}

	return ret;
}

float cotangent(vec3 a, vec3 b) {
	return acos(dot(normalize(a), normalize(b)));
}

vec3 cotangentLaplacian(const std::vector<vec3>& vertices, const vv::Adjacency& adjacency, unsigned int vertex) {
	glm::vec3 centroid(0);
	float weightSum = 0.f;

	vec3 v = vertices[vertex];

	for (unsigned int i = 0; i < adjacency.neighbours.size();  i++) {
		vec3 n = vertices[adjacency.neighbours[i]];
		vec3 l = vertices[adjacency.neighbours[intMod(i - 1, adjacency.neighbours.size())]];
		vec3 r = vertices[adjacency.neighbours[intMod(i + 1, adjacency.neighbours.size())]];
		
		float weight = cotangent(n - l, v - l) + cotangent(n - r, v - r);
		centroid += weight*n;
		weightSum += weight;
	}

	return centroid / weightSum - v;
}

vec3 normal(const std::vector<vec3>& vertices, const vv::Adjacency& adjacency, unsigned int vertex) {
	glm::vec3 centroid(0);
	float weightSum = 0.f;

	vec3 v = vertices[vertex];

	glm::vec3 normalSum(0);

	for (unsigned int i = 0; i < adjacency.neighbours.size(); i++) {
		vec3 n = vertices[adjacency.neighbours[i]];
		vec3 l = vertices[adjacency.neighbours[intMod(i - 1, adjacency.neighbours.size())]];
		vec3 r = vertices[adjacency.neighbours[intMod(i + 1, adjacency.neighbours.size())]];

		float weight = acos(dot(normalize(n-v), normalize(l-v)));

		normalSum += weight*normalize(cross(l - v, n - v));
		weightSum += weight;
	}

	return normalize(normalSum / weightSum);
}

template<typename T>
T angleLaplacian(const std::vector<vec3>& vertices, const vector<T>& f, const vv::Adjacency& adjacency, unsigned int vertex) {
	T centroid(0);
	float weightSum = 0.f;

	vec3 v = vertices[vertex];

	for (unsigned int i = 0; i < adjacency.neighbours.size(); i++) {
		vec3 n = vertices[adjacency.neighbours[i]];
		vec3 l = vertices[adjacency.neighbours[intMod(i - 1, adjacency.neighbours.size())]];
		vec3 r = vertices[adjacency.neighbours[intMod(i + 1, adjacency.neighbours.size())]];

		T n_val = f[adjacency.neighbours[i]];

		float weight = acos(dot(normalize(n - v), normalize(l - v))); //cotangent(n - l, v - l) + cotangent(n - r, v - r);
		centroid += weight * n_val;
		weightSum += weight;
	}

	return centroid / weightSum - f[vertex];
}

template<typename T>
T cotangentLaplacian(const std::vector<vec3>& vertices, const vector<T>& f, const vv::Adjacency& adjacency, unsigned int vertex) {
	T centroid(0);
	float weightSum = 0.f;

	vec3 v = vertices[vertex];

	for (unsigned int i = 0; i < adjacency.neighbours.size(); i++) {
		vec3 n = vertices[adjacency.neighbours[i]];
		vec3 l = vertices[adjacency.neighbours[intMod(i - 1, adjacency.neighbours.size())]];
		vec3 r = vertices[adjacency.neighbours[intMod(i + 1, adjacency.neighbours.size())]];

		T n_val = f[adjacency.neighbours[i]];

		float weight = cotangent(n - l, v - l) + cotangent(n - r, v - r);
		centroid += weight * n_val;
		weightSum += weight;
	}

	return centroid / weightSum - f[vertex];
}

vec3 umbrellaLaplacian(const std::vector<vec3>& vertices, const vv::Adjacency& adjacency, unsigned int vertex) {
	glm::vec3 centroid (0);

	for (unsigned int adj_v : adjacency.neighbours) {
		centroid += vertices[adj_v];
	}

	return centroid/float(adjacency.neighbours.size()) - vertices[vertex];
}

void cotangentLaplacian3rdLaw(const std::vector<vec3>& vertices, const vv::Adjacency& adjacency, unsigned int vertex, std::vector<vec3>* outputVertices) {
	float step = 0.001f;

	glm::vec3 centroid(0);
	float weightSum = 0.f;
	std::vector<float> weight;
	weight.resize(adjacency.neighbours.size());
	vec3 v = vertices[vertex];

	for (unsigned int i = 0; i < adjacency.neighbours.size(); i++) {
		vec3 n = vertices[adjacency.neighbours[i]];
		vec3 l = vertices[adjacency.neighbours[intMod(i - 1, adjacency.neighbours.size())]];
		vec3 r = vertices[adjacency.neighbours[intMod(i + 1, adjacency.neighbours.size())]];

		weight[i] = cotangent(n - l, v - l) + cotangent(n - r, v - r);
		centroid += weight[i] * n;
		weightSum += weight[i];
	}

	vec3 dir = centroid / weightSum - v;

	for (unsigned int i = 0; i < adjacency.neighbours.size(); i++)
		(*outputVertices)[adjacency.neighbours[i]] -= dir * 0.5f*step*weight[i] / weightSum;

	(*outputVertices)[vertex] += dir * 0.5f*step;
}


void laplacianOnMesh(const std::vector<vec3>& vertices, const std::vector <vv::Adjacency> & adjacency, std::vector<vec3>* output) {
	float weight = 0.001f;

	for (int i = 0; i < vertices.size(); i++) {

		vec3 diff = cotangentLaplacian(vertices, adjacency[i], i);
		//vec3 diff = normal(vertices, adjacency[i], i);
		(*output)[i] = vertices[i] + diff * weight*0.5f;
		
		//cotangentLaplacian3rdLaw(vertices, adjacency[i], i, output);
	}
}

void laplacianOnMesh2ndOrder(const std::vector<vec3>& vertices, const std::vector <vv::Adjacency> & adjacency, std::vector<vec3>* output) {
	float weight = 0.01f;
	std::vector<vec3> laplacians;

	for (int i = 0; i < vertices.size(); i++) {

		laplacians.push_back(-cotangentLaplacian(vertices, vertices, adjacency[i], i));
	}

	for (int i = 0; i < vertices.size(); i++) {

		vec3 diff = cotangentLaplacian(vertices, laplacians, adjacency[i], i);
		(*output)[i] = vertices[i] + diff * weight*0.5f;

		//cotangentLaplacian3rdLaw(vertices, adjacency[i], i, output);
	}
}

void pushAwayFromBranches(const vector<vec3>& branches, const vector<float>& radii, vec3 junction, vector<vec3>* points) {
	for (auto& p : *points) {
		int index = -1;
		float closestDistance = 1000000.f;
		vec3 vectorFrom;
		for (int i = 0; i < branches.size(); i++) {
			vec3 v = p - junction;
			float t = dot(v, branches[i]) / dot(branches[i], branches[i]);
			vec3 v_p = t*branches[i];
			float dist = length(v - v_p);
			if (dist < closestDistance && t > 0 && t < 1) {
				index = i;
				closestDistance = dist;
				vectorFrom = v - v_p;
			}
		}

		
		p += normalize(vectorFrom)*std::max(radii[index] - closestDistance, 0.f)*0.01f;
	}
}

vec3 vectorDf(vec3 vec, char comp) {
	vec.x *= (comp == 'x') ? 1.f : 0.f;
	vec.y *= (comp == 'y') ? 1.f : 0.f;
	vec.z *= (comp == 'z') ? 1.f : 0.f;

	return vec;
}

std::vector<vec3> extrudeSurface(const std::vector<vec3>& vertices, const std::vector <vv::Adjacency> & adjacency, float amount) {
	std::vector<vec3> outputVertices = vertices;
	
	for (int i = 0; i < vertices.size(); i++) {
		outputVertices[i] += normal(vertices, adjacency[i], i)*amount;
		if (outputVertices[i] != outputVertices[i])
			printf("Problem at %d\n", i);
	}

	return outputVertices;
}

float volumeOfTriangularPrism(vec3 a0, vec3 b0, vec3 c0, vec3 a1, vec3 b1, vec3 c1) {
	//return dot(cross(b0 - a0, c0 - a0), a1 - a0)*0.5f +
		//dot(cross(b1 - a1, c1 - a1), a1 - a0)*0.5f;
	return dot(cross(b1 - a1, c1 - a1), a1 - a0)*1.f/6.f
		+ dot(b1 - b0, cross(c0 - b0, a0 - b0))*1.f/6.f
		+ dot(b1 - c0, cross(c1 - c0, a0 - c0))*1.f/6.f;
}

vec3 volumeGradient(vec3 a1, vec3 b1, vec3 c1, vec3 a0) {
	vec3 u = b1 - a1;
	vec3 v = c1 - a1;
	vec3 w = a1 - a0;

	vec3 gradient = vec3(
		dot((u + v), cross(vec3(1, 0, 0), w)) + dot(u, cross(v, vec3(1, 0, 0))),
		dot((u + v), cross(vec3(0, 1, 0), w)) + dot(u, cross(v, vec3(0, 1, 0))),
		dot((u + v), cross(vec3(0, 0, 1), w)) + dot(u, cross(v, vec3(0, 0, 1)))
	)*1.f/6.f;

	return gradient;
}

vec3 volumeGradientNormalized(vec3 a0, vec3 b1, vec3 c1) {
	return normalize(cross(b1 - a0, c1 - a0));
}

float& accessVolume(unsigned int a, unsigned int b, unsigned int c);

//Maybe not exactly gradient? Gives good guess for which direction to step in and by how much
vec3 volumeGradient(const std::vector<vec3>& lowerSurface, const std::vector<vec3>& upperSurface, const vv::Adjacency& adjacency, unsigned int index, float growthHeight) {
	vec3 a0 = lowerSurface[index];
	vec3 a1 = upperSurface[index];
	
	vec3 totalGradient = vec3(0);
	float totalVolumeDiff = 0.f;

	for (int i = 0; i < adjacency.neighbours.size(); i++) {
		unsigned int b0_i = adjacency.neighbours[i];
		unsigned int c0_i = adjacency.neighbours[intMod(i + 1, adjacency.neighbours.size())];

		vec3 b0 = lowerSurface[adjacency.neighbours[i]];
		vec3 c0 = lowerSurface[adjacency.neighbours[intMod(i + 1, adjacency.neighbours.size())]];
		vec3 b1 = upperSurface[adjacency.neighbours[i]];
		vec3 c1 = upperSurface[adjacency.neighbours[intMod(i + 1, adjacency.neighbours.size())]];

		float currentVolume = volumeOfTriangularPrism(a0, b0, c0, a1, b1, c1);
		float targetVolume = accessVolume(index, b0_i, c0_i)*growthHeight;	//length(cross(b0 - a0, c0 - a0))*0.5f*growthHeight;
		vec3 gradient = volumeGradientNormalized(a0, b1, c1);

		//Newton Rhapson?
		totalGradient += gradient*(targetVolume - currentVolume)/length(gradient);
		totalVolumeDiff += targetVolume - currentVolume;
	}

	return totalGradient * abs(totalVolumeDiff);
}

std::map<unsigned int, std::map<unsigned int, std::map<unsigned int, float>>> volumeMap;

float& accessVolume(unsigned int a, unsigned int b, unsigned int c) {
	vector<unsigned int> keys = { a, b, c };
	std::sort(keys.begin(), keys.end());
	return volumeMap[keys[0]][keys[1]][keys[2]];
}

void calculateVolumes(const std::vector<vec3>& vertices, const std::vector <vv::Adjacency> & adjacency) {
	for (int v_i = 0; v_i < vertices.size(); v_i++) {
		vec3 v = vertices[v_i];
		for (int i = 0; i < adjacency[v_i].neighbours.size(); i++) {
			unsigned int n_i = adjacency[v_i].neighbours[i];
			vec3 n = vertices[n_i];
			unsigned int l_i = adjacency[v_i].neighbours[intMod(i - 1, adjacency[v_i].neighbours.size())];
			vec3 l = vertices[l_i];
			accessVolume(v_i, n_i, l_i) = length(cross(l - v, n - v))*0.5f;
		}
	}
}

void surfaceGrowth(const std::vector<vec3>& vertices, const std::vector <vv::Adjacency> & adjacency, std::vector<vec3>* output, float growth) {
	float weight = 1.f;

	std::vector<vec3> oldSurface = vertices;
	std::vector<vec3> newSurface = extrudeSurface(vertices, adjacency, growth);

	for (int it = 0; it < 0; it++) {
		std::vector<vec3> modifiedSurface = newSurface;
		for (int i = 0; i < newSurface.size(); i++) {
			modifiedSurface[i] += volumeGradient(oldSurface, newSurface, adjacency[i], i, growth)*weight;
		}
		newSurface = modifiedSurface;
	}

	(*output) = newSurface;
}

void volumeGradientMesh(const std::vector<vec3>& vertices, const std::vector<vec3>& oldVertices,  const std::vector <vv::Adjacency> & adjacency, std::vector<vec3>* output, float growth) {
	vector<vec3> oldSurface = oldVertices;
	vector<vec3> newSurface = vertices;
	
	for (int it = 0; it < 2000; it++) {
		std::vector<vec3> modifiedSurface = newSurface;
		for (int i = 0; i < newSurface.size(); i++) {
			modifiedSurface[i] += volumeGradient(oldSurface, newSurface, adjacency[i], i, growth)*500.f;
		}
		newSurface = modifiedSurface;
	}

	(*output) = newSurface;
}

void WindowManager::laplacianSmoothingMeshLoop() {
	glfwSetKeyCallback(window, keyCallback);
	glfwSetWindowSizeCallback(window, windowResizeCallback);

	SimpleTexManager tm;
	SimpleShader shader;
	BlinnPhongShaderT bpShader;

	float r0 = 0.2f;
	float r1 = 0.15f;
	float r2 = sqrt(r0*r0 + r1 * r1);

	///*
	vec3 b0 = vec3(-0, 1.f, 0);
	vec3 b1 = vec3(0.7f, 0.3f, 0);
	vec3 b2 = vec3(-0.2, -1.f, 0);

	/*
	BranchingStructureData data = generateBranchingStructure(
		b2, r2,
		b0, r0,
		b1, r1,
		vec3(0), 8);
	//*/
	/*
	BranchingStructureData data = generateBranchingStructure(
		b2, 0.01,
		b0, 0.01,
		b1, 0.01,
		vec3(0), 10);
		//*/
	printf("Volume of half cube = %f\n", 
	volumeOfTriangularPrism(
		vec3(0, 0, 0), vec3(0, 0, 1), vec3(1, 0, 0),
		vec3(0, 1, 0), vec3(0, 1, 1), vec3(1, 1, 0))
	);
	BranchingStructureData data = createBranchingStructure("models/LowResThinPipe.ply");

	std::vector<vv::Adjacency> adjacencyList = vv::faceListToVertexVertex(data.faces, data.points);

	auto geometry = make<IndexGeometryUint<attrib::Position>>(GL_TRIANGLES);

	geometry->loadIndices(data.faces.data(), data.faces.size());
	geometry->loadBuffers(data.points.data(), data.points.size());

	Drawable mesh(geometry, std::make_shared<ColorMat>(vec3(1, 0, 0)));
	vector<vec3> vertices[3] = { data.points, data.points, data.points };

	vector<vec3> lastSurface;

	calculateVolumes(vertices[0], adjacencyList);

	//Textured plane
	FT_Library freetype;
	FT_Init_FreeType(&freetype);

	Font font = createFont(&freetype, "fonts/OpenSans-Regular.ttf", &tm, 64);

	//auto fontGeom = createPlaneGeometry(Orientation::PositiveZ);
	//Character c = font.character('R');
	//std::vector<vec3> verts = { vec3(c.pointCoords[0], 0), vec3(c.pointCoords[1], 0), vec3(c.pointCoords[2], 0), vec3(c.pointCoords[3], 0) };
	std::vector<glm::vec3> verts;
	std::vector<glm::vec3> offsets;
	std::vector<glm::vec2> uvs;
	std::vector<unsigned int> faces;

	TextShader textShader;
	getTextBuffers("Random words", font, &verts, &offsets, &uvs, &faces);
	auto fontGeom = make<IndexGeometryT<unsigned int, attrib::Position, attrib::TexCoord>>(GL_TRIANGLES);
	fontGeom->loadBuffers(verts.data(), uvs.data(), verts.size());
	fontGeom->loadIndices(faces.data(), faces.size());

	Drawable fontDrawable(fontGeom, make<TextureMat>(font.tex));
	fontDrawable.addMaterial(make<ColorMat>(vec4(0.1f, 0.2f, 0.4f, 1.f)));
	SimpleTexShader texShader;	

	Drawable sphereDrawable(createSphereGeometry(), make<ColorMat>(vec3(1.0, 1.0, 1.0)));
	sphereDrawable.addMaterial(make<ShadedMat>(0.0f, 0.8f, 0.5f, 10.f));
	Drawable planeDrawable(createPlaneGeometry(), make<ColorMat>(vec3(0.5, 0.5, 0.2)));
	planeDrawable.addMaterial(make<ShadedMat>(0.3f, 0.2f, 0.8f, 10.f));
	planeDrawable.position = vec3(0, -1, 0);
	glm::vec3 lightPosition = vec3(1.f, 1.f, 0.f)*2.f;
	Drawable lightDrawable(createSphereGeometry(20, 20, 1.f), make<ColorMat>(vec3(1.0, 1.0, 0.)));
	lightDrawable.addMaterial(make<ShadedMat>(1.f, 0.f, 0.f, 10.f));
	lightDrawable.position = lightPosition;
	lightDrawable.setScale(vec3(0.2, 0.2, 0.2));

	auto testGeom = make<GeometryT<attrib::Position, attrib::TexCoord>>(GL_TRIANGLES);
	testGeom->loadBuffers(verts.data(), uvs.data(), verts.size());
	GeometryT<attrib::Position> testGeom2;

	std::vector<vec3> bpPositions = { vec3(0, 0, 0.1), vec3(1, 0, 0.1), vec3(0, 1, 0.1) };
	std::vector<vec3> bpNormals = { vec3(0, 0, 1), vec3(0, 0, 1), vec3(0, 0, 1) };
	auto bpGeom = make<GeometryT<attrib::Normal, attrib::Position>>(GL_TRIANGLES);
	bpGeom->loadBuffers(bpNormals.data(), bpPositions.data(), bpPositions.size());

	Drawable bpDrawable(bpGeom, make<ColorMat>(vec4(1.f, 0.f, 0.f, 1.f)));
	bpDrawable.addMaterial(make<ShadedMat>(0.3f, 0.4f, 0.4f, 10.f));

	//loadBuffers_rec2<GeometryT<attrib::Position>, attrib::Position>(&testGeom2, vertices->data());
	//loadBuffers_rec2<GeometryT<attrib::Position, attrib::TexCoord>, attrib::Position, attrib::TexCoord>
	//	(static_cast<GeometryT<attrib::Position, attrib::TexCoord>*>(testGeom.get()), verts.data(), uvs.data());

	//testGeom->loadBuffer<attrib::Position>(verts.data());

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(3.f);

	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		static bool next_iteration_pressed = false;
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !next_iteration_pressed) {
			next_iteration_pressed = true;
			static int count = 0;
			for (int i = 0; i < 1; i++) {
				unsigned int input = count % 2;
				unsigned int output = (count + 1) % 2;
				float growth = 0.01f;

				if (count % 2 == 0) {
					lastSurface = vertices[input];
					vertices[output] = extrudeSurface(vertices[input], adjacencyList, growth);
				}
				else {
					volumeGradientMesh(vertices[input], lastSurface, adjacencyList, &vertices[output], growth);
				}
				//laplacianOnMesh2ndOrder(vertices[input], adjacencyList, &vertices[output]);
				//pushAwayFromBranches({ b0, b1, b2 }, { r0, r1, r2 }, vec3(0), &vertices[output]);

				//surfaceGrowth(vertices[input], adjacencyList, &vertices[output], 0.01f);

				for (unsigned int index : data.fixedPoints) {
					//vertices[output][index] = vertices[input][index];
				}

				count++;
			}

			unsigned int output = count % 2;
			geometry->loadBuffers(vertices[output].data(), vertices[output].size());
		}
		else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
			next_iteration_pressed = false;
		}
		
		mesh.getMaterial<ColorMat>()->color = vec4(0, 0, 1, 1);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		//shader.draw(cam, mesh);

		textShader.draw(cam, fontDrawable);
		//bpShader.draw(cam, vec3(10, 10, 10), bpDrawable);

		//bpShader.draw(cam, lightPosition, sphereDrawable);
		//bpShader.draw(cam, lightPosition, planeDrawable);
		bpShader.draw(cam, lightPosition, lightDrawable);
		
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		mesh.getMaterial<ColorMat>()->color = vec4(1, 0, 0, 1);
		//shader.draw(cam, mesh);

		glfwSwapBuffers(window);
		glfwWaitEvents();
	}

	glfwTerminate();
}

vec3 perpendicular2D(vec3 v) {
	return vec3(-v.y, v.x, v.z);
}

void WindowManager::growthLoop2D() {

	glfwSetKeyCallback(window, keyCallback);
	glfwSetWindowSizeCallback(window, windowResizeCallback);

	SimpleTexManager tm;
	SimpleShader shader;

	auto debugGeometry = make<PositionGeometry>(GL_LINES);
	vector<vec3> debugPoints;
	Drawable debugDrawable(
		debugGeometry,
		make<ColorMat>(vec3(0, 1, 0))
	);

	vector<vec3> points;

	vec3 xDirection = normalize(vec3(1, -1, 0));
	const int X_DIVISIONS = 20;
	const int Y_DIVISIONS = 20;

	const float width = 1.f;
	const float height = 1.f;
	vec3 position = vec3(width*0.5f, -height * 0.5f, 0.f);
	for (int x = 0; x < X_DIVISIONS; x++) {
		position -= xDirection*width / float(X_DIVISIONS);
		points.push_back(position);
	}
	for (int y = 0; y < Y_DIVISIONS; y++) {
		position.y += height / float(Y_DIVISIONS);
		points.push_back(position);
	}

	unsigned int lastLayerStart = 0;
	unsigned int layerSize = X_DIVISIONS + Y_DIVISIONS;

	vector<unsigned int> indices;

	float growth = 0.1;

	float growthArea = growth * width / float(X_DIVISIONS);
	float targetLength = width / float(X_DIVISIONS);

	unsigned int layerStart = points.size();
	for (int i = 0; i < layerSize; i++) {
		vec3 normal(0.f);
		if (i + 1 < layerSize)
			normal += perpendicular2D(points[lastLayerStart + i] - points[lastLayerStart + i + 1]);
		if (i - 1 >= 0)
			normal += perpendicular2D(points[lastLayerStart + i - 1] - points[lastLayerStart + i]);
		points.push_back(points[lastLayerStart + i] + normalize(normal)*growth);
	}

	//Triangulate
	for (int i = 0; i < layerSize - 1; i++) {
		indices.push_back(lastLayerStart+i);
		indices.push_back(layerStart + i);
		indices.push_back(layerStart + i + 1);
		
		indices.push_back(lastLayerStart + i);
		indices.push_back(layerStart + i + 1);
		indices.push_back(lastLayerStart + i + 1);
	}

	

	auto geometry = make<IndexGeometryUint <attrib::Position>>(GL_TRIANGLES, indices.data(), indices.size(), points.data(), points.size());

	Drawable tree(geometry, std::make_shared<ColorMat>(vec3(1, 0, 0)));

	//cam.projection = mat4(1.f);

	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		static bool next_iteration_pressed = false;
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !next_iteration_pressed) {
			next_iteration_pressed = true;
			
			static int count = 0;

			if(count %10 == 9){
				lastLayerStart = layerStart;
				layerStart = points.size();
				for (int i = 0; i < layerSize; i++) {
					vec3 normal(0.f);
					if (i + 1 < layerSize)
						normal += perpendicular2D(points[lastLayerStart + i] - points[lastLayerStart + i + 1]);
					if (i - 1 >= 0)
						normal += perpendicular2D(points[lastLayerStart + i - 1] - points[lastLayerStart + i]);
					points.push_back(points[lastLayerStart+i] + normalize(normal)*growth*0.5f);
				}

				//Triangulate
				for (int i = 0; i < layerSize - 1; i++) {
					indices.push_back(lastLayerStart + i);
					indices.push_back(layerStart + i);
					indices.push_back(layerStart + i + 1);

					indices.push_back(lastLayerStart + i);
					indices.push_back(layerStart + i + 1);
					indices.push_back(lastLayerStart + i + 1);
				}
			}

			//Follow volume gradient
			else {
				//printf("--------------------\n");
				for(int i=0; i<1; i++){
					vector<vec3> offsets;
					offsets.resize(layerSize, vec3(0.f));

					const float stepSize = 1.f;
					for (int i = 0; i < layerSize; i++) {

						/*
						const float targetDiagonal = sqrt(targetLength*targetLength + growth * growth);
						vec3 c1 = points[layerStart + i];
						vec3 c0 = points[lastLayerStart + i];
						vec3 gradient = normalize(c0 - c1)*(growth - length(c0 - c1))/growth;

						if (i - 1 >= 0) {
							vec3 c1 = points[layerStart + i];
							vec3 r1 = points[layerStart + i - 1];
							vec3 r0 = points[lastLayerStart + i - 1];

							gradient += normalize(r1 - c1)*(targetLength - length(r1 - c1))/targetLength*0.25f;
							printf("Target = %f\n", targetLength - length(r1-c1));
							gradient += normalize(r0 - c1)*(targetDiagonal - length(r0 - c1))/targetDiagonal;
						}
						if (i + 1 < layerSize) {
							vec3 l1 = points[layerStart + i + 1];
							vec3 l0 = points[lastLayerStart + i + 1];

							gradient += normalize(l1 - c1)*(targetLength - length(l1 - c1))/targetLength*0.25f;
							printf("Target = %f\n", targetLength - length(l1 - c1));
							gradient += normalize(l0 - c1)*(targetLength - length(l0 - c1))/targetDiagonal;
						}

						offsets[i] = -gradient * 1.f*0.1f;
						*/
						///*
						vec3 gradient(0.f);
						vec3 lengthGradient(0.f);
						float volumeDiff = 0.f;
						float lengthDiff = 0.f;
						
						if (i - 1 >= 0) {
							vec3 c1 = points[layerStart + i];
							vec3 r1 = points[layerStart + i - 1];
							vec3 c0 = points[lastLayerStart + i];
							vec3 r0 = points[lastLayerStart + i - 1];

							float currentVolume = length(cross(c0 - c1, r1 - c1))*0.5f + length(cross(r0 - c0, r1 - c0))*0.5f;
							float targetVolume = growthArea;

							debugPoints.push_back(c1);
							debugPoints.push_back(c1 + normalize(perpendicular2D(r1 - c0))*(targetVolume - currentVolume)*20.f);

							vec3 temp = perpendicular2D(r1 - c0);

							//if (i == 7)
								//printf("RIGHT: Current volume = %f - Target volume = %f\n\tp = (%f, %f)\n", currentVolume, targetVolume, temp.x, temp.y);

							gradient += normalize(perpendicular2D(r1 - c0))*(targetVolume - currentVolume);
							volumeDiff += targetVolume - currentVolume;
							lengthGradient += normalize(c1 - r1)*(targetLength - length(r1 - c1));
							lengthDiff += targetLength - length(r1 - c1);
						}
						if (i + 1 < layerSize) {
							vec3 c1 = points[layerStart + i];
							vec3 l1 = points[layerStart + i + 1];
							vec3 c0 = points[lastLayerStart + i];
							vec3 l0 = points[lastLayerStart + i + 1];

							float currentVolume = length(cross(l1 - c1, c0 - c1))*0.5f + length(cross(l1 - c0, l0 - c0))*0.5f;
							float targetVolume = growthArea;	//length(c0 - l0)*growth;

							//printf("Volume = %f\n", currentVolume);

							debugPoints.push_back(c1);
							debugPoints.push_back(c1 + normalize(perpendicular2D(c0 - l1))*(targetVolume - currentVolume)*20.f);

							vec3 temp = perpendicular2D(c0 - l1);

							//if (i == 7)
								//printf("LEFT: Current volume = %f - Target volume = %f\n\tp = (%f, %f)\n", currentVolume, targetVolume, temp.x, temp.y);

							gradient += normalize(perpendicular2D(c0 - l1))*(targetVolume - currentVolume);
							volumeDiff += targetVolume - currentVolume;
							lengthGradient += normalize(c1 - l1)*(targetLength - length(l1 - c1));
							lengthDiff += targetLength - length(l1 - c1);
						}

						//printf("volumeDiff = %f\n", volumeDiff);
						debugPoints.push_back(points[layerStart + i]);
						debugPoints.push_back(points[layerStart + i] + normalize(gradient)*volumeDiff*20.f);
						lengthGradient = (length(lengthGradient) < 0.0001f) ? vec3(0) : normalize(lengthGradient);
						//points[layerStart + i] += normalize(gradient)*abs(volumeDiff) * stepSize;
						offsets[i] = normalize(gradient)*abs(volumeDiff) * stepSize;
						//*/
					}

					for (int i = 0; i < layerSize; i++)
						points[layerStart + i] += offsets[i]*0.1f;
				}
			}

			count++;

			geometry->loadBuffers(points.data(), points.size());
			geometry->loadIndices(indices.data(), indices.size());
			debugGeometry->loadBuffers(debugPoints.data(), debugPoints.size());

			
		}
		else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
			next_iteration_pressed = false;
		}

		glLineWidth(2.f);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		tree.getMaterial<ColorMat>()->color = vec4(0, 0, 1, 1);
		shader.draw(cam, tree);

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		tree.getMaterial<ColorMat>()->color = vec4(1, 0, 0, 1);
		shader.draw(cam, tree);

		shader.draw(cam, debugDrawable);

		shader.draw(cam, debugDrawable);
		debugPoints.clear();

		glfwSwapBuffers(window);
		glfwWaitEvents();
	}

	glfwTerminate();
}

void WindowManager::laplacianSmoothing() {

	glfwSetKeyCallback(window, keyCallback);
	glfwSetWindowSizeCallback(window, windowResizeCallback);

	SimpleTexManager tm;
	SimpleShader shader;

	///*
	const int SIZE = 9;
	vec3 points[SIZE] = {
		vec3(-1.333, 0, 0),
		vec3(-1, 0, 0), vec3(-0.666, 0, 0), vec3(-0.333, 0, 0), vec3(0, 0, 0), vec3(0, 0.333, 0), vec3(0, 0.666, 0), vec3(0, 1, 0),
		vec3(0, 1.333, 0)
	};
	//*/
	/*
	const int SIZE = 8;
//	vec3 points[SIZE] = { vec3(-1, -1, 0), vec3(0, -1, 0), vec3(1, -1, 0), vec3(1, 0, 0), vec3(1, 1, 0), vec3(0, 1, 0), vec3(-1, 1, 0), vec3(-1, 0, 0)};
	vec3 points[SIZE] = { vec3(-0.1, -1, 0), vec3(0, -1, 0), vec3(1, -1, 0), vec3(0.4, 0, 0), vec3(1, 1, 0), vec3(0, 1, 0), vec3(-1, 1, 0), vec3(-1, 0, 0) };
	
	//*/
	vec3 newPoints[SIZE];
	copy(begin(points), end(points), begin(newPoints));

	auto geometry = make<GeometryT<attrib::Position>>(GL_LINE_STRIP, points, SIZE);
	
	Drawable curve(geometry, std::make_shared<ColorMat>(vec3(1, 0, 0)));


	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		static bool next_iteration_pressed = false;
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !next_iteration_pressed) {
			next_iteration_pressed = true;
			for (int j = 0; j < 20; j++) {
				float weight = 0.5f;
				float inflate = 0.11f;

				copy(begin(points), end(points), begin(newPoints));
				float weights[SIZE];
				float averageWeight = 0.f;
				/*
				for (int i = 0; i < SIZE; i++) {
					newPoints[i] += weight * (0.5f* (points[intMod(i - 1, SIZE)] + points[intMod(i + 1, SIZE)]) - points[i]);
					weights[i] = length(0.5f* (points[intMod(i - 1, SIZE)] + points[intMod(i + 1, SIZE)]) - points[i]);
					float a = length(0.5f* (points[intMod(i - 1, SIZE)] + points[intMod(i + 1, SIZE)]) - points[i]);
					float b = 0.5f*length(points[intMod(i - 1, SIZE)] - points[intMod(i + 1, SIZE)]);
					float R = (a*a + b * b) / (2 * a);
					float r = R - a*weight;
					if (r > 0 && R > 0) {
						averageWeight += R / r;	//weights[i];
						weights[i] = R;
					}
					else
						weights[i] = 1.f;
				}

				averageWeight /= float(SIZE);

				copy(begin(newPoints), end(newPoints), begin(points));

				//Inflate
				for (int i = 0; i < SIZE; i++) {
					float a = length(0.5f* (points[intMod(i - 1, SIZE)] + points[intMod(i + 1, SIZE)]) - points[i]);
					float b = 0.5f*length(points[intMod(i - 1, SIZE)] - points[intMod(i + 1, SIZE)]);
					float R = (a*a + b * b) / (2 * a);
					float r = R - a * weight;
					if (averageWeight > 0)
						//newPoints[i] -= weight*weights[i]/R*(0.5f* (points[intMod(i - 1, SIZE)] + points[intMod(i + 1, SIZE)]) - points[i]);
						newPoints[i] -= inflate * (0.5f* (points[intMod(i - 1, SIZE)] + points[intMod(i + 1, SIZE)]) - points[i]);
				}

				//*/
				//CLOSED CURVE
				
				for (int i = 1; i < SIZE-1; i++) {
					//NORMAL METHOD
					//newPoints[i] = (1.f - weight)*points[i] + weight *0.5f* (points[i - 1] + points[i + 1]);
					vec3 l = 0.5f*(points[i - 1] + points[i + 1]) - points[i];
					newPoints[i] += 0.5f*weight*l;
					newPoints[i - 1] -= 0.25f*weight*l;
					newPoints[i + 1] -= 0.25f*weight*l;
				}

				//newPoints[1].y -= points[1].y;
				//newPoints[SIZE-2].x -= points[SIZE-2].x;
				newPoints[0] = points[0];
				newPoints[1] = points[1];
				newPoints[SIZE - 2] = points[SIZE - 2];
				newPoints[SIZE-1] = points[SIZE-1];
				//*/

				geometry->loadBuffers(newPoints, SIZE);

				copy(begin(newPoints), end(newPoints), begin(points));
			}
		}
		else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
			next_iteration_pressed = false;
		}

		shader.draw(cam, curve);

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
		make<GeometryT<attrib::Position, attrib::TexCoord>>(GL_TRIANGLES, points, coords, 6),
		make<TextureMat>(createTexture2D(1, 1, &tm)));

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

vec3 rationalBezierSpline(float u, vec3 p0, float w0, vec3 p1, float w1, vec3 p2, float w2) {
	u = glm::clamp(u, 0.f, 1.f);

	return ((1 - u)*(1 - u)*w0*p0 + 2.f*(1 - u)*u*w1*p1 + u * u*w2*p2) /
		((1 - u)*(1 - u)*w0 + 2.f*(1 - u)*u*w1 + u * u*w2);
}

float arcLengthApprox(vec4 p0, vec4 p1, vec4 p2, float u) {
	float w = p1.w;
	float l1 = length(vec3(p0) - vec3(p1)/w);
	float l2 = length(vec3(p2) - vec3(p1)/w);
	l1 = l1 / (l1 + l2);
	l2 = 1.f - l1;
	//printf("l1 = %f\n", l1);
	//Use approximation of 1D quadratic nurbs curve with control points 0, 1 and 2
	return (2.f*l1*u*(1 - u)*w + u * u) / ((1 - u)*(1 - u) + 2.f*(1 - u)*u*w + u * u); //(1 + (w - 1) * 2 * u*(1 - u));
}

vec3 getCursorPos(GLFWwindow* window, unsigned int width, unsigned int height, glm::mat4 projectionMatrix) {
	//glfwGetCursorPos
	double x, y;
	float w = float(width);
	float h = float(height);
	glfwGetCursorPos(window, &x, &y);
	vec4 mousePos(2.f*x / w - 1.f, -(2.f*y / h - 1.f), 0, 1.f);
	vec4 unprojected = glm::inverse(projectionMatrix)*mousePos;
	return vec3(unprojected);
}

void WindowManager::rationalBezierLoop() {
	glfwSetKeyCallback(window, keyCallback);
	glfwSetWindowSizeCallback(window, windowResizeCallback);

	vec3 controlPoints[3] = {
		vec3(-0.5, 0.5, 0),
		vec3(-0.5, -0.5, 0),
		vec3(-0.5, -0.75, 0)
	};

	float radius = 0.02f;

	SimpleTexManager tm;

	Texture checkerTexture = createTexture2D("./textures/Checkerboard.jpg", &tm);

	glActiveTexture(NO_ACTIVE_TEXTURE);
	glBindTexture(GL_TEXTURE_2D, checkerTexture.getID());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	BlinnPhongShader bpShader;
	auto controlGeometry = make<PositionGeometry>(GL_LINE_STRIP);
	Drawable controlDrawable(controlGeometry, make<ColorMat>(vec3(0, 0, 0)));
	auto curveGeometry = make<TextureGeometry>(GL_LINE_STRIP);
	Drawable curveDrawable(curveGeometry, make<TextureMat>(checkerTexture));
	auto normalSpacingCurveGeometry = make<TextureGeometry>(GL_LINE_STRIP);
	Drawable normalSpacingDrawable(normalSpacingCurveGeometry, make<TextureMat>(checkerTexture));
	auto controlPointGeometry = createSphereGeometry(40, 20);
	Drawable controlPointDrawable(controlPointGeometry, make<ColorMat>(vec3(0, 0, 0)));
	controlPointDrawable.setScale(glm::vec3(radius));
	
	vec3 offset(1, 0, 0);

	curveDrawable.position += offset;

	FlatColorShader shader;
	SimpleTexShader texShader;

	glLineWidth(3.f);
	glPointSize(6.f);

	bool updated = true;

	Camera cam;
	cam.projection = createAspectRatioMatrix(window_width, window_height);

	glClearColor(1.f, 1.f, 1.f, 1.f);
	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (windowResized) {
			window_width = windowWidth;
			window_height = windowHeight;
			glViewport(0, 0, window_width, window_height);
			cam.projection = createAspectRatioMatrix(window_width, window_height);
		}
	
		vec3 cursorPos = getCursorPos(window, window_width, window_height, cam.projection);
		
		static bool upPressed = false;
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && !upPressed) {
			vec3 v = normalize(controlPoints[1] - controlPoints[0]);
			vec3 v_perp = vec3(-v.y, v.x, 0.f);
			vec3 rotated = cos(glm::pi<float>()/4.f)*v + sin(glm::pi<float>() / 4.f)*v_perp;
			controlPoints[0] = length(controlPoints[1] - controlPoints[0])*rotated + controlPoints[1];
			upPressed = true;
			updated = true;
		}
		else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_RELEASE)
			upPressed = false;

		static bool downPressed = false;
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && !downPressed) {
			vec3 v = normalize(controlPoints[1] - controlPoints[0]);
			vec3 v_perp = vec3(-v.y, v.x, 0.f);
			vec3 rotated = cos(-glm::pi<float>() / 4.f)*v + sin(-glm::pi<float>() / 4.f)*v_perp;
			controlPoints[0] = length(controlPoints[1] - controlPoints[0])*rotated + controlPoints[1];
			downPressed = true;
			updated = true;
		}
		else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_RELEASE)
			downPressed = false;


		static int grabIndex = -1;
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
			if (distance(cursorPos, controlPoints[0]) < 2.f*radius)
				grabIndex = 0;
			else if (distance(cursorPos, controlPoints[2]) < 2.f*radius)
				grabIndex = 2;
			if (grabIndex >= 0) {
				float l = length(controlPoints[grabIndex] - controlPoints[1]);
				controlPoints[grabIndex] = normalize(cursorPos - controlPoints[1])*l + controlPoints[1];
				updated = true;
			}
		}
		else
			grabIndex = -1;

		if (updated) {
			controlGeometry->loadBuffers(controlPoints, 3);
			const int CURVE_RESOLUTION = 50;
			float depth = -0.0f;
			float weight = 3.f;
			std::vector<vec3> curvePoints;
			std::vector<vec2> curveCoords;
			std::vector<vec2> nsCurveCoords;
			for (int i = 0; i < CURVE_RESOLUTION; i++) {
				float u = float(i) / float(CURVE_RESOLUTION - 1);
				curvePoints.push_back(
					rationalBezierSpline(u, 
						controlPoints[0], 1.f,
						controlPoints[1], weight,
						controlPoints[2], 1.f) + vec3(0, 0, depth));
				curveCoords.push_back(vec2(
					2.f*arcLengthApprox(
						vec4(controlPoints[0], 1.f), 
						vec4(controlPoints[1], 1.f)*weight, 
						vec4(controlPoints[2], 1.f), 
						u), 0.51f));
				nsCurveCoords.push_back(vec2(2.f*u, 0.51f));
			}
			curveGeometry->loadBuffers(curvePoints.data(), curveCoords.data(), curvePoints.size());
			normalSpacingCurveGeometry->loadBuffers(curvePoints.data(), nsCurveCoords.data(), curvePoints.size());
			updated = false;

			auto b = [&](float u) { return rationalBezierSpline(u,
				controlPoints[0], 1.f,
				controlPoints[1], weight,
				controlPoints[2], 1.f); };

			float step = 0.00001f;
			vec3 tangentBeginning = (b(step) - b(0))/step;
			vec3 tangentMiddle = (b(0.5f + step) - b(0.5f - step)) / (2.f*step);

			printf("T(0) = %f\tT(0.5) = %f\n", length(tangentBeginning), length(tangentMiddle));
		}

	
		texShader.draw(cam, curveDrawable);
		texShader.draw(cam, normalSpacingDrawable);
		//shader.draw(cam, controlDrawable);
		for (int i = 0; i < 3; i++) {
			controlPointDrawable.position = controlPoints[i];
			shader.draw(cam, controlPointDrawable);
		}

		for (int i = 0; i < 3; i++) {
			controlPointDrawable.position = controlPoints[i]+offset;
			shader.draw(cam, controlPointDrawable);
		}

		glfwSwapBuffers(window);
		glfwWaitEvents();
	}

	glfwTerminate();
}

void WindowManager::simpleModelLoop() {
	glfwSetKeyCallback(window, keyCallback);
	glfwSetWindowSizeCallback(window, windowResizeCallback);

	unsigned int indices[6] = { 0, 1, 2, 1, 2, 3 };

	PinnedGeometry<attrib::Position, attrib::Pinned<attrib::Normal>> testGeometry(100, indices, 6);

	SimpleTexManager tm;

	BlinnPhongShader bpShader;

	//Dragon
	Drawable dragon(
		objToElementGeometry("models/dragon.obj"),
		make<ColorMat>(vec3(0.75f, 0.1f, 0.3f)));
	dragon.addMaterial(make<ShadedMat>(0.2, 0.4f, 0.4f, 1.f));
	//Plane
	Drawable plane(createPlaneGeometry(),
		make<ShadedMat>(0.2, 0.4f, 0.4f, 1.f));
	plane.addMaterial(make<ColorMat>(vec3(1.f, 0, 0)));
	plane.position = vec3(0, -0.3, 0);

	vec3 lightPos(10, 10, 10);

	glClearColor(0.f, 0.f, 0.f, 1.f);
	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (windowResized) {
			window_width = windowWidth;
			window_height = windowHeight;
			glViewport(0, 0, window_width, window_height);
		}

		bpShader.draw(cam, lightPos, dragon);
		bpShader.draw(cam, lightPos, plane);

		glfwSwapBuffers(window);
		glfwWaitEvents();
	}

	glfwTerminate();
}

void WindowManager::shadowLoop() {
	glfwSetKeyCallback(window, keyCallback);
	glfwSetWindowSizeCallback(window, windowResizeCallback);

	SimpleTexManager tm;

	BlinnPhongShader bpShader;
	ShadowShader<ColorMat> shadowShader;
	DepthVarianceShader depthShader;
	GaussianBlurShader blurShader;

	TrackballCamera shadowCam(
		normalize(vec3(-1, -1, -1)), vec3(2, 2, 2),
		glm::perspective(70.f*3.14159f / 180.f, 1.f, 0.1f, 100.f));

	Framebuffer fbWindow(window_width, window_height);

	const int SMAP_WIDTH = 1000;
	const int SMAP_HEIGHT = 1000;
	
	Framebuffer depthFramebuffer = createNewFramebuffer(SMAP_WIDTH, SMAP_HEIGHT);
	depthFramebuffer.addTexture(createTexture2D(TexInfo(GL_TEXTURE_2D, { SMAP_WIDTH, SMAP_HEIGHT }, 0,
		GL_RG, GL_RG32F, GL_FLOAT), &tm), GL_COLOR_ATTACHMENT0);
	depthFramebuffer.addTexture(createDepthTexture(SMAP_WIDTH, SMAP_HEIGHT, &tm), GL_DEPTH_ATTACHMENT);

	Framebuffer gaussianFramebuffer = createNewFramebuffer(SMAP_WIDTH, SMAP_HEIGHT);
	gaussianFramebuffer.addTexture(createTexture2D(TexInfo(GL_TEXTURE_2D, { SMAP_WIDTH, SMAP_HEIGHT }, 0,
		GL_RG, GL_RG32F, GL_FLOAT), &tm), GL_COLOR_ATTACHMENT0);
	gaussianFramebuffer.addTexture(createDepthTexture(SMAP_WIDTH, SMAP_HEIGHT, &tm), GL_DEPTH_ATTACHMENT);

	Drawable blurPlaneHorizontal(createPlaneGeometry(Orientation::PositiveZ),
		make<TextureMat>(depthFramebuffer.getTexture(GL_COLOR_ATTACHMENT0)));

	Drawable blurPlaneVertical(createPlaneGeometry(Orientation::PositiveZ),
		make<TextureMat>(gaussianFramebuffer.getTexture(GL_COLOR_ATTACHMENT0)));

	//Dragon
	Drawable dragon(
		objToElementGeometry("models/dragon.obj"),
		make<ColorMat>(vec3(0.75f, 0.1f, 0.3f)));
	dragon.addMaterial(make<ShadedMat>(0.2, 0.4f, 0.4f, 1.f));
	//Plane
	Drawable plane(createPlaneGeometry(),
		make<ShadedMat>(0.2, 0.4f, 0.4f, 1.f));
	plane.addMaterial(make<ColorMat>(vec3(1.f, 0, 0)));
	plane.position = vec3(0, -0.3, 0);

	vec3 lightPos(10, 10, 10);

	const int N = 5;
	const float SIGMA = 0.4f*float(N + 1);

	glClearColor(0.f, 0.f, 0.f, 1.f);
	while (!glfwWindowShouldClose(window)) {

		if (reloadShaders) {
			shadowShader = ShadowShader<ColorMat>();
			reloadShaders = false;
		}

		if (windowResized) {
			window_width = windowWidth;
			window_height = windowHeight;
			glViewport(0, 0, window_width, window_height);
		}

		depthFramebuffer.use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		depthShader.draw(shadowCam, dragon);
		depthShader.draw(shadowCam, plane);

		gaussianFramebuffer.use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		blurShader.draw(SIGMA, N, GaussianBlurShader::Direction::X, blurPlaneHorizontal);

		depthFramebuffer.use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		blurShader.draw(SIGMA, N, GaussianBlurShader::Direction::Y, blurPlaneVertical);

		fbWindow.use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shadowShader.draw(cam, lightPos, shadowCam, depthFramebuffer.getTexture(GL_COLOR_ATTACHMENT0), dragon);
		shadowShader.draw(cam, lightPos, shadowCam, depthFramebuffer.getTexture(GL_COLOR_ATTACHMENT0), plane);
		//bpShader.draw(cam, lightPos, dragon);
		//bpShader.draw(cam, lightPos, plane);

		glfwSwapBuffers(window);
		glfwWaitEvents();
	}

	glfwTerminate();
}

using PinnedColorGeometry = PinnedGeometry<
	attrib::Position,
	attrib::Normal,
	attrib::Pinned<attrib::ColorIndex>>;

void meshUpdateThread(
	Resource<PinnedColorGeometry::AttributePointers, 3>* attribResource,
	std::atomic_bool* shouldClose, size_t meshSize) 
{
	using namespace attrib;

	while (!(*shouldClose)) {
		static char color = 0;
		static int iteration = 0;
		color = (color + 1) % 4;
	
		auto attribWrite = attribResource->getWrite();
		printf("Color = %d    iteration = %d    id = %d\n", color, iteration, attribWrite.id);
		//for (int i = (iteration%3)*meshSize/3; i < meshSize / 3; i++) {
		for(int i=0; i<meshSize; i++){
			attribWrite->get<Pinned<ColorIndex>>()[i] = color;
		}
		iteration++;
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}

	printf("CLOSED------------------------\n");
}

void WindowManager::colorUpdatingLoop() {
	glfwSetKeyCallback(window, keyCallback);
	glfwSetWindowSizeCallback(window, windowResizeCallback);

	using namespace attrib;

	SimpleTexManager tm;

	BlinnPhongShader bpShader;

	//Dragon
	/*Drawable dragon(
		objToElementGeometry("models/dragon.obj"),
		make<ColorMat>(vec3(0.75f, 0.1f, 0.3f)));
	*/

	MeshInfoLoader minfo("models/dragon.obj");
	std::vector<unsigned char> colors;
	colors.resize(minfo.vertices.size(), 2);

	auto csMat = make<ColorSetMat>(std::vector<glm::vec3>{ 
		glm::vec3(1, 1, 1), 
		glm::vec3(1, 0, 0), 
		glm::vec3(0, 1, 0), 
		glm::vec3(0, 0, 1) });
	ColorShader colorShader(4);
	sptr<PinnedColorGeometry> dragonGeometry = make<PinnedColorGeometry>(minfo.vertices.size());
	dragonGeometry->loadIndices(minfo.indices.data(), minfo.indices.size());
	dragonGeometry->loadBuffers(minfo.vertices.data(), minfo.normals.data(), colors.data());
	Drawable dragon(dragonGeometry, csMat);
	dragon.addMaterial(make<ShadedMat>(0.2, 0.4f, 0.4f, 1.f));
	//Plane
	Drawable plane(createPlaneGeometry(),
		make<ShadedMat>(0.2, 0.4f, 0.4f, 1.f));
	plane.addMaterial(make<ColorMat>(vec3(1.f, 0, 0)));
	plane.position = vec3(0, -0.3, 0);

	vec3 lightPos(10, 10, 10);

	std::atomic_bool shouldClose = false;

	std::thread paintingThread (meshUpdateThread,
		&dragonGeometry->pinnedData, &shouldClose, colors.size());

	glClearColor(0.f, 0.f, 0.f, 1.f);
	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (windowResized) {
			window_width = windowWidth;
			window_height = windowHeight;
			glViewport(0, 0, window_width, window_height);
		}

		colorShader.draw(cam, lightPos, dragon);
		bpShader.draw(cam, lightPos, plane);

		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	shouldClose = true;

	paintingThread.join();

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

	Framebuffer gaussianFramebuffer2 = createNewFramebuffer(window_width, window_height);
	if (!gaussianFramebuffer2.addTexture(createTexture2D(TexInfo(GL_TEXTURE_2D, { window_width, window_height }, 0,
		GL_RGBA, GL_RGB32F, GL_FLOAT), &tm), GL_COLOR_ATTACHMENT0)
		|| !gaussianFramebuffer2.addTexture(createDepthTexture(window_width, window_height, &tm), GL_DEPTH_ATTACHMENT)) {
		printf("Failed to initialize gaussianFramebuffer\n");
	}
	
	Framebuffer fbWindow(window_width, window_height);

	Camera identityCam;

	FlatColorShader flatShader;
	GaussianBlurShader gaussianShader;

	Drawable texSquareHorizontal(
		make<GeometryT<attrib::Position, attrib::TexCoord>>(GL_TRIANGLES, points, coords, 6),
		make<TextureMat>(lightFramebuffer.getTexture(GL_COLOR_ATTACHMENT0)));

	Drawable texSquareVertical(
		make<GeometryT<attrib::Position, attrib::TexCoord>>(GL_TRIANGLES, points, coords, 6),
		make<TextureMat>(gaussianFramebuffer.getTexture(GL_COLOR_ATTACHMENT0)));

	Drawable texSquareFinal(
		make < GeometryT < attrib::Position, attrib::TexCoord>>(GL_TRIANGLES, points, coords, 6),
		make<TextureMat>(gaussianFramebuffer2.getTexture(GL_COLOR_ATTACHMENT0))
	);


	BlinnPhongShader bpShader;
	SimpleTexShader texShader;

	//Dragon
	Drawable dragon(
		objToElementGeometry("models/dragon.obj"),
		make<ColorMat>(vec3(0.75f, 0.1f, 0.3f)));

	Drawable glowDragon(
		objToElementGeometry("models/dragon.obj"),
		//make<ColorMat>(vec3(0.75f, 0.1f, 0.3f)*5.f));
		//make<ColorMat>(vec3(0.2f, 0.4f, 1.f)*5.f));
		make<ColorMat>(vec3(1.f, 0.2f, 0.6f)*5.f));

	const int N = 100;
	const float SIGMA = 0.4f*float(N + 1);

	glClearColor(0.f, 0.f, 0.f, 1.f);
	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (windowResized) {
			window_width = windowWidth;
			window_height = windowHeight;
			glViewport(0, 0, window_width, window_height);
		}

		int texelOffsetMin;
		glGetIntegerv(GL_MIN_PROGRAM_TEXEL_OFFSET, &texelOffsetMin);
		int texelOffsetMax;
		glGetIntegerv(GL_MAX_PROGRAM_TEXEL_OFFSET, &texelOffsetMax);

		printf("Min = %d Max = %d\n", texelOffsetMin, texelOffsetMax);

		lightFramebuffer.use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		flatShader.draw(cam, glowDragon);

		gaussianFramebuffer.use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		gaussianShader.draw(SIGMA, N, GaussianBlurShader::Direction::X, texSquareHorizontal);

		gaussianFramebuffer2.use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		gaussianShader.draw(SIGMA, N, GaussianBlurShader::Direction::Y, texSquareVertical);

		fbWindow.use();
		texShader.draw(identityCam, texSquareFinal);

		glClear(GL_DEPTH_BUFFER_BIT);
		//bpShader.draw(cam, vec3(10, 10, 10), dragon);
		flatShader.draw(cam, glowDragon);

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
	//NormalIndexGeometry testGeom (GL_TRIANGLES, planeIndices, 6, planePoints, planeNormals, 4, 1);
	Drawable plane(make<NormalIndexGeometry>(GL_TRIANGLES, planeIndices, 6, planePoints, planeNormals, 4, 1),
		make<ShadedMat>(0.2, 0.4f, 0.4f, 1.f));
	plane.addMaterial(make<ColorMat>(vec3(1.f, 0, 0)));
	
	SimpleTexManager tm;
	MeshInfoLoader minfo("models/coryPrism.obj");

	Drawable rectPrism(make<TexNormalIndexGeometry>(GL_TRIANGLES,
		minfo.indices.data(), minfo.indices.size(),
		minfo.vertices.data(), minfo.normals.data(), minfo.uvs.data(), 
		minfo.vertices.size()), 
		make<ShadedMat>(0.2f, 0.4f, 0.4f, 10.f));
	rectPrism.addMaterial(new ColorMat(vec3(1.f)));

	BlinnPhongShader bpShader;

	/*
	cam = TrackballCamera(
		vec3(0, 0, -1), vec3(0, 0, 5),
		glm::perspective(90.f*3.14159f / 180.f, 1.f, 0.1f, 20.f));
		*/
	cam.projection = glm::perspective(90.f*3.14159f / 180.f, 1.f, 0.1f, 1000.f),
		cam.center = glm::vec3(0, 0, -1);
	cam.zoom(5.f);

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

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
}

bool leftButtonPressed = false;
bool inputHandled = false;
glm::vec2 lastClickPosition;

//Temporary testing
void WindowManager::adaptiveNoiseLoop() {
	//Original main loop
	glfwSetKeyCallback(window, keyCallback);
	glfwSetWindowSizeCallback(window, windowResizeCallback);
	//glfwSetCursorPosCallback(window, adaptiveCursorPosCallback);

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

	Camera cam;

	adaptive::SimpleNoiseField simpNoise(5, 5);

	validateTopFaces(simpNoise.noise);

	const size_t DIMENSION = 1000;
	glm::vec3* image = new glm::vec3[DIMENSION*DIMENSION];
	for (int y = 0; y < DIMENSION; y++) {
		for (int x = 0; x < DIMENSION; x++) {
			image[y*DIMENSION + x] = vec3(1.f)*simpNoise.evaluateAt(glm::vec2(float(x) / float(DIMENSION - 1), float(y) / float(DIMENSION - 1)));
		}
	}
	//GeometryT<vec3, vec4, float> testGeometry;
	SimpleTexShader texShader;	
	auto texGeometry = make<GeometryT<attrib::Position, attrib::TexCoord>>(GL_TRIANGLES, points, coords, 6);
	Texture noiseTex = createTexture2DFromData(TexInfo(GL_TEXTURE_2D, { int(DIMENSION), int(DIMENSION) }, 0, GL_RGB, GL_RGB16F, GL_FLOAT), &tm, image);
	Drawable texDrawable(texGeometry, std::make_shared<TextureMat>(noiseTex));
	
	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !leftButtonPressed) {
			leftButtonPressed = true;

			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
			int vp[4];
			glGetIntegerv(GL_VIEWPORT, vp);
			vec2 mousePos = vec2(float(xpos) / float(vp[2]),
				float(-ypos) / float(vp[3]) + 1.f);
			lastClickPosition = mousePos;

			simpNoise.subdivideSquare(lastClickPosition);
			for (int y = 0; y < DIMENSION; y++) {
				for (int x = 0; x < DIMENSION; x++) {
					image[y*DIMENSION + x] = vec3(1.f)*simpNoise.evaluateAt(glm::vec2(float(x) / float(DIMENSION - 1), float(y) / float(DIMENSION - 1)));
				}
			}

			glActiveTexture(NO_ACTIVE_TEXTURE);
			glBindTexture(GL_TEXTURE_2D, noiseTex.getID());
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, DIMENSION, DIMENSION, 0, GL_RGB, GL_FLOAT, image);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		else if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) != GLFW_PRESS && leftButtonPressed){
			leftButtonPressed = false;
		}

		if (windowResized) {
			window_width = windowWidth;
			window_height = windowHeight;
		}

		texShader.draw(cam, texDrawable);

		glfwSwapBuffers(window);
		glfwWaitEvents();
	}

	glfwTerminate();
}

void WindowManager::petioleAlignmentLoop()
{

	glfwSetKeyCallback(window, keyCallback);
	glfwSetWindowSizeCallback(window, windowResizeCallback);

	SimpleTexManager tm;

	BlinnPhongShader bpShader;
	BlinnPhongShader bpTexShader(BPTextureUsage::TEXTURE);
	SimpleShader colorShader;

	//Plane
	Drawable plane(createPlaneGeometry(),
		make<ShadedMat>(0.5, 0.4f, 0.4f, 1.f));
	//plane.addMaterial(make<ColorMat>(vec3(0, 1, 0)));
	plane.addMaterial(make<TextureMat>(createTexture2D("textures/green_teardrop_leaf_front.png", &tm)));
	plane.position = vec3(0, -0.3, 0);
	
	auto lineGeometry = make<PositionGeometry>(GL_LINES);
	Drawable lineDrawable(lineGeometry, make<ColorMat>(vec3(1, 0, 0)));

	vec3 lightPos(10, 10, 10);

	//Tangent angle
	float azimuth = 0.f;
	float altitude = 0.f;
	float roll = 0.f;
	
	bool update = true;
	
	PetioleInfo pinfo = { vec3(0.f), vec3(1, 0, 0), vec3(2, 0, 0), quat() };
	quat initialOrientation = quat();

	auto targetLineGeometry = make<PositionGeometry>(GL_LINE_LOOP);
	Drawable targetLineDrawable(targetLineGeometry, make<ColorMat>(vec3(0, 1, 0)));

	auto yellowGeometry = make<PositionGeometry>(GL_LINES);
	Drawable yellowDrawable(yellowGeometry, make<ColorMat>(vec3(1, 0.8, 0.6)));

	auto blueGeometry = make<PositionGeometry>(GL_LINE_LOOP);
	Drawable blueDrawable(blueGeometry, make<ColorMat>(vec3(0.2, 0.5, 0.8)));

	auto purpleGeometry = make<PositionGeometry>(GL_LINES);
	Drawable purpleDrawable(purpleGeometry, make<ColorMat>(vec3(0.9, 0.9, 0.6)));

	windowResized = true;
	glfwGetWindowSize(window, &windowWidth, &windowHeight);

	altitude = -0.310;
	azimuth = 0;
	roll = -0.66;
	cam.altitude = 0.220463f;
	cam.azimuth = 0.753396f;

	glLineWidth(2.f);

	glClearColor(0.f, 0.f, 0.f, 1.f);
	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		static float targetRotation = 0.f;

		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
			roll += 0.01f;
			update = true;
		}
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
			roll -= 0.01f;
			update = true;
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			altitude = glm::clamp(altitude + 0.01f, -0.9f*glm::half_pi<float>(), 0.9f*glm::half_pi<float>());
			printf("Altitude = %f, Azimuth = %f, Roll = %f\n", altitude, azimuth, roll);
			update = true;
		}
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			altitude = glm::clamp(altitude - 0.01f, -0.9f*glm::half_pi<float>(), 0.9f*glm::half_pi<float>());
			printf("Altitude = %f, Azimuth = %f, Roll = %f\n", altitude, azimuth, roll);
			printf("Camera alt = %f, Camera azi = %f\n", cam.altitude, cam.azimuth);
			update = true;
		}
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
			targetRotation += 0.01f;
			update = true;
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
			targetRotation -= 0.01f;
			update = true;
		}

		const float angleFromVertical = (1.f/6.f)*M_PI;

		if (update) {

			std::vector<vec3> targetLinePoints;
			for (float azi = 0.f; azi < 2.f*M_PI; azi += M_PI / 10.f) {
				targetLinePoints.push_back(vec3(
					cos(0.5f*M_PI - angleFromVertical)*cos(azi),
					sin(0.5f*M_PI - angleFromVertical),
					cos(0.5f*M_PI - angleFromVertical)*sin(azi)));
			}

			targetLineGeometry->loadBuffers(targetLinePoints.data(), targetLinePoints.size());

			vec3 tangent(cos(altitude)*cos(azimuth), sin(altitude), cos(altitude)*sin(azimuth));
			vec3 normal = angleAxis(roll, tangent)*normalize(cross(tangent, vec3(0, 1, 0)));
			initialOrientation = mat3(cross(tangent, normal), normal, -tangent);
			pinfo = petioleRotation(vec3(0.f), tangent, normal, 0.5f, angleFromVertical, glm::half_pi<float>(), glm::pi<float>()*0.1f);
			/*float u = std::max(pinfo.originalLambertion, 0.f);
			quat endOrientation = normalize(
				u * initialOrientation +
				(1-u) * pinfo.endOrientation);*/
			std::vector<vec3> points = {
				vec3(0), vec3(0, 1.5f, 0),
				pinfo.start, pinfo.start + normal
			};
			std::vector<vec3> purplePoints = {
				pinfo.start, pinfo.middle,
				pinfo.middle, pinfo.end,
				pinfo.start, pinfo.start + normal,
				pinfo.end, pinfo.end + pinfo.endOrientation*vec3(0, 1, 0)
			};//*/
			lineGeometry->loadBuffers(points.data(), points.size());
			purpleGeometry->loadBuffers(purplePoints.data(), purplePoints.size());

			vector<vec3> yellowPoints = { vec3(0), pinfo.endOrientation*vec3(0, 1, 0) };
			for (float u = 0; u < 1.f; u += 0.01f) {
				quat orientation = normalize(u * initialOrientation + (1 - u)*pinfo.endOrientation);
				yellowPoints.push_back(yellowPoints.back());
				yellowPoints.push_back(orientation*vec3(0, 1, 0));
			}


			yellowPoints.push_back(initialOrientation*vec3(0, 0, -1));
			yellowPoints.push_back(initialOrientation*vec3(0, 0, -1));
			/*
			for (float u = 0; u < 1.f; u += 0.01f) {
				quat orientation = normalize(u * initialOrientation + (1 - u)*pinfo.endOrientation);
				yellowPoints.push_back(yellowPoints.back());
				yellowPoints.push_back(orientation*vec3(0, 0, -1));
			}*/
			vec3 peak = normalize(tangent.y*tangent + normal.y*normal);
			if (peak.y < cos(angleFromVertical)) {
				yellowPoints.push_back(peak);
				yellowPoints.push_back(1.2f*yellowPoints.back());
			}
			else {
				float height = cos(angleFromVertical);
				vec3 center = peak * (height / peak.y);
				vec2 toCenter = vec2(center.x, center.z);
				float dist = sqrt(1.f - height*height - dot(toCenter, toCenter)); 
				vec3 toEdge = normalize(vec3(-center.z, 0, center.x))*dist;
				yellowPoints.push_back(center + toEdge);
				yellowPoints.push_back(yellowPoints.back()*1.2f);

				yellowPoints.push_back(center - toEdge);
				yellowPoints.push_back(yellowPoints.back()*1.2f);
			}
			yellowGeometry->loadBuffers(yellowPoints.data(), yellowPoints.size());

			vector<vec3> bluePoints;
			for (float u = 0; u < M_PI*2.f; u += 0.01f) {
				bluePoints.push_back(tangent*sin(u) + normal * cos(u));
			}
			blueGeometry->loadBuffers(bluePoints.data(), bluePoints.size());
			update = false;
		}

		if (windowResized) {
			window_width = windowWidth;
			window_height = windowHeight;
			glViewport(0, 0, window_width, window_height);

			float windowRatio = float(window_width) / float(window_height);
			float windowSize = 2.f;
			cam.projection = glm::ortho(-windowRatio*windowSize, windowRatio*windowSize, -windowSize, windowSize, 0.001f, 100.f);
		}

		plane.setScale(vec3(0.3f));

		colorShader.draw(cam, lineDrawable);
		colorShader.draw(cam, targetLineDrawable);
		colorShader.draw(cam, purpleDrawable);
		colorShader.draw(cam, yellowDrawable);
		colorShader.draw(cam, blueDrawable);
		plane.position = pinfo.start;
		plane.orientation = initialOrientation;
		bpTexShader.draw(cam, lightPos, plane);

		plane.position = pinfo.end;
		plane.orientation = pinfo.endOrientation;
		bpTexShader.draw(cam, lightPos, plane);

		glfwSwapBuffers(window);
		glfwWaitEvents();
	}

	glfwTerminate();
}

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
	SimpleTexManager tm;

	//AO framebuffer
	Framebuffer pnFbo = createNewFramebuffer(window_width, window_height);
	pnFbo.addTexture(createTexture2D(
		TexInfo(GL_TEXTURE_2D, { window_width, window_height }, 0, GL_RGB, GL_RGB32F, GL_FLOAT), &tm),
		GL_COLOR_ATTACHMENT0);

	pnFbo.addTexture(createTexture2D(
		TexInfo(GL_TEXTURE_2D, { window_width, window_height }, 0, GL_RGB, GL_RGB32F, GL_FLOAT), &tm),
		GL_COLOR_ATTACHMENT1);
	pnFbo.addTexture(createDepthTexture(window_width, window_height, &tm), GL_DEPTH_ATTACHMENT);

	Framebuffer fbWindow(window_width, window_height);
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
		make<ColorMat>(vec3(0.75f, 0.1f, 0.3f)));
	dragon.addMaterial(new ShadedMat(0.2f, 0.5f, 0.3f, 10.f));

	Drawable texSquare(
		make<GeometryT<attrib::Position, attrib::TexCoord>>(GL_TRIANGLES, points, coords, 6),
		make<TextureMat>(fbTex.getTexture(GL_COLOR_ATTACHMENT0)));

	texSquare.addMaterial(make<TextureMat>(pnFbo.getTexture(GL_COLOR_ATTACHMENT0), TextureMat::POSITION));
	texSquare.addMaterial(make<TextureMat>(pnFbo.getTexture(GL_COLOR_ATTACHMENT1), TextureMat::NORMAL));

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

void WindowManager::treeGrowthTest() {
	SimpleShader shader;
	
	auto pointGeometry = make<GeometryT<attrib::Position>>(GL_POINTS);
	auto lineGeometry = make<GeometryT<attrib::Position>>(GL_LINES);

	Drawable pointDrawable(pointGeometry, make<ColorMat>(vec3(1, 0, 0)));
	Drawable lineDrawable(lineGeometry, make<ColorMat>(vec3(0, 1, 0)));

	vector<vec3> lines = { vec3(0, -1, 0), vec3(0, 0, 0), vec3(0,0, 0), vec3(0, 1, 0), vec3(0, 0, 0), vec3(0.5, 0.5, 0)};
	lineGeometry->loadBuffers(lines.data(), lines.size());

	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.draw(cam, lineDrawable);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void WindowManager::particleLoop() {
	HeatParticleShader pShader;

	HeatParticleSystem pSystem;
	auto pGeometry = make<HeatParticleGeometry>(GL_TRIANGLE_FAN);
	auto pMat = make<HeatParticleMat>(0.07f);

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

		//pGeometry->loadParticles(pSystem.particles.data(), pSystem.particles.size());
		updateHeatParticleGeometry(*pGeometry, pSystem.particles.data(), pSystem.particles.size());

		pShader.draw(cam, pDrawable);

		timeElapsed = currentTime;

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}
/*
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
*/

/*
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
*/

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

