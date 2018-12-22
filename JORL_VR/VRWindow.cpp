#include "VRWindow.h"

#include <iostream>

using namespace glm;
using namespace std;

#include "Drawable.h"
#include "SimpleGeometry.h"

#include "SimpleShader.h"
#include "simpleTexShader.h"
#include "BlinnPhongShader.h"
#include "AOShader.h"
#include "PosNormalShader.h"

#include "ColorMat.h"
#include "ShadedMat.h"
#include "TextureMat.h"

#include "TrackballCamera.h"
#include "TextureCreation.h"
#include "Framebuffer.h"
#include "SimpleTexManager.h"
#include "MeshInfoLoader.h"
#include "ModelLoader.h"

//Painting
#include "StreamGeometry.h"
#include "ColorShader.h"
#include "BubbleShader.h"
#include "ColorSetMat.h"
#include "kd_tree.h"
#include "VolumeIO.h"
#include "UndoStack.h"
#include "ColorWheel.h"
#include "VRColorShader.h"

#include "VRController.h"
#include "VRView.h"

//Screenshot
#pragma warning(disable:4996)
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include <glm/gtc/matrix_transform.hpp>

const float PI = 3.14159265358979323846;

using namespace renderlib;

int gWindowWidth, gWindowHeight;

bool reloadShaders = false;

TrackballCamera cam(
	vec3(0, 0, -1), vec3(0, 0, 1),
	//	glm::perspective(90.f*3.14159f/180.f, 1.f, 0.1f, 3.f));
	mat4(1.f));

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

void windowResizeCallback(GLFWwindow *window, int wWidth, int wHeight) {
	gWindowWidth = wWidth;
	gWindowHeight = wHeight;
}

WindowManager::WindowManager() :
	window_width(800), window_height(800)
{
	glfwInit();
	window = createWindow(window_width, window_height,
		"You really should rename this");
	initGlad();
	vrDisplay = initVR();

	glfwSwapInterval(0);

	initGL();
}

WindowManager::WindowManager(int width, int height, std::string name, glm::vec4 color) :
	window_width(width), window_height(height)
{
	glfwInit();
	window = createWindow(window_width, window_height, name);
	initGlad();
	vrDisplay = initVR();

	glfwSwapInterval(0);

	initGL();
}

void printMat4(const mat4 &m) {
	printf("[%.2f, %.2f, %.2f, %.2f]\n[%.2f, %.2f, %.2f, %.2f]\n[%.2f, %.2f, %.2f, %.2f]\n[%.2f, %.2f, %.2f, %.2f]\n\n",
		m[0][0], m[0][1], m[0][2], m[0][3],
		m[1][0], m[1][1], m[1][2], m[1][3],
		m[2][0], m[2][1], m[2][2], m[2][3],
		m[3][0], m[3][1], m[3][2], m[3][3]);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE)
		reloadShaders = true;
}

void WindowManager::mainLoopNoAO() {
	glfwSetCursorPosCallback(window, cursorPositionCallback);

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
		vec2(1, 0.f),
		vec2(0.f, 0.f),
		vec2(0.f, 1.f),
		//Second triangle
		vec2(0.f, 1.f),
		vec2(1.f, 1.f),
		vec2(1.f, 0.f)
	};

	SimpleTexManager tm;

	if (vrDisplay == nullptr) {
		vr::VR_Shutdown();
		glfwTerminate();
		return;
	}

	Framebuffer fbWindow(window_width, window_height);
	unsigned int TEX_WIDTH = 800;
	unsigned int TEX_HEIGHT = 800;
	vrDisplay->GetRecommendedRenderTargetSize(&TEX_WIDTH, &TEX_HEIGHT);

	Framebuffer fbLeftEyeDraw = createNewFramebuffer(TEX_WIDTH, TEX_HEIGHT);
	Framebuffer fbRightEyeDraw = createNewFramebuffer(TEX_WIDTH, TEX_HEIGHT);
	Framebuffer fbLeftEyeRead = createNewFramebuffer(TEX_WIDTH, TEX_HEIGHT);
	Framebuffer fbRightEyeRead = createNewFramebuffer(TEX_WIDTH, TEX_HEIGHT);


	const int NUM_SAMPLES = 8;

	if (!fbLeftEyeDraw.addTexture(
		createTexture2DMulti(TEX_WIDTH, TEX_HEIGHT, &tm, NUM_SAMPLES),
		GL_COLOR_ATTACHMENT0) ||
		!fbLeftEyeDraw.addTexture(
			createDepthTextureMulti(TEX_WIDTH, TEX_HEIGHT, &tm, NUM_SAMPLES), GL_DEPTH_ATTACHMENT))
	{
		std::cout << "FBO creation failed" << endl;
	}
	if (!fbRightEyeDraw.addTexture(
		createTexture2DMulti(TEX_WIDTH, TEX_HEIGHT, &tm, NUM_SAMPLES),
		GL_COLOR_ATTACHMENT0) ||
		!fbRightEyeDraw.addTexture(
			createDepthTextureMulti(TEX_WIDTH, TEX_HEIGHT, &tm, NUM_SAMPLES), GL_DEPTH_ATTACHMENT))
	{
		std::cout << "FBO creation failed" << endl;
	}

	if (!fbLeftEyeRead.addTexture(
		createTexture2D(TEX_WIDTH, TEX_HEIGHT, &tm), GL_COLOR_ATTACHMENT0)) {
		std::cout << "FBO creation failed" << endl;
	}
	else if (!fbRightEyeRead.addTexture(
		createTexture2D(TEX_WIDTH, TEX_HEIGHT, &tm), GL_COLOR_ATTACHMENT0)) {
		std::cout << "FBO creation failed" << endl;
	}

	Viewport leftEyeView(window_width / 2, window_height);
	Viewport rightEyeView(window_width / 2, window_height, window_width / 2);

	//Parse tracked devices
	int headsetIndex = 0;
	vector<VRController> controllers;
	vr::TrackedDevicePose_t poses[vr::k_unMaxTrackedDeviceCount];

	for (int i = 0; i < vr::k_unMaxTrackedDeviceCount; i++) {
		vr::TrackedDeviceClass deviceClass = vrDisplay->GetTrackedDeviceClass(i);

		switch (deviceClass) {
		case vr::TrackedDeviceClass_HMD:
			headsetIndex = i;
			break;
		case vr::TrackedDeviceClass_Controller:
			controllers.push_back(VRController(i, vrDisplay, poses[i], &tm));
			break;
		}
	}

	VRCameraController vrCam(&poses[headsetIndex], vrDisplay);

	//Dragon
	auto dragonGeom = shared_ptr<ElementGeometry>(objToElementGeometry("models/dragon.obj"));
	Drawable dragon(
		dragonGeom,
		make_shared<ColorMat>(vec3(0.75f, 0.1f, 0.3f))
	);
	dragon.addMaterial(new ShadedMat(0.2f, 0.5f, 0.3f, 10.f));

	dragon.setPosition(vec3(1.f, 0, 0));
	dragon.setOrientation(angleAxis(-PI*0.5f, vec3(0.f, 1.f, 0.f)));

	auto sphereGeom = shared_ptr<ElementGeometry>(objToElementGeometry("models/icosphere.obj"));
	Drawable sphere(
		sphereGeom,
		make_shared<ColorMat>(vec3(0.1, 0.3f, 0.8f))
	);
	sphere.addMaterial(new ShadedMat(0.2f, 0.5f, 0.3f, 10.f));

	sphere.setPosition(vec3(1.f, 0, 0));

	//Squares for left and right views
	Drawable leftSquare(
		new SimpleTexGeometry(points, coords, 6, GL_TRIANGLES),
		new TextureMat(fbLeftEyeRead.getTexture(GL_COLOR_ATTACHMENT0)));

	Drawable rightSquare(
		new SimpleTexGeometry(points, coords, 6, GL_TRIANGLES),
		new TextureMat(fbRightEyeRead.getTexture(GL_COLOR_ATTACHMENT0)));

	SimpleTexShader texShader;
	SimpleShader shader;
	BlinnPhongShader bpShader;
	BlinnPhongShader bpTexShader(BPTextureUsage::TEXTURE);

	TrackballCamera savedCam = cam;

	vec3 lightPos(-100.f, 100.f, 100.f);

	fbLeftEyeDraw.use();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	bpShader.draw(cam, lightPos, dragon);

	fbWindow.use();

	vector<Drawable> drawables;

	auto objGeometry = shared_ptr<ElementGeometry>(objToElementGeometry("untrackedmodels/riccoSurface_take3.obj"));

	drawables.push_back(Drawable(objGeometry, make_shared<ShadedMat>(0.3, 0.4, 0.4, 10.f)));
	drawables[0].addMaterial(new ColorMat(vec3(1, 1, 1)));

	for (int i = 0; i < drawables.size(); i++) {
		drawables[i].setPosition(vec3(0, 0, -6.f));
		drawables[i].setScale(vec3(10.0));
	}

	vector<vec3> controllerPositions(controllers.size());

	quat perFrameRot = angleAxis(3.14159f / 90.f, vec3(0, 1, 0));

	//Velocity
	vec3 linearVelocity(0.f);
	quat angularVelocity = quat();

	vector<vec3> tempData (objGeometry->numElements(), vec3(0, 1, 0));

	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Get pose
		vr::VRCompositor()->WaitGetPoses(poses, vr::k_unMaxTrackedDeviceCount,
			NULL, 0);

		//Update camera
		vrCam.update();
		vrCam.setProjection(vrDisplay, 0.2f, 400.f);

		//Update controllers
		vector<int> triggersPressed;
		for (int i = 0; i < controllers.size(); i++) {
			vr::VRControllerState_t state;
			vr::TrackedDevicePose_t pose;
			if (!vrDisplay->GetControllerStateWithPose(
				vr::VRCompositor()->GetTrackingSpace(),
				controllers[i].index,
				&state,
				sizeof(vr::VRControllerState_t),
				&pose))
			{
				printf("Error reading controller state\n");
			}

			controllers[i].updatePose(poses[controllers[i].index]);
			controllers[i].updateState(state);
			if (controllers[i].axes[VRController::TRIGGER_AXIS].x > 0.5f)
				triggersPressed.push_back(i);
		}

		vec3 positionTransform(0.f);
		quat orientationTransform;

		bool updatePositions = true;

		switch (triggersPressed.size()) {
		case 1:
		{
			vec3 lastPos = controllerPositions[triggersPressed[0]];
			positionTransform = controllers[triggersPressed[0]].getPos() - lastPos;
			linearVelocity = positionTransform;
			break;
		}
		case 2:
		{
			vec3 axisA = normalize(controllerPositions[triggersPressed[0]]
				- controllerPositions[triggersPressed[1]]);
			vec3 axisB = normalize(controllers[triggersPressed[0]].getPos()
				- controllers[triggersPressed[1]].getPos());
			vec3 rotAxis = cross(axisA, axisB);
			if (length(rotAxis) > 0.0001) {
				float angle = asin(length(rotAxis));
				orientationTransform = angleAxis(angle, normalize(rotAxis));
				angularVelocity = orientationTransform;
			}
			else
				updatePositions = false;
			break;

		}
		default:
			for (int i = 0; i < drawables.size(); i++) {
				quat orientation = drawables[i].getOrientationQuat();
				//				drawables[i].setOrientation(normalize(angularVelocity*orientation));
				drawables[i].setPosition(drawables[i].getPos() + linearVelocity);
			}

			//angularVelocity = slerp(angularVelocity, quat(), 0.002f);
			linearVelocity *= 0.99f;
		}

		if (updatePositions) {
			for (int i = 0; i < controllerPositions.size(); i++) {
				controllerPositions[i] = controllers[i].getPos();
			}
		}


		//Update model
		for (int i = 0; i < drawables.size(); i++) {
			quat orientation = drawables[i].getOrientationQuat();
			drawables[i].setOrientation(normalize(orientationTransform*orientation));
			drawables[i].setPosition(drawables[i].getPos() + positionTransform);

		}

		glEnable(GL_MULTISAMPLE);
		glClearColor(0.f, 0.f, 0.f, 1.f);

		dragon.setPosition(0.5f*vrCam.leftEye.getPosition()
			+ 0.5f*vrCam.rightEye.getPosition()
			+ vec3(0, 2, 0));

		//Draw left eye
		fbLeftEyeDraw.use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//		bpShader.draw(vrCam.leftEye, lightPos, dragon);
		for (int i = 0; i < controllers.size(); i++)
			bpTexShader.draw(vrCam.leftEye, lightPos, controllers[i]);

		for (int i = 0; i < drawables.size(); i++) {
			if (drawables[i].getMaterial(TextureMat::id) != nullptr) {
				bpTexShader.draw(vrCam.leftEye, lightPos, drawables[i]);
			}
			else
				bpShader.draw(vrCam.leftEye, lightPos, drawables[i]);
		}

		//Draw right eye
		fbRightEyeDraw.use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//		bpShader.draw(vrCam.rightEye, lightPos, dragon);
		for (int i = 0; i < controllers.size(); i++)
			bpTexShader.draw(vrCam.rightEye, lightPos, controllers[i]);
		for (int i = 0; i < drawables.size(); i++) {
			if (drawables[i].getMaterial(TextureMat::id) != nullptr) {
				bpTexShader.draw(vrCam.rightEye, lightPos, drawables[i]);
			}
			else
				bpShader.draw(vrCam.rightEye, lightPos, drawables[i]);
		}

		blit(fbLeftEyeDraw, fbLeftEyeRead);
		blit(fbRightEyeDraw, fbRightEyeRead);

		glDisable(GL_MULTISAMPLE);

		//Draw window
		fbWindow.use();
		leftEyeView.use();
		glClearColor(1.0, 1.0, 1.0, 1.0);
		texShader.draw(cam, leftSquare);

		rightEyeView.use();
		glClearColor(1.0, 1.0, 1.0, 1.0);
		texShader.draw(cam, rightSquare);

		//Draw headset
		if (vrDisplay) {
			vr::Texture_t leftTexture = {
				(void*)(uintptr_t)GLuint(fbLeftEyeRead.getTexture(GL_COLOR_ATTACHMENT0).getID()),
				vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
			vr::Texture_t rightTexture = {
				(void*)(uintptr_t)GLuint(fbRightEyeRead.getTexture(GL_COLOR_ATTACHMENT0).getID()),
				vr::TextureType_OpenGL, vr::ColorSpace_Gamma };

			vr::VRCompositor()->Submit(vr::Eye_Left, &leftTexture);
			vr::VRCompositor()->Submit(vr::Eye_Right, &rightTexture);
		}

		checkGLErrors("Buffer overflow?");

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	vr::VR_Shutdown();
}

