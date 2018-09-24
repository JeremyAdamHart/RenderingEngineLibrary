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

//Other
#include "StreamGeometry.h"
#include "ColorShader.h"
#include "ColorSetMat.h"

//Rigid body test
#include "Physics.h"

//Convex Hull
#include "ConvexHull.h"

//Random
#include <random>
#include <ctime>

#include <limits>

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

void WindowManager::testLoop() {
	srand(13);	//time(0));

	SlotMap<int> testMap;
	testMap.add(1);
	testMap.add(2);
	testMap.add(3);
	testMap.add(4);
	auto last = testMap.add(5);
	testMap.remove(last);

	MeshInfoLoader modelMinfo("models/dragon.obj");
	ElementGeometry modelGeom(modelMinfo.vertices.data(), modelMinfo.normals.data(), nullptr, 
		modelMinfo.indices.data(), modelMinfo.vertices.size(), modelMinfo.indices.size());

	Drawable modelDrawable(new ColorMat(vec3(0, 1, 0)), &modelGeom);
	modelDrawable.addMaterial(new ShadedMat(0.3f, 0.4f, 0.4f, 5.f));

	for (auto it = testMap.begin(); it != testMap.end(); ++it) {
		//auto ending = testMap.end();
  		printf("%d\n", *it);
	}

	std::vector<int>::iterator it;

	cam = TrackballCamera(
		vec3(0, 0, -1), vec3(0, 0, 5),
		glm::perspective(90.f*3.14159f / 180.f, 1.f, 0.1f, 20.f));

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
	TorranceSparrowShader tsShader;
	ElementGeometry geom(points.data(), normals.data(), nullptr, indices.data(), points.size(), indices.size());
	//enum {POSITIONS=0, NORMALS};
	//StreamGeometry<vec3, vec3> geom({ false, false })

	Drawable heObject(new ColorMat(vec3(1, 0, 0)), &geom);
	heObject.addMaterial(new ShadedMat(0.3, 0.5, 0.4, 10.f));



	glPointSize(3.f);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	Drawable hullObject(new ColorMat(vec3(1, 1, 1)),
		new SimpleGeometry(hullPoints.data(), hullPoints.size(), GL_POINTS));

	vector<vec3> debugPoints;
	SimpleGeometry debugGeometry(GL_POINTS);
	Drawable debugObject(new ColorMat(vec3(0.2, 0.5, 1)), &debugGeometry);

	SimpleGeometry halfEdgeDebugGeometry(GL_LINES);
	Drawable halfEdgeDebugObject(new ColorMat(vec3(1.f, 0.f, 1.f)), &halfEdgeDebugGeometry);

	SimpleGeometry halfEdgeTrackingGeometry(GL_LINES);
	Drawable halfEdgeTrackingObject(new ColorMat(vec3(0, 1.f, 0.f)), &halfEdgeTrackingGeometry);
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
				geom.loadGeometry(points.data(), normals.data(), texCoords.data(), &indices[0], points.size(), indices.size());
				debugGeometry.loadGeometry(debugPoints.data(), debugPoints.size());
				
			}
			
			iterationPressed = true;
		}
		else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE){
			iterationPressed = false;
		}

		static bool pKeyPressed = false;
		if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !pKeyPressed) {
			pKeyPressed = true;
			vec3 normal = (!mesh.isBoundary(trackingEdge)) ? mesh.normal(mesh[mesh[trackingEdge].face]) : vec3(0.f);
			trackingEdge = mesh[trackingEdge].pair;
			vec3 endPoints[2] = { mesh[mesh[trackingEdge].head].pos + normal*0.01f, mesh[mesh[mesh.prev(trackingEdge)].head].pos + normal*0.01f };
			halfEdgeTrackingGeometry.loadGeometry(endPoints, 2);

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
			debugGeometry.loadGeometry(arr, 2);
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
			halfEdgeTrackingGeometry.loadGeometry(endPoints, 2);
		}
		else if(glfwGetKey(window, GLFW_KEY_N) == GLFW_RELEASE){
			nKeyPressed = false;
		}

		if (windowResized) {
			window_width = windowWidth;
			window_height = windowHeight;
			glViewport(0, 0, window_width, window_height);
		}

		loadGeometryWithHalfEdgeMesh(&halfEdgeDebugGeometry, mesh);

		simpleShader.draw(cam, halfEdgeTrackingObject);