#include <assert.h>

void setControllerBindingsOculusTouch(VRControllerInterface *input, VRControllerHand hand) {
	input->assignButton(VRSceneTransform::TRANSFORM_CONTROL, vr::k_EButton_Grip);
}

void setControllerBindingsVive(VRControllerInterface *input, VRControllerHand hand) {
	input->assignButton(VRSceneTransform::TRANSFORM_CONTROL, vr::k_EButton_Grip);
}

void setControllerBindingsWindows(VRControllerInterface *input, VRControllerHand hand) {
	input->assignButton(VRSceneTransform::TRANSFORM_CONTROL, vr::k_EButton_Grip);
}

struct ControllerReferenceFilepaths {
	char* trackpadFrame;
	char* drawPosition;
	char* grabPosition;

	ControllerReferenceFilepaths(VRControllerType type):trackpadFrame(nullptr), drawPosition(nullptr), grabPosition(nullptr) {
		switch (type) {
		case VRControllerType::VIVE:
		case VRControllerType::UNKNOWN:
			trackpadFrame = "models/controllerTrackpadFrame.obj";
			drawPosition = "models/ViveDrawPosition.obj";
			grabPosition = "models/ViveGrabPosition.obj";
			break;
		case VRControllerType::WINDOWS:
			trackpadFrame = "models/WMRTrackpadFrame.obj";
			drawPosition = "models/WMRDrawPosition.obj";
			grabPosition = "models/WMRGrabPosition.obj";
			break;
		case VRControllerType::OCULUS_TOUCH:
			trackpadFrame = "models/OculusTouchTrackpadFrameLeft.obj";
			drawPosition = "models/OculusTouchDrawPosition.obj";
			grabPosition = "models/OculusTouchGrabPosition.obj";
			break;
		}
	}
};

void WindowManager::paintingLoop(const char* loadedFile, const char* savedFile, int sampleNumber) {
	glfwSetCursorPosCallback(window, cursorPositionCallback);
	glfwSetWindowSizeCallback(window, windowResizeCallback);

	const int FRAMES_PER_SECOND = 90;

	vec3 points[6] = {
		//First triangle
		vec3(-0.5f, 0.5f, 0.f)*2.f,
		vec3(0.5f, -0.5f, 0.f)*2.f,
		vec3(0.5f, 0.5f, 0.f)*2.f,
		//Second triangle
		vec3(0.5f, -0.5f, 0.f)*2.f,
		vec3(-0.5f, 0.5f, 0.f)*2.f,
		vec3(-0.5f, -0.5f, 0.f)*2.f
	};

	vec2 coords[6] = {
		//First triangle
		vec2(1, 0.f),
		vec2(0.f, 1.f),
		vec2(0.f, 0.f),
		//Second triangle
		vec2(0.f, 1.f),
		vec2(1.f, 0.f),
		vec2(1.f, 1.f)
	};

	vec3 normals[6] = {
		vec3(0, 0, -1),
		vec3(0, 0, -1),
		vec3(0, 0, -1),
		vec3(0, 0, -1),
		vec3(0, 0, -1),
		vec3(0, 0, -1)
	};

	SimpleTexManager tm;

	if (vrDisplay == nullptr) {
		vr::VR_Shutdown();
		glfwTerminate();
		return;
	}

	Framebuffer fbWindow(window_width, window_height);
	gWindowWidth = window_width;
	gWindowHeight = window_height;
	unsigned int TEX_WIDTH = 800;
	unsigned int TEX_HEIGHT = 800;
	vrDisplay->GetRecommendedRenderTargetSize(&TEX_WIDTH, &TEX_HEIGHT);

	Framebuffer fbLeftEyeDraw = createNewFramebuffer(TEX_WIDTH, TEX_HEIGHT);
	Framebuffer fbRightEyeDraw = createNewFramebuffer(TEX_WIDTH, TEX_HEIGHT);
	Framebuffer fbLeftEyeRead = createNewFramebuffer(TEX_WIDTH, TEX_HEIGHT);
	Framebuffer fbRightEyeRead = createNewFramebuffer(TEX_WIDTH, TEX_HEIGHT);

	int NUM_SAMPLES = sampleNumber;

	if (!fbLeftEyeDraw.addTexture(
		createTexture2DMulti(TEX_WIDTH, TEX_HEIGHT, &tm, NUM_SAMPLES),
		GL_COLOR_ATTACHMENT0) ||
		!fbLeftEyeDraw.addTexture(
			createDepthTextureMulti(TEX_WIDTH, TEX_HEIGHT, &tm, NUM_SAMPLES), GL_DEPTH_ATTACHMENT))
	{
		std::cout << "FBO creation failed" << endl;
	}
	if (!fbRightEyeDraw.addTexture(
		createTexture2DMulti(TEX_WIDTH, TEX_HEIGHT, &tm, NUM_SAMPLES),
		GL_COLOR_ATTACHMENT0) ||
		!fbRightEyeDraw.addTexture(
			createDepthTextureMulti(TEX_WIDTH, TEX_HEIGHT, &tm, NUM_SAMPLES), GL_DEPTH_ATTACHMENT))
	{
		std::cout << "FBO creation failed" << endl;
	}

	if (!fbLeftEyeRead.addTexture(
		createTexture2D(TEX_WIDTH, TEX_HEIGHT, &tm), GL_COLOR_ATTACHMENT0)) {
		std::cout << "FBO creation failed" << endl;
	}
	else if (!fbRightEyeRead.addTexture(
		createTexture2D(TEX_WIDTH, TEX_HEIGHT, &tm), GL_COLOR_ATTACHMENT0)) {
		std::cout << "FBO creation failed" << endl;
	}

	Viewport leftEyeView(window_width, window_height);
	Viewport rightEyeView(window_width / 2, window_height, window_width / 2);

	//Parse tracked devices
	int headsetIndex = 0;
	vector<VRController> controllers(2);
	vr::TrackedDevicePose_t poses[vr::k_unMaxTrackedDeviceCount];

	for (int i = 0; i < vr::k_unMaxTrackedDeviceCount; i++) {
		vr::TrackedDeviceClass deviceClass = vrDisplay->GetTrackedDeviceClass(i);

		switch (deviceClass) {
		case vr::TrackedDeviceClass_HMD:
			headsetIndex = i;
			break;
		case vr::TrackedDeviceClass_Controller:
			VRController controller(i, vrDisplay, poses[i], &tm);
			if (controllers[controller.hand].index != -1) {
				controller.hand = (controller.hand == VRControllerHand::LEFT)
					? VRControllerHand::RIGHT : VRControllerHand::LEFT;
			}
			
			controllers[controller.hand] = controller;
			break;
		}
	}

	VRControllerType controllerType = controllers[0].type;

	if (controllerType == VRControllerType::VIVE) {
		setControllerBindingsVive(&controllers[VRControllerHand::LEFT].input, VRControllerHand::LEFT);
		setControllerBindingsVive(&controllers[VRControllerHand::RIGHT].input, VRControllerHand::RIGHT);
	}else if(controllerType == VRControllerType::OCULUS_TOUCH){
		setControllerBindingsOculusTouch(&controllers[VRControllerHand::LEFT].input, VRControllerHand::LEFT);
		setControllerBindingsOculusTouch(&controllers[VRControllerHand::RIGHT].input, VRControllerHand::RIGHT);
	}
	else if (controllerType == VRControllerType::WINDOWS) {
		setControllerBindingsWindows(&controllers[VRControllerHand::LEFT].input, VRControllerHand::LEFT);
		setControllerBindingsWindows(&controllers[VRControllerHand::RIGHT].input, VRControllerHand::RIGHT);
	}
	else {
		printf("Error: Unknown controller model - Using Vive controls as default\n");
		setControllerBindingsVive(&controllers[VRControllerHand::LEFT].input, VRControllerHand::LEFT);
		setControllerBindingsVive(&controllers[VRControllerHand::RIGHT].input, VRControllerHand::RIGHT);
	}

	bool controllerHasTrackpad = controllerType == VRControllerType::VIVE || controllers[0].type == VRControllerType::WINDOWS || controllerType == VRControllerType::UNKNOWN;

	VRCameraController vrCam(&poses[headsetIndex], vrDisplay);

	//Squares for left and right views
	Drawable leftSquare(
		new SimpleTexGeometry(points, coords, 6, GL_TRIANGLES),
		new TextureMat(fbLeftEyeRead.getTexture(GL_COLOR_ATTACHMENT0)));

	Drawable rightSquare(
		new SimpleTexGeometry(points, coords, 6, GL_TRIANGLES),
		new TextureMat(fbRightEyeRead.getTexture(GL_COLOR_ATTACHMENT0)));

	SimpleTexShader texShader;
	BlinnPhongShader bpTexShader({ { GL_FRAGMENT_SHADER, "#define USING_TEXTURE\n" }
	});
	BlinnPhongShader bpShader;
	BubbleShader bubbleShader;
	SimpleShader wireframeShader;

	/*bpTexShader.deleteProgram();
	bpShader.deleteProgram();

	bpTexShader.createNewProgram(
		{ {GL_VERTEX_SHADER, "shaders/marchingCubeShader.vert"}, {GL_FRAGMENT_SHADER, "shaders/marchingCubeShader.frag"} },
		{ { GL_FRAGMENT_SHADER, "#define USING_TEXTURE\n" } }
	);

	bpShader.createNewProgram(
		{ { GL_VERTEX_SHADER, "shaders/marchingCubeShader.vert" },{ GL_FRAGMENT_SHADER, "shaders/marchingCubeShader.frag" } },
		{}
	);*/

	TrackballCamera savedCam = cam;

	vec3 lightPos(-100.f, 100.f, 100.f);

	fbWindow.use();

	vector<Drawable> drawables;

	//Set up transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	/////////////////////////	
	// STREAM GEOMETRY SETUP
	/////////////////////////
	//Generate color set
	vector<vec3> colorSet;
	colorSet = {
		vec3(1, 1, 1),
		vec3(1, 0, 0),
		vec3(1, 1, 0),
		vec3(0, 1, 0),
		vec3(0, 1, 1),
		vec3(0, 0, 1),
		vec3(1, 0, 1),
		vec3(1, 0.5, 0.25f)
	};

	colorSet = colorMapLoader("default.cmp");

	int COLOR_NUM = colorSet.size();

	VRColorShader colorShader(colorSet.size());

	/*colorShader.createNewProgram(
	{ { GL_VERTEX_SHADER, "shaders/marchingCubeColor.vert" },{ GL_FRAGMENT_SHADER, "shaders/marchingCubeColor.frag" } },
	{ { GL_VERTEX_SHADER, string("#define MAX_COLOR_NUM " + to_string(colorSet.size()) + "\n") } }
	);*/

	enum {
		POSITION=0, NORMAL, COLOR	//Attribute indices
	 };

	//TEST -- Ground plane
	unsigned int groundIndices[6] = { 0, 2, 1, 4, 3, 5 };
	unsigned char groundColors[6] = { 0, 0, 0, 0, 0, 0 };
	auto groundGeom = make_shared<StreamGeometry<vec3, vec3, unsigned char>>(6, vector<char>{ false, false, false });
	groundGeom->loadElementArray(6, GL_STATIC_DRAW, groundIndices);
	groundGeom->loadBuffer<POSITION>(points);
	groundGeom->loadBuffer<NORMAL>(normals);
	groundGeom->loadBuffer<COLOR>(groundColors);
	Drawable groundPlane(
		groundGeom,
		make_shared<ColorSetMat>(colorSet)
	);
	groundPlane.addMaterial(new ShadedMat(0.3f, 0.4f, 0.4f, 50.f));
	groundPlane.orientation = glm::angleAxis(PI / 2.f, vec3(1, 0, 0));
	groundPlane.setScale(vec3(100.f));

	//END TEST -- Ground Plane

	auto streamGeometry = make_shared<StreamGeometry<vec3, vec3, unsigned char>>(minfo.vertices.size(),
	std::vector<char>({ false, false, true }));
	streamGeometry->loadElementArray(minfo.indices.size(), GL_STATIC_DRAW, minfo.indices.data());
	streamGeometry->loadBuffer<POSITION>(minfo.vertices.data());
	streamGeometry->loadBuffer<NORMAL>(minfo.normals.data());
	streamGeometry->loadBuffer<COLOR>(colors.data());

	drawables.push_back(Drawable(streamGeometry, make_shared<ShadedMat>(0.4, 0.7, 0.6, 10.f /*0.4, 0.5, 0.5, 10.f*/)));
	drawables[0].addMaterial(new ColorSetMat(colorSet));
	drawables[0].addMaterial(new ColorMat(vec3(1, 0, 0)));
	

	//Trackpad frame
	ControllerReferenceFilepaths controllerPath(controllerType);
//	const char* filePathTrackpadFrame = (controllerType == VRControllerType::VIVE)
//		? "models/controllerTrackpadFrame.obj" : "models/OculusTouchTrackpadFrameLeft.obj";
	MeshInfoLoader trackpadFrameObj(controllerPath.trackpadFrame);
	vec3 trackpadCenter(trackpadFrameObj.vertices[0]);
	vec3 trackpadBx(trackpadFrameObj.vertices[1] - trackpadCenter);
	vec3 trackpadBy(trackpadFrameObj.vertices[2] - trackpadCenter);
	vec3 trackpadNormal = normalize(cross(trackpadBx, trackpadBy));
	const float DIST_FROM_TPAD = 0.013f;
	const float COLOR_WHEEL_SCALE = 1.5f;

	//Draw position
	MeshInfoLoader drawPositionObj(controllerPath.drawPosition);
	vec3 drawPositionModelspace = drawPositionObj.vertices[0];

	//Grab position
	MeshInfoLoader grabPositionObj(controllerPath.grabPosition);
	vec3 grabPositionModelspace = grabPositionObj.vertices[0];

	//Trackpad geometry
	ColorWheel colorWheel(
		trackpadCenter + trackpadNormal*DIST_FROM_TPAD, 
		trackpadBx*COLOR_WHEEL_SCALE, 
		trackpadBy*COLOR_WHEEL_SCALE, 
		COLOR_NUM, 10);
	colorWheel.addMaterial(new ShadedMat(0.7f, 0.3f, 0.3f, 10.f));
	colorWheel.addMaterial(new ColorSetMat(colorSet));

	const float TRACKPAD_LIGHT_DIST = 0.5f;

	//Undo class
	const int MAX_UNDO = 5;
	UndoStack<unsigned char> undoStack(colors.data(), colors.size(), MAX_UNDO);

	//Drawing sphere
	unsigned char drawColor = 1;
	float sphereTransparency = 1.0f;
	float drawRadius = 0.05f;
	auto sphereGeom = shared_ptr<ElementGeometry>(objToElementGeometry("models/icosphere.obj"));
	auto sphereColorMat = make_shared<ColorMat>(colorSet[drawColor]);
	Drawable drawingSphere[2];
	for (int i = 0; i < 2; i++) {
		drawingSphere[i] = Drawable(sphereGeom, sphereColorMat);
		drawingSphere[i].setScale(vec3(drawRadius));
	}

	//Setup KDTree
	vector<IndexVec3> vertIndexPair;
	for (int i = 0; i < minfo.vertices.size(); i++) {
		vertIndexPair.push_back(IndexVec3(i, minfo.vertices[i]));
	}
	using namespace spatial;
	build_kdTree_inplace<dimensions<IndexVec3>()>(vertIndexPair.begin(), vertIndexPair.end());

	//Load convex hull
	string convexHullName = swapExtension(objName, ".hull");
	MeshInfoLoader convexHullMesh;
	convexHullMesh.loadModelPly(convexHullName.c_str());

/*	//TEST FOR DUPLICATE VERTICES
	const float TEST_RANGE = 0.0001f;
	int totalDuplicates = 0;
	for (int i = 0; i < minfo.vertices.size(); i++) {
		vector<IndexVec3> neighbours;

		kdTree_findNeighbours<dimensions<IndexVec3>()>(
			vertIndexPair.begin(), vertIndexPair.end(),
			IndexVec3(-1, minfo.vertices[i]),
			TEST_RANGE,
			neighbours);

		if (neighbours.size() > 1) {
			printf("Duplicate found %d\n", neighbours.size());
			totalDuplicates++;
		}

	}

	printf("Total duplicates = %d\n", totalDuplicates);

	//TEST FOR DUPLICATE VERTICES
	*/
	//Time tracking
	double frameTime = 0.f;
 	int frameTimeSamples = 0;
	double lastTime = glfwGetTime();

	vector<vec3> controllerPositions(controllers.size());

	VRSceneTransform sceneTransform(&controllers);
	sceneTransform.setPosition(vec3(0.f, 1.f, -1.f));

	//Updating
	int counter = 0;
	float lastAngle_TrackpadRadius = 0.f; 
	bool released_TrackpadRadius = true;
	bool displaySphere[2] = { false, false };
	bool displayColorWheel = false;
	bool paintingButtonPressed[2] = { false, false };

	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (gWindowWidth != window_width || gWindowHeight != window_height) {
			window_width = gWindowWidth;
			window_height = gWindowHeight;
			fbWindow.resize(window_width, window_height);
			leftEyeView.width = window_width;
			leftEyeView.height = window_height;
			rightEyeView.x = leftEyeView.width;
			rightEyeView.width = window_width - leftEyeView.width;
			rightEyeView.height = window_height;
		}

		displaySphere[0]= false;
		displaySphere[1] = false;

		//Get pose
		vr::VRCompositor()->WaitGetPoses(poses, vr::k_unMaxTrackedDeviceCount,
			NULL, 0);

		//Update camera
		vrCam.update();
		vrCam.setProjection(vrDisplay, 0.2f, 400.f);

		vec2 trackpadDir[4] = { vec2(0, 1), vec2(1, 0), vec2(0, -1), vec2(-1, 0) };

		//Update colormap
		static bool updateMapButtonPressed = false;
		if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && !updateMapButtonPressed) {
			updateMapButtonPressed = true;
			colorSet = colorMapLoader("default.cmp");
			dynamic_pointer_cast<ColorSetMat>(drawables[0].getMaterial(ColorSetMat::id))->colors = colorSet;
			dynamic_pointer_cast<ColorSetMat>(colorWheel.getMaterial(ColorSetMat::id))->colors = colorSet;
		}
		else if (glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE)
			updateMapButtonPressed = false;

		//Update controllers
		static vector<bool> modeSwitchButton(controllers.size(), false);
		static int drawMode = 0;
		bool buttonChanged = false;
		for (int i = 0; i < controllers.size(); i++) {
			vr::VRControllerState_t state;
			vr::TrackedDevicePose_t pose;
			if (!vrDisplay->GetControllerStateWithPose(
				vr::VRCompositor()->GetTrackingSpace(),
				controllers[i].index,
				&state,
				sizeof(vr::VRControllerState_t),
				&pose))
			{
				printf("Error reading controller state\n");
			}

			controllers[i].updatePose(poses[controllers[i].index]);
			controllers[i].updateState(state);
			controllers[i].input.updateState(state);
		}

		//Get time
		static double lastTime = 0.f;
		double currentTime = glfwGetTime();
		sceneTransform.updateTransform(currentTime - lastTime, grabPositionModelspace);	//Not using time yet
		lastTime = currentTime;

		const float MIN_TILT = (controllerHasTrackpad) ? 0.3f : 0.1f;	//Minimum offset from center for trackpads and joysticks

		//Change color based on axis
		if (controllers[VRControllerHand::LEFT].input.getActivation(COLOR_DISPLAY_CONTROL) ||
			(!controllerHasTrackpad && 
			length(controllers[VRControllerHand::LEFT].input.getAxis(COLOR_SELECT_CONTROL)) > MIN_TILT))
		{
			vec2 axis = controllers[VRControllerHand::LEFT].input.getAxis(COLOR_SELECT_CONTROL);
			displayColorWheel = true;
			colorWheel.thumbPos(axis);
			if (controllerHasTrackpad || length(axis) > MIN_TILT) {
				//			vec2 axis = controllers[0].axes[VRController::TRACKPAD_AXIS];
				drawColor = axisToIndex(axis, COLOR_NUM);
				sphereColorMat->color = vec4(colorSet[drawColor], sphereTransparency);
				colorWheel.selectColor(drawColor);
			}
		}
		else {
			colorWheel.selectColor(COLOR_NUM);		//Unset value
			displayColorWheel = false;
			colorWheel.thumbPos(vec2(20.f, 20.f));
		}
		//Change radius based on axis
		const float SCALE_PER_ROTATION = 2.f;
		const float MIN_DRAW_RADIUS = 0.01f;
		const float MAX_DRAW_RADIUS = 0.2f;