//		simpleShader.draw(cam, hullObject);
		simpleShader.draw(cam, debugObject);
		simpleShader.draw(cam, halfEdgeDebugObject);
		tsShader.draw(cam, glm::vec3(10, 10, 10), heObject);
		tsShader.draw(cam, glm::vec3(10, 10, 10), modelDrawable);

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
		new TextureMat(createTexture2D(1, 1, &tm)),
		new SimpleTexGeometry(points, coords, 6, GL_TRIANGLES));

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

	ElementGeometry planeGeom(planePoints, planeNormals, nullptr, planeIndices, 4, 6);
	Drawable plane(new ShadedMat(0.2, 0.4f, 0.4f, 1.f), &planeGeom);
	plane.addMaterial(new ColorMat(vec3(1.f, 0, 0)));

	SimpleTexManager tm;
	MeshInfoLoader minfo("models/coryPrism.obj");
	ElementGeometry rectPrismGeom(
		minfo.vertices.data(), minfo.normals.data(), minfo.uvs.data(), minfo.indices.data(), 
		minfo.vertices.size(), minfo.indices.size());

	Drawable rectPrism(new ShadedMat(0.2f, 0.4f, 0.4f, 10.f), &rectPrismGeom);
	rectPrism.addMaterial(new ColorMat(vec3(1.f)));

	TorranceSparrowShader tsShader;

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
		tsShader.draw(cam, lightPos, rectPrism);
		tsShader.draw(cam, lightPos, plane);

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
	StreamGeometry<vec3, vec3, unsigned char> streamGeometry(minfo.vertices.size(), 
	{ false, false, true});
	streamGeometry.loadElementArray(minfo.indices.size(), GL_STATIC_DRAW, minfo.indices.data());

	vector<unsigned char> colors(minfo.vertices.size(), 1);

	streamGeometry.loadBuffer<POSITION>(minfo.vertices.data());
	streamGeometry.loadBuffer<NORMAL>(minfo.normals.data());
	streamGeometry.loadBuffer<COLOR>(colors.data());

	drawables.push_back(Drawable(new ShadedMat(0.3, 0.4, 0.4, 10.f), &streamGeometry));
	drawables[0].addMaterial(new ColorSetMat({ vec3(1, 0, 0), vec3(0, 0, 1) }));

	TorranceSparrowShader tsShader;
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

		int writeIndex = streamGeometry.buffManager.getWrite();

		interval++;
		if (interval >= maxInterval) {
			useColor1 = !useColor1;
			interval = 0;
		}

		for (int i = 0; i < drawables.size(); i++) {
			colorShader.draw(cam, lightPos, drawables[i]);
//			tsShader.draw(cam, lightPos, drawables[i]);
		}
	
//		int bufferNum = streamGeometry.buffManager.getWrite();
		int offset = 0;
		unsigned char *color = streamGeometry.vboPointer<COLOR>();
		const int UPDATE_NUM = 10000;
		for (int i = 0; i < UPDATE_NUM; i++) {
			if (counter + i >= streamGeometry.getBufferSize() && offset == 0) {
				offset = -int(streamGeometry.getBufferSize());
				drawColor = (drawColor == 0) ? 1 : 0;
			}
			int index = (counter + i) + offset;
//			color[index] = (color[index] == 0)?  1 : 0;
			streamGeometry.modify<COLOR>(index, drawColor);
		}

		counter = counter+UPDATE_NUM+offset;
		
		streamGeometry.dump<COLOR>();
		streamGeometry.buffManager.endWrite();
		
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