//		bool joystickCondition = (controllers[VRControllerHand::RIGHT].type == VRControllerType::VIVE)
//			? false : controllers[VRControllerHand::RIGHT].input.getAxis(SPHERE_SIZE_CONTROL) > MIN_TILT;


		if ((controllers[VRControllerHand::RIGHT].input.getActivation(SPHERE_SIZE_TOUCH_CONTROL)
			+ (length(controllers[VRControllerHand::RIGHT].input.getAxis(SPHERE_SIZE_CONTROL)) > MIN_TILT)
			+ (!controllerHasTrackpad)) >= 2){
			//vec2 axis = controllers[1].axes[VRController::TRACKPAD_AXIS];
			vec2 axis = controllers[VRControllerHand::RIGHT].input.getAxis(SPHERE_SIZE_CONTROL);
			float currentAngle = atan2(axis.y, axis.x);

			if (controllerHasTrackpad) {
				if (released_TrackpadRadius) {
					lastAngle_TrackpadRadius = currentAngle;
					released_TrackpadRadius = false;
				}
				else {
					//Find shortest angular distance from last to current position
					float savedCurrentAngle = currentAngle;
					float diffA = currentAngle - lastAngle_TrackpadRadius;
					if (currentAngle < lastAngle_TrackpadRadius) currentAngle += 2.f*PI;
					else lastAngle_TrackpadRadius += 2.f*PI;
					float diffB = currentAngle - lastAngle_TrackpadRadius;
					float diff = (abs(diffA) < abs(diffB)) ? diffA : diffB;
				
					drawRadius = clamp(drawRadius*pow(SCALE_PER_ROTATION, -diff / (2.f*PI)), MIN_DRAW_RADIUS, MAX_DRAW_RADIUS);
					drawingSphere[0].setScale(vec3(drawRadius));
					drawingSphere[1].setScale(vec3(drawRadius));

					lastAngle_TrackpadRadius = savedCurrentAngle;
				}
			}
			else {
				float scaleChange = pow(2.f, axis.y/float(FRAMES_PER_SECOND/2));	//Sphere size doubles in half a second

				drawRadius = clamp(drawRadius*scaleChange, MIN_DRAW_RADIUS, MAX_DRAW_RADIUS);
				drawingSphere[0].setScale(vec3(drawRadius));
				drawingSphere[1].setScale(vec3(drawRadius));
			}
			displaySphere[1] = true;
		}
		else {
			released_TrackpadRadius = true;
		}

		//Update model
		for (int i = 0; i < drawables.size(); i++) {
			drawables[i].setOrientation(sceneTransform.getOrientationQuat());
			drawables[i].setPosition(sceneTransform.getPos());
			drawables[i].setScale(vec3(sceneTransform.scale));
		}

		//SEARCH KDTREE
		vector<IndexVec3> neighbours;
		for (int i = 0; i < controllers.size(); i++) {
			if (controllers[i].input.getActivation(SPHERE_DISPLAY_CONTROL)){
				vec3 pos = vec3(controllers[i].getTransform()*vec4(drawPositionModelspace, 1.f)); // TODO: write better code
				mat4 invrsTrans = inverse(sceneTransform.getTransform());
				pos = vec3(invrsTrans*vec4(pos, 1));
				float searchRadius = drawRadius / sceneTransform.scale;

				if(controllers[i].input.getScalar(PAINT_CONTROL) > 0.95f){
					if (paintingButtonPressed[i] == false) {
						paintingButtonPressed[i] = true;
						undoStack.startNewState();
					}
					kdTree_findNeighbours<dimensions<IndexVec3>()>(
						vertIndexPair.begin(), vertIndexPair.end(),
						IndexVec3(-1, pos),
						searchRadius*searchRadius,
						neighbours);
				}
				else {
					paintingButtonPressed[i] = false;
				}

				displaySphere[i] = true;		//TODO get rid of?
			}
			else {
				paintingButtonPressed == false;
			}
		}
		for (int i = 0; i < neighbours.size(); i++) {
			streamGeometry->modify<COLOR>(neighbours[i].index, drawColor);
			undoStack.modify(neighbours[i].index, drawColor);
		}

		streamGeometry->dump<COLOR>();
		streamGeometry->buffManager.endWrite();

		//Update color wheel position
		colorWheel.position = controllers[0].position;
		colorWheel.orientation = controllers[0].orientation;

		//Update sphere positions
		drawingSphere[0].position = vec3(controllers[0].getTransform()*vec4(drawPositionModelspace, 1.f));	//controllers[0].position;
		drawingSphere[1].position = vec3(controllers[1].getTransform()*vec4(drawPositionModelspace, 1.f));	//controllers[1].position;

		//Update bounding sphere on model and find fog bounds
		/*vec3 bSphereCenter = vec3(sceneTransform.getTransform()*vec4(boundingSphere.pos, 1));
		float bSphereRadius = sceneTransform.scale*boundingSphere.radius;
		float fogDistance = std::max(length(0.5f*vrCam.leftEye.getPosition() + 0.5f*vrCam.rightEye.getPosition() 
		- bSphereCenter) - bSphereRadius, 0.f);
		float fogScale = bSphereRadius*0.5f;*/
		//Update fog distance and scale
		float closestPoint = std::numeric_limits<float>::max();
		float furthestPoint = -std::numeric_limits<float>::max();
		mat4 invModelMatrix = inverse(sceneTransform.getTransform());
		vec3 cameraPosition = vec3(invModelMatrix*vec4(vrCam.leftEye.getPosition(), 1));
		/*vec3 cameraDirection = normalize(vec3(invModelMatrix*vec4(vrCam.leftEye.getDirection(), 0)));
		for (vec3 vert : convexHullMesh.vertices) {
			float vertDist = dot(cameraDirection, vert - cameraPosition);
			closestPoint = std::min(closestPoint, vertDist);
			furthestPoint = std::max(furthestPoint, vertDist);
		}
		float fogDistance = std::max(closestPoint*sceneTransform.scale, 0.f);
		float fogScale = (furthestPoint - closestPoint)*sceneTransform.scale;
		*/
		std::pair<float, float> distPair = getClosestAndFurthestDistanceToConvexHull(
			cameraPosition, 
			convexHullMesh.vertices.data(), 
			convexHullMesh.vertices.size(), 
			convexHullMesh.indices.data(), 
			convexHullMesh.indices.size() / 3);

		float fogDistance = distPair.first*sceneTransform.scale;
		float fogScale = (distPair.second - distPair.first)*0.5f*sceneTransform.scale;

		//Setup screenshot
		//Projection matrix for screenshots
		//float aspectRatio = float(fbRightEyeRead.vp.width) / float(fbRightEyeRead.vp.height);
		float aspectRatio = float(SCREENSHOT_WIDTH) / float(SCREENSHOT_HEIGHT);
		static mat4 screenshotProjection = perspective(radians(80.f), 1.f, 0.01f, 10.f);
		static mat4 savedProjection;
		static bool screenshotPressed = false;
		if (controllers[VRControllerHand::LEFT].input.getActivation(SCREENSHOT_CONTROL) && !screenshotPressed) {
			screenshotPressed = true;
			savedProjection = vrCam.rightEye.getProjectionMatrix();
			vrCam.rightEye.setProjectionMatrix(screenshotProjection);
		}
		else if(controllers[VRControllerHand::LEFT].input.getActivation(SCREENSHOT_CONTROL) && !screenshotPressed){
			vrCam.rightEye.setProjectionMatrix(savedProjection);
		}
		else {
			screenshotPressed = false;
		}

		//Load and save views
		static bool saveViewPressed = false;
		if (controllers[VRControllerHand::RIGHT].input.getActivation(SAVE_VIEW_CONTROL) && !saveViewPressed) {
			saveViewPressed = true;
			string filename = findFilenameVariation(string(loadedFile)+".view");
			vec3 cameraDir = vec3(vrCam.rightEye.getCameraMatrix()*vec4(0, 0, -1, 0));
			VRView view;
			view.generateView(vrCam.rightEye.getPosition(), cameraDir, &sceneTransform);
			view.scale = sceneTransform.scale;	//Should be part of function
			saveVRViewToFile(filename.c_str(), &view);

		}
		else if(!controllers[VRControllerHand::RIGHT].input.getActivation(SAVE_VIEW_CONTROL) && saveViewPressed)
			saveViewPressed = false;

		static bool loadViewPressed = false;
		int keyPressed = getNumberKeyPressed(window);
		if (keyPressed && !loadViewPressed) {
			loadViewPressed = true;
			vec3 cameraDir = vec3(vrCam.rightEye.getCameraMatrix()*vec4(0, 0, -1, 0));
			VRView view;
			if (loadVRViewFromFile((string(loadedFile) + to_string(keyPressed) + string(".view")).c_str(), &view)) {
				view.getViewFromCameraPositionAndOrientation(vrCam.rightEye.getPosition(), cameraDir, &sceneTransform);		//&drawables[0]);
				sceneTransform.scale = view.scale;		//Should be part of function
				sceneTransform.velocity = vec3(0);
				sceneTransform.angularVelocity = quat();
			}
		}
		else if (!keyPressed && loadViewPressed)
			loadViewPressed = false;

		////////////
		// DRAWING
		///////////
		glLineWidth(10.f);
		glEnable(GL_MULTISAMPLE);
		glClearColor(0.f, 0.f, 0.f, 0.f);

		//Draw left eye
		fbLeftEyeDraw.use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//		colorShader.draw(vrCam.leftEye, lightPos, groundPlane);
//		glClear(GL_DEPTH_BUFFER_BIT);
		for (int i = 0; i < controllers.size(); i++)
			bpTexShader.draw(vrCam.leftEye, lightPos, controllers[i]);
		for (int i = 0; i < drawables.size(); i++) {
			colorShader.draw(vrCam.leftEye, lightPos, 
				fogScale, fogDistance, 
				vec3(0.02f, 0.04f, 0.07f), drawables[i]);		//Add lightPos and colorMat checking
		}
		if (displayColorWheel) {
			colorShader.draw(
				vrCam.leftEye,
				colorWheel.trackpadLightPosition(TRACKPAD_LIGHT_DIST),
				fogScale, 10.f,
				vec3(0.02f, 0.04f, 0.07f),
				colorWheel);
		}
		/*
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		for (auto& drawable : drawables) {
			wireframeShader.draw(vrCam.leftEye, drawable);
		}
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		*/
		for (int i = 0; i < 2; i++) {
			if (displaySphere[i]) {
				glCullFace(GL_FRONT);
				bubbleShader.draw(vrCam.leftEye, drawingSphere[i]);
				glCullFace(GL_BACK);
				bubbleShader.draw(vrCam.leftEye, drawingSphere[i]);
			}
		}


		//Draw right eye -- Or screenshot
		if (!controllers[VRControllerHand::LEFT].input.getActivation(SCREENSHOT_CONTROL) || !screenshotPressed)
			fbRightEyeDraw.use();
		else
			fbScreenshotDraw.use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//		colorShader.draw(vrCam.rightEye, lightPos, groundPlane);
//		glClear(GL_DEPTH_BUFFER_BIT);
		for (int i = 0; i < controllers.size(); i++)
			bpTexShader.draw(vrCam.rightEye, lightPos, controllers[i]);
		for (int i = 0; i < drawables.size(); i++) {
			colorShader.draw(vrCam.rightEye, lightPos, 
				fogScale, fogDistance, vec3(0.02f, 0.04f, 0.07f), drawables[i]);
		}
		if (displayColorWheel) {
			colorShader.draw(
				vrCam.rightEye,
				colorWheel.trackpadLightPosition(TRACKPAD_LIGHT_DIST), 
				fogScale, 10.f,
				vec3(0.02f, 0.04f, 0.07f),
				colorWheel);
		}
		/*
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		for (auto& drawable : drawables) {
			wireframeShader.draw(vrCam.rightEye, drawable);
		}

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		*/
		for (int i = 0; i < 2; i++) {
			if (displaySphere[i]) {
				glCullFace(GL_FRONT);
				bubbleShader.draw(vrCam.rightEye, drawingSphere[i]);
				glCullFace(GL_BACK);
				bubbleShader.draw(vrCam.rightEye, drawingSphere[i]);
			}
		}


		glDisable(GL_BLEND);

		//Transfer data to regular textures
		blit(fbLeftEyeDraw, fbLeftEyeRead);
		blit(fbRightEyeDraw, fbRightEyeRead);

		if (controllers[VRControllerHand::LEFT].input.getActivation(SCREENSHOT_CONTROL) && screenshotPressed)
			blit(fbScreenshotDraw, fbScreenshotRead);

		glDisable(GL_MULTISAMPLE);

		//Draw window
		fbWindow.use();
		leftEyeView.use();
		glClearColor(1.0, 1.0, 1.0, 0.0);
		texShader.draw(cam, leftSquare);

		//rightEyeView.use();
		//glClearColor(1.0, 1.0, 1.0, 1.0);
		//texShader.draw(cam, rightSquare);

		//Draw headset
		if (vrDisplay) {
			vr::Texture_t leftTexture = {
				(void*)(uintptr_t)GLuint(fbLeftEyeRead.getTexture(GL_COLOR_ATTACHMENT0).getID()),
				vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
			vr::Texture_t rightTexture = {
				(void*)(uintptr_t)GLuint(fbRightEyeRead.getTexture(GL_COLOR_ATTACHMENT0).getID()),
				vr::TextureType_OpenGL, vr::ColorSpace_Gamma };

			vr::VRCompositor()->Submit(vr::Eye_Left, &leftTexture);
			vr::VRCompositor()->Submit(vr::Eye_Right, &rightTexture);

		}

		glEnable(GL_BLEND);

		checkGLErrors("Buffer overflow?");

		//Write screenshot to file
		if (screenshotPressed && controllers[VRControllerHand::LEFT].input.getActivation(SCREENSHOT_CONTROL)) {
			string filename = findFilenameVariation("Screenshot.png");
			//fbRightEyeRead.use();

			Texture rightTex = fbScreenshotRead.getTexture(GL_COLOR_ATTACHMENT0);

			size_t imageSize = rightTex.getWidth()*rightTex.getHeight() * 4;
			unsigned char *data = new unsigned char[imageSize];
			glGetTextureImage(rightTex.getID(), 0, GL_RGBA, GL_UNSIGNED_BYTE, imageSize, data);
			stbi_flip_vertically_on_write(true);
			stbi_write_png(filename.c_str(), rightTex.getWidth(), rightTex.getHeight(), 4, data, 0);

			delete[] data;

			fbWindow.use();
		}

		if (frameTimeSamples > 30) {
			double currentTime = glfwGetTime();
			frameTime = currentTime - lastTime;
//			cout << "Time per frame = " << frameTime/double(frameTimeSamples) << endl;
			frameTimeSamples = 0;
			lastTime = currentTime;
		}
		else {
			frameTimeSamples++;
		}

		static bool saveButtonPressed = false;
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !saveButtonPressed) {
			if (saveVolume(savedFilename.c_str(), objName.c_str(), streamGeometry->vboPointer<COLOR>(), colors.size()))
				printf("Saved %s successfully\n", savedFilename.c_str());
			else {
				printf("Attempting fallback - Saving to fallback.clr...\n");
				if (saveVolume("fallback.clr", objName.c_str(), streamGeometry->vboPointer<COLOR>(), colors.size()))
					printf("Saved fallback.clr successfully\n");
			}
			saveButtonPressed = true;
		}
		else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
			saveButtonPressed = false;
		}
		static bool undoButtonPressed = false;
//		bool pressed = controllers[0].buttons[VRController::MENU_BUTTON];
		bool pressed = controllers[0].input.getActivation(UNDO_CONTROL);
		if (pressed == false && undoButtonPressed) {
			map<size_t, unsigned char> changes;
			undoStack.undo(&changes);
			for (auto &it : changes) {
				streamGeometry->modify<COLOR>(it.first, it.second);
			}
			streamGeometry->dump<COLOR>();
			streamGeometry->buffManager.endWrite();
			undoButtonPressed = false;
		} else if (pressed) {
			undoButtonPressed = true;
		}
		static bool redoButtonPressed = false;
//		pressed = controllers[1].buttons[VRController::MENU_BUTTON];
		pressed = controllers[1].input.getActivation(REDO_CONTROL);
		if (pressed == false && redoButtonPressed) {
			map<size_t, unsigned char> changes;
			undoStack.redo(&changes);
			for (auto &it : changes) {
				streamGeometry->modify<COLOR>(it.first, it.second);
			}
			streamGeometry->dump<COLOR>();
			streamGeometry->buffManager.endWrite();
			redoButtonPressed = false;
		}
		else if (pressed) {
			redoButtonPressed = true;
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	vr::VR_Shutdown();
}


//Temporary testing
void WindowManager::mainLoop() {

	//Test quaterions
	/*	quat rot = angleAxis(3.14159f, vec3(0, 1, 0));
	vec4 point(1, 0, 0, 1);
	vec4 res1 = rot*point*rot.
	*/
	glfwSetCursorPosCallback(window, cursorPositionCallback);

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

	/*	vec2 coords[6] = {
	//First triangle
	vec2(1, 0.f),
	vec2(0.f, 0.f),
	vec2(0.f, 1.f),
	//Second triangle
	vec2(0.f, 1.f),
	vec2(1.f, 1.f),
	vec2(1.f, 0.f)
	};
	*/
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

	vec2 coords2[6] = {
		//First triangle
		vec2(1, 0.f),
		vec2(0.f, 0.f),
		vec2(0.f, 1.f),
		//Second triangle
		vec2(0.f, 1.f),
		vec2(1.f, 1.f),
		vec2(1.f, 0.f)
	};

	SimpleTexManager tm;

	if (vrDisplay == nullptr) {
		vr::VR_Shutdown();
		glfwTerminate();
		return;
	}

	Framebuffer fbWindow(window_width, window_height);
	unsigned int TEX_WIDTH = 800;
	unsigned int TEX_HEIGHT = 800;
	vrDisplay->GetRecommendedRenderTargetSize(&TEX_WIDTH, &TEX_HEIGHT);
	//	glfwSetWindowSize(window, TEX_WIDTH * 2, TEX_HEIGHT);

	glfwSetKeyCallback(window, keyCallback);

	Framebuffer fbLeftEyeDraw = createNewFramebuffer(TEX_WIDTH, TEX_HEIGHT);
	Framebuffer fbRightEyeDraw = createNewFramebuffer(TEX_WIDTH, TEX_HEIGHT);
	Framebuffer fbLeftEyeRead = createNewFramebuffer(TEX_WIDTH, TEX_HEIGHT);
	Framebuffer fbRightEyeRead = createNewFramebuffer(TEX_WIDTH, TEX_HEIGHT);


	const int NUM_SAMPLES = 1;
	/*
	if (!fbLeftEyeDraw.addTexture(
	createTexture2DMulti(TEX_WIDTH, TEX_HEIGHT, &tm, NUM_SAMPLES),
	GL_COLOR_ATTACHMENT0) ||
	!fbLeftEyeDraw.addTexture(
	createDepthTextureMulti(TEX_WIDTH, TEX_HEIGHT, &tm, NUM_SAMPLES), GL_DEPTH_ATTACHMENT))
	{
	std::cout << "FBO creation failed" << endl;
	}
	if (!fbRightEyeDraw.addTexture(
	createTexture2DMulti(TEX_WIDTH, TEX_HEIGHT, &tm, NUM_SAMPLES),
	GL_COLOR_ATTACHMENT0) ||
	!fbRightEyeDraw.addTexture(
	createDepthTextureMulti(TEX_WIDTH, TEX_HEIGHT, &tm, NUM_SAMPLES), GL_DEPTH_ATTACHMENT))
	{
	std::cout << "FBO creation failed" << endl;
	}
	*/

	//TEST vv

	if (!fbLeftEyeDraw.addTexture(
		createTexture2DMulti(
			TexInfo(GL_TEXTURE_2D_MULTISAMPLE, { int(TEX_WIDTH), int(TEX_HEIGHT) }, 0, GL_RGB, GL_RGB16F, GL_FLOAT), &tm, NUM_SAMPLES),
		GL_COLOR_ATTACHMENT0) ||
		!fbLeftEyeDraw.addTexture(
			createDepthTextureMulti(TEX_WIDTH, TEX_HEIGHT, &tm, NUM_SAMPLES),
			GL_DEPTH_ATTACHMENT) ||
		!fbLeftEyeDraw.addTexture(
			createTexture2DMulti(
				TexInfo(GL_TEXTURE_2D_MULTISAMPLE, { int(TEX_WIDTH), int(TEX_HEIGHT) }, 0, GL_RGB, GL_RGB16F, GL_FLOAT), &tm, NUM_SAMPLES),
			GL_COLOR_ATTACHMENT1))
	{
		std::cout << "FBO creation failed" << endl;
	}

	if (!fbRightEyeDraw.addTexture(
		createTexture2DMulti(
			TexInfo(GL_TEXTURE_2D_MULTISAMPLE, { int(TEX_WIDTH), int(TEX_HEIGHT) }, 0, GL_RGB, GL_RGB16F, GL_FLOAT), &tm, NUM_SAMPLES),
		GL_COLOR_ATTACHMENT0) ||
		!fbRightEyeDraw.addTexture(
			createDepthTextureMulti(TEX_WIDTH, TEX_HEIGHT, &tm, NUM_SAMPLES),
			GL_DEPTH_ATTACHMENT) ||
		!fbRightEyeDraw.addTexture(
			createTexture2DMulti(
				TexInfo(GL_TEXTURE_2D_MULTISAMPLE, { int(TEX_WIDTH), int(TEX_HEIGHT) }, 0, GL_RGB, GL_RGB16F, GL_FLOAT), &tm, NUM_SAMPLES),
			GL_COLOR_ATTACHMENT1))
	{
		std::cout << "FBO creation failed" << endl;
	}

	Framebuffer aoRenderLeft = createNewFramebuffer(TEX_WIDTH, TEX_HEIGHT);
	Framebuffer aoRenderRight = createNewFramebuffer(TEX_WIDTH, TEX_HEIGHT);
	aoRenderLeft.addTexture(createTexture2DMulti(
		TEX_WIDTH, TEX_HEIGHT, &tm, NUM_SAMPLES),
		GL_COLOR_ATTACHMENT0);
	aoRenderRight.addTexture(createTexture2DMulti(
		TEX_WIDTH, TEX_HEIGHT, &tm, NUM_SAMPLES),
		GL_COLOR_ATTACHMENT0);


	//TEST ^^



	if (!fbLeftEyeRead.addTexture(
		createTexture2D(TEX_WIDTH, TEX_HEIGHT, &tm), GL_COLOR_ATTACHMENT0)) {
		std::cout << "FBO creation failed" << endl;
	}
	else if (!fbRightEyeRead.addTexture(
		createTexture2D(TEX_WIDTH, TEX_HEIGHT, &tm), GL_COLOR_ATTACHMENT0)) {
		std::cout << "FBO creation failed" << endl;
	}

	Viewport leftEyeView(window_width / 2, window_height);
	Viewport rightEyeView(window_width / 2, window_height, window_width / 2);

	//Parse tracked devices
	int headsetIndex = 0;
	vector<VRController> controllers;
	vr::TrackedDevicePose_t poses[vr::k_unMaxTrackedDeviceCount];

	for (int i = 0; i < vr::k_unMaxTrackedDeviceCount; i++) {
		vr::TrackedDeviceClass deviceClass = vrDisplay->GetTrackedDeviceClass(i);

		switch (deviceClass) {
		case vr::TrackedDeviceClass_HMD:
			headsetIndex = i;
			break;
		case vr::TrackedDeviceClass_Controller:
			controllers.push_back(VRController(i, vrDisplay, poses[i], &tm));
			break;
		}
	}

	VRCameraController vrCam(&poses[headsetIndex], vrDisplay);

	//Dragon
	auto dragonGeom = shared_ptr<ElementGeometry>(objToElementGeometry("models/dragon.obj"));
	Drawable dragon(
		dragonGeom,
		make_shared<ColorMat>(vec3(0.75f, 0.1f, 0.3f)));
	dragon.addMaterial(new ShadedMat(0.2f, 0.5f, 0.3f, 10.f));

	dragon.setPosition(vec3(1.f, 0, 0));
	dragon.setOrientation(angleAxis(-PI*0.5f, vec3(0.f, 1.f, 0.f)));

	auto sphereGeom = shared_ptr<ElementGeometry>(objToElementGeometry("models/icosphere.obj"));
	Drawable sphere(
		sphereGeom,
		make_shared<ColorMat>(vec3(0.1, 0.3f, 0.8f)));
	sphere.addMaterial(new ShadedMat(0.2f, 0.5f, 0.3f, 10.f));

	sphere.setPosition(vec3(1.f, 0, 0));

	//Squares for left and right views
	/*	Drawable leftSquare(
	new TextureMat(fbLeftEyeRead.getTexture(GL_COLOR_ATTACHMENT0)),
	new SimpleTexGeometry(points, coords, 6, GL_TRIANGLES));

	Drawable rightSquare(
	new TextureMat(fbRightEyeRead.getTexture(GL_COLOR_ATTACHMENT0)),
	new SimpleTexGeometry(points, coords, 6, GL_TRIANGLES));
	*/
	//TEST vv
	Drawable leftSquare(
		new SimpleTexGeometry(points, coords, 6, GL_TRIANGLES),
		new TextureMat(fbLeftEyeDraw.getTexture(GL_COLOR_ATTACHMENT0), TextureMat::POSITION));

	Drawable rightSquare(
		new SimpleTexGeometry(points, coords, 6, GL_TRIANGLES),
		new TextureMat(fbRightEyeDraw.getTexture(GL_COLOR_ATTACHMENT0), TextureMat::POSITION));

	leftSquare.addMaterial(new TextureMat(fbLeftEyeDraw.getTexture(GL_COLOR_ATTACHMENT1), TextureMat::NORMAL));
	rightSquare.addMaterial(new TextureMat(fbRightEyeDraw.getTexture(GL_COLOR_ATTACHMENT1), TextureMat::NORMAL));

	AOShader aoShader;
	PosNormalShader pnShader;

	Drawable leftSquareTest(
		new SimpleTexGeometry(points, coords2, 6, GL_TRIANGLES),
		new TextureMat(fbLeftEyeRead.getTexture(GL_COLOR_ATTACHMENT0)));

	Drawable rightSquareTest(
		new SimpleTexGeometry(points, coords2, 6, GL_TRIANGLES),
		new TextureMat(fbRightEyeRead.getTexture(GL_COLOR_ATTACHMENT0)));

	//TEST ^^

	SimpleTexShader texShader;
	SimpleShader shader;
	BlinnPhongShader bpShader;
	BlinnPhongShader bpTexShader({ { GL_FRAGMENT_SHADER, "#define USING_TEXTURE\n" }
	});

	TrackballCamera savedCam = cam;

	vec3 lightPos(-100.f, 100.f, 100.f);

	fbLeftEyeDraw.use();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	bpShader.draw(cam, lightPos, dragon);

	fbWindow.use();

	vector<Drawable> drawables;
	//	loadWavefront("untrackedmodels/OrganodronCity/", "OrganodronCity", &drawables, &tm);
	//	loadWavefront("untrackedmodels/SciFiCenter/CenterCity/", "scificity", &drawables, &tm);
	//	loadWavefront("untrackedmodels/lstudio/", "lsystem.obj", &drawables, &tm);
	//	loadWavefront("untrackedmodels/", "riccoSurface_take2", &drawables, &tm);

	auto objGeometry = shared_ptr<ElementGeometry>(objToElementGeometry("untrackedmodels/riccoSurface_take3.obj"));
	drawables.push_back(Drawable(objGeometry, make_shared<ShadedMat>(0.3, 0.4, 0.4, 10.f)));
	drawables[0].addMaterial(new ColorMat(vec3(1, 1, 1)));


//	drawables.push_back(dragon);

	for (int i = 0; i < drawables.size(); i++) {
		drawables[i].setPosition(vec3(0, 0, -1.f));
		drawables[i].setScale(vec3(10.0));
	}

	vector<vec3> controllerPositions(controllers.size());

	quat perFrameRot = angleAxis(3.14159f / 90.f, vec3(0, 1, 0));

	//Velocity
	vec3 linearVelocity(0.f);
	quat angularVelocity = quat();

	while (!glfwWindowShouldClose(window)) {
		if (reloadShaders) {
			aoShader.createProgram();
			reloadShaders = false;
		}


		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Get pose
		vr::VRCompositor()->WaitGetPoses(poses, vr::k_unMaxTrackedDeviceCount,
			NULL, 0);

		//Update camera
		vrCam.update();
		vrCam.setProjection(vrDisplay, 0.2f, 400.f);

		//Update controllers
		vector<int> triggersPressed;
		for (int i = 0; i < controllers.size(); i++) {
			vr::VRControllerState_t state;
			vr::TrackedDevicePose_t pose;
			if (!vrDisplay->GetControllerStateWithPose(
				vr::VRCompositor()->GetTrackingSpace(),
				controllers[i].index,
				&state,
				sizeof(vr::VRControllerState_t),
				&pose))
			{
				printf("Error reading controller state\n");
			}

			controllers[i].updatePose(poses[controllers[i].index]);
			controllers[i].updateState(state);
			if (controllers[i].axes[VRController::TRIGGER_AXIS].x > 0.5f)
				triggersPressed.push_back(i);
		}

		vec3 positionTransform(0.f);
		quat orientationTransform;

		bool updatePositions = true;

		switch (triggersPressed.size()) {
		case 1:
		{
			vec3 lastPos = controllerPositions[triggersPressed[0]];
			positionTransform = controllers[triggersPressed[0]].getPos() - lastPos;
			linearVelocity = positionTransform;
			break;
		}
		case 2:
		{
			vec3 axisA = normalize(controllerPositions[triggersPressed[0]]
				- controllerPositions[triggersPressed[1]]);
			vec3 axisB = normalize(controllers[triggersPressed[0]].getPos()
				- controllers[triggersPressed[1]].getPos());
			vec3 rotAxis = cross(axisA, axisB);
			if (length(rotAxis) > 0.0001) {
				float angle = asin(length(rotAxis));
				orientationTransform = angleAxis(angle, normalize(rotAxis));
				angularVelocity = orientationTransform;
			}
			else
				updatePositions = false;
			break;

		}
		default:
			for (int i = 0; i < drawables.size(); i++) {
				quat orientation = drawables[i].getOrientationQuat();
				//				drawables[i].setOrientation(normalize(angularVelocity*orientation));
				drawables[i].setPosition(drawables[i].getPos() + linearVelocity);
			}

			//angularVelocity = slerp(angularVelocity, quat(), 0.002f);
			linearVelocity *= 0.97f;
		}

		if (updatePositions) {
			for (int i = 0; i < controllerPositions.size(); i++) {
				controllerPositions[i] = controllers[i].getPos();
			}
		}


		//Update model
		for (int i = 0; i < drawables.size(); i++) {
			quat orientation = drawables[i].getOrientationQuat();
			drawables[i].setOrientation(normalize(orientationTransform*orientation));
			drawables[i].setPosition(drawables[i].getPos() + positionTransform);

		}

		glEnable(GL_MULTISAMPLE);		//TEST
		glClearColor(0.f, 0.f, 0.f, 1.f);

		dragon.setPosition(0.5f*vrCam.leftEye.getPosition()
			+ 0.5f*vrCam.rightEye.getPosition()
			+ vec3(0, 2, 0));

		/*		//Draw left eye
		fbLeftEyeDraw.use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		for (int i = 0; i < controllers.size(); i++)
		bpTexShader.draw(vrCam.leftEye, lightPos, controllers[i]);

		for (int i = 0; i < drawables.size(); i++) {
		if (drawables[i].getMaterial(TextureMat::id) != nullptr) {
		bpTexShader.draw(vrCam.leftEye, lightPos, drawables[i]);
		}
		else
		bpShader.draw(vrCam.leftEye, lightPos, drawables[i]);
		}

		//Draw right eye
		fbRightEyeDraw.use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		for (int i = 0; i < controllers.size(); i++)
		bpTexShader.draw(vrCam.rightEye, lightPos, controllers[i]);
		for (int i = 0; i < drawables.size(); i++) {
		if (drawables[i].getMaterial(TextureMat::id) != nullptr) {
		bpTexShader.draw(vrCam.rightEye, lightPos, drawables[i]);
		}
		else
		bpShader.draw(vrCam.rightEye, lightPos, drawables[i]);
		}

		blit(fbLeftEyeDraw, fbLeftEyeRead);
		blit(fbRightEyeDraw, fbRightEyeRead);
		*/

		//TEST vv AMBIENT OCCLUSION
		//Draw left eye
		fbLeftEyeDraw.use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		for (int i = 0; i < controllers.size(); i++)
			pnShader.draw(vrCam.leftEye, lightPos, controllers[i]);

		for (int i = 0; i < drawables.size(); i++) {
			pnShader.draw(vrCam.leftEye, lightPos, drawables[i]);
		}

		//Ambient occlusion
		aoRenderLeft.use();
		aoShader.draw(cam, lightPos, leftSquare);

		//Draw right eye
		fbRightEyeDraw.use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		for (int i = 0; i < controllers.size(); i++)
			pnShader.draw(vrCam.rightEye, lightPos, controllers[i]);
		for (int i = 0; i < drawables.size(); i++) {
			pnShader.draw(vrCam.rightEye, lightPos, drawables[i]);
		}

		//Ambient occlusion
		aoRenderRight.use();
		aoShader.draw(cam, lightPos, rightSquare);

		//TEST ^^ 

		glDisable(GL_MULTISAMPLE);

		blit(aoRenderLeft, fbLeftEyeRead);
		blit(aoRenderRight, fbRightEyeRead);

		//Draw window
		fbWindow.use();
		leftEyeView.use();
		glClearColor(1.0, 1.0, 1.0, 1.0);
		texShader.draw(cam, leftSquareTest);

		rightEyeView.use();
		glClearColor(1.0, 1.0, 1.0, 1.0);
		texShader.draw(cam, rightSquareTest);

		//Draw headset
		if (vrDisplay) {
			vr::Texture_t leftTexture = {
				(void*)(uintptr_t)GLuint(fbLeftEyeRead.getTexture(GL_COLOR_ATTACHMENT0).getID()),
				vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
			vr::Texture_t rightTexture = {
				(void*)(uintptr_t)GLuint(fbRightEyeRead.getTexture(GL_COLOR_ATTACHMENT0).getID()),
				vr::TextureType_OpenGL, vr::ColorSpace_Gamma };

			vr::VRCompositor()->Submit(vr::Eye_Left, &leftTexture);
			vr::VRCompositor()->Submit(vr::Eye_Right, &rightTexture);
		}

		checkGLErrors("Buffer overflow?");

		glfwSwapBuffers(window);
		glfwPollEvents();
	}


	glfwTerminate();
	vr::VR_Shutdown();
}

void initGlad() {
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "GLAD initialization failed" << std::endl;
	}
}

vr::IVRSystem *initVR() {

	vr::EVRInitError error = vr::VRInitError_None;
	vr::IVRSystem *vrDisplay = vr::VR_Init(&error, vr::VRApplication_Scene);

	if (error != vr::VRInitError_None)
	{
		vrDisplay = nullptr;
		std::cout << "[Error]" << VR_GetVRInitErrorAsSymbol(error) << std::endl;
	}
	else if (!vr::VR_GetGenericInterface(vr::IVRRenderModels_Version, &error)) {
		printf("Failed to obtain render models\n");
		vrDisplay = nullptr;
	}
	else if (!vr::VRCompositor()) {
		printf("Compositor failed\n");
		vrDisplay = nullptr;
	}

	return vrDisplay;
}

void WindowManager::initGL() {
	glClearColor(1.f, 1.f, 1.f, 1.f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glViewport(0, 0, window_width, window_height);
}

GLFWwindow *createWindow(int width, int height, std::string name)
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
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

