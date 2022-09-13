// OpenVRTest.cpp : Defines the entry point for the console application.
//

#include "VRWindow.h"
#include <string>
#include <iostream>

#include "ConvexHull.h"
#include "TrackballCamera.h"
#include "SimpleTexManager.h"
#include "TextureCreation.h"
#include "TextureMat.h"
#include "simpleTexShader.h"
#include "BlinnPhongShader.h"
#include "MeshInfoLoader.h"

#include "VRContext.h"
#include "VRController.h"
#include "VRDeviceManager.h"
#include <stb/stb_image_write.h>

#include <glm/gtc/matrix_transform.hpp>

int gWindowWidth, gWindowHeight;

bool reloadShaders = false;

using namespace renderlib;

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

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE)
		reloadShaders = true;
}

#include <assert.h>

enum {
	SCREENSHOT_CONTROL=VRSceneTransform::ACTION_COUNT
};

void setControllerBindingsOculusTouch(VRControllerInterface *input, VRControllerHand hand) {
	input->assignButton(VRSceneTransform::TRANSFORM_CONTROL, vr::k_EButton_Grip);
}

void setControllerBindingsVive(VRControllerInterface *input, VRControllerHand hand) {
	input->assignButton(VRSceneTransform::TRANSFORM_CONTROL, vr::k_EButton_Grip);
	input->assignButton(SCREENSHOT_CONTROL, vr::k_EButton_SteamVR_Trigger);
}

void setControllerBindingsWindows(VRControllerInterface *input, VRControllerHand hand) {
	input->assignButton(VRSceneTransform::TRANSFORM_CONTROL, vr::k_EButton_Grip);
}

void setBindings(VRControllerType type, VRControllerInterface *interface, VRControllerHand hand) {
	if (type == VRControllerType::VIVE) {
		setControllerBindingsVive(interface, hand);	
	}
	else if (type == VRControllerType::OCULUS_TOUCH) {
		setControllerBindingsOculusTouch(interface, hand);
	}
	else if (type == VRControllerType::WINDOWS) {
		setControllerBindingsWindows(interface, hand);
	}
	else {
		printf("Error: Unknown controller model - Using Vive controls as default\n");
		setControllerBindingsVive(interface, hand);
	}
}

struct ControllerReferenceFilepaths {
	char* trackpadFrame;
	char* drawPosition;
	char* grabPosition;

	ControllerReferenceFilepaths(VRControllerType type) :trackpadFrame(nullptr), drawPosition(nullptr), grabPosition(nullptr) {
		switch (type) {
		case VRControllerType::VIVE:
		case VRControllerType::UNKNOWN:
		case VRControllerType::INDEX:
			printf("Detected vive controller\n");
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

//Changes filename if file exists with that name
string findFilenameVariation(string filepath) {
	int counter = 1;
	size_t splitPos = filepath.find_last_of('.');
	if (splitPos > filepath.size()) {
		splitPos = filepath.size() - 1;
	}
	FILE *f;
	errno_t err = fopen_s(&f, filepath.c_str(), "r");
	while (f != nullptr) {
		if (counter == 1)
			filepath.insert(splitPos, to_string(counter));
		else {
			filepath.erase(splitPos, to_string(counter - 1).size());
			filepath.insert(splitPos, to_string(counter));
		}
		errno_t err = fopen_s(&f, filepath.c_str(), "r");
		counter++;
	}

	return filepath;
}

struct Bindings {
	vr::VRActionSetHandle_t actionSet;

	vr::VRActionHandle_t grabLeft = 0;
	vr::VRActionHandle_t grabRight = 0;
	vr::VRActionHandle_t position2D = 0;
	vr::VRActionHandle_t button = 0;
	vr::VRActionHandle_t trigger = 0;
	vr::VRActionHandle_t hapticLeft = 0;
	vr::VRActionHandle_t hapticRight = 0;
	vr::VRActionHandle_t poseLeft = 0;
	vr::VRActionHandle_t poseRight = 0;
};

Bindings setBindings(const char* filePath) {
	Bindings b;

	setActionPath(filePath);
	b.actionSet = getActionSetHandle("/actions/demo");
	b.grabLeft = getActionHandle("/actions/demo/in/Grab_Left");
	b.grabRight = getActionHandle("/actions/demo/in/Grab_Right");
	b.button = getActionHandle("/actions/demo/in/Button");
	b.trigger = getActionHandle("/actions/demo/in/Trigger");
	b.position2D = getActionHandle("/actions/demo/in/Position2D");
	b.hapticLeft = getActionHandle("/actions/demo/out/Haptic_Left");
	b.hapticRight = getActionHandle("/actions/demo/out/Haptic_Right");
	b.poseLeft = getActionHandle("/actions/demo/in/Hand_Left");
	b.poseRight = getActionHandle("/actions/demo/in/Hand_Right");
	//getActionHandle("DefinitelyFake");
	/*
	//vr::VRInput()->GetActionSetHandle("/actions/demo", &b.actionSet);
	vr::VRInput()->GetActionHandle("/actions/demo/in/Grab_Left", &b.grabLeft);
	vr::VRInput()->GetActionHandle("/actions/demo/in/Grab_Right", &b.grabRight);
	vr::VRInput()->GetActionHandle("/actions/demo/in/Position2D", &b.position2D);
	vr::VRInput()->GetActionHandle("/actions/demo/out/Haptic_Left", &b.hapticLeft);
	vr::VRInput()->GetActionHandle("/actions/demo/out/Haptic_Right", &b.hapticRight);
	vr::VRInput()->GetActionHandle("/actions/demo/in/Hand_Left", &b.poseLeft);
	vr::VRInput()->GetActionHandle("/actions/demo/in/Hand_Right", &b.poseRight);
	*/
	return b;
}

void modelLoop(WindowManager *wm, int sampleNumber) {
	printf("Model loop\n");
	glfwSetCursorPosCallback(wm->window, cursorPositionCallback);
	glfwSetWindowSizeCallback(wm->window, windowResizeCallback);

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
		vec2(0, 1.f),
		vec2(1.f, 0.f),
		vec2(1.f, 1.f),
		//Second triangle
		vec2(1.f, 0.f),
		vec2(0.f, 1.f),
		vec2(0.f, 0.f)
	};

	SimpleTexManager tm;
	VRContext vrContext(&tm);

	if (!vrContext.vrSystem) {
		vr::VR_Shutdown();
		glfwTerminate();
		return;
	}

	VRDeviceManager<VRCameraController, VRController> devices (vrContext.vrSystem, &tm);

	Framebuffer fbWindow(wm->window_width, wm->window_height);
	gWindowWidth = wm->window_width;
	gWindowHeight = wm->window_height;
	
	unsigned int TEX_WIDTH = vrContext.renderTargetWidth();
	unsigned int TEX_HEIGHT = vrContext.renderTargetHeight();

	int NUM_SAMPLES = sampleNumber;

	Framebuffer fbLeftEyeDraw = createFramebufferWithColorAndDepth(vrContext.renderTargetWidth(), vrContext.renderTargetHeight(), &tm, sampleNumber); //createNewFramebuffer(vrContext.renderTargetWidth(), vrContext.renderTargetHeight());
	Framebuffer fbRightEyeDraw = createFramebufferWithColorAndDepth(vrContext.renderTargetWidth(), vrContext.renderTargetHeight(), &tm, sampleNumber);

	Viewport leftEyeView(wm->window_width, wm->window_height);
	Viewport rightEyeView(wm->window_width / 2, wm->window_height, wm->window_width / 2);

	//Parse tracked devices
	int headsetIndex = 0;
	VRController *controllers = devices.controllers;

	VRControllerType controllerType = controllers[0].type;

	//setBindings(controllerType, &controllers[VRControllerHand::LEFT].input, VRControllerHand::LEFT);
	//setBindings(controllerType, &controllers[VRControllerHand::RIGHT].input, VRControllerHand::RIGHT);

	//Squares for left and right views
	Drawable leftSquare(
		make<TextureGeometry>(GL_TRIANGLES, points, coords, 6),
		make<TextureMat>(vrContext.getTexture(vr::Eye_Left)));

	Drawable rightSquare(
		make<TextureGeometry>(GL_TRIANGLES, points, coords, 6),
		make<TextureMat>(vrContext.getTexture(vr::Eye_Right)));

	SimpleTexShader texShader;
	BlinnPhongShader bpTexShader(BPTextureUsage::TEXTURE);
	BlinnPhongShader bpShader;

	vec3 lightPos(-100.f, 100.f, 100.f);

	fbWindow.use();

	vector<Drawable> drawables;
	drawables.push_back(Drawable(objToElementGeometry("models/dragon.obj"), make_shared<ShadedMat>(0.3, 0.4, 0.4, 10.0)));
	drawables.back().addMaterial(new ColorMat(vec4(1, 0, 0, 1)));

	//@TODO Encapsulate this in something
	//Trackpad frame
	ControllerReferenceFilepaths controllerPath(controllerType);

	//Draw position
	MeshInfoLoader drawPositionObj(controllerPath.drawPosition);
	vec3 drawPositionModelspace = drawPositionObj.vertices[0];

	//Grab position
	MeshInfoLoader grabPositionObj(controllerPath.grabPosition);
	vec3 grabPositionModelspace = grabPositionObj.vertices[0];

	//Time tracking
	double frameTime = 0.f;
	int frameTimeSamples = 0;
	double lastTime = glfwGetTime();

	VRSceneTransform sceneTransform;
	sceneTransform.setPosition(vec3(0.f, 1.f, -1.f));

	//Screenshots
	const int SCREENSHOT_WIDTH = 4000;
	const int SCREENSHOT_HEIGHT = 3000;
	Framebuffer fbScreenshotDraw = createFramebufferWithColorAndDepth(SCREENSHOT_WIDTH, SCREENSHOT_HEIGHT, &tm, 8);
	Framebuffer fbScreenshotRead = createFramebufferWithColorAndDepth(SCREENSHOT_WIDTH, SCREENSHOT_HEIGHT, &tm);
	bool screenshotPressed = false;

	//Setup input
	Bindings b = setBindings("./sample_action.json");

	while (!glfwWindowShouldClose(wm->window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Input
		updateActionSet(b.actionSet);

		if (getScalar(b.trigger) > 0.1f)
			printf("Trigger = %f\n", getScalar(b.trigger));

		if (getActivation(b.button))
			printf("Button activated for first time\n");
		if (getBool(b.button))
			printf("Button currently pressed\n");

		glm::vec2 v = getAxis(b.position2D);
		if(length(v) > 0.01f)
			printf("(%f, %f)\n", v.x, v.y);

		if (gWindowWidth != wm->window_width || gWindowHeight != wm->window_height) {
			wm->window_width = gWindowWidth;
			wm->window_height = gWindowHeight;
			fbWindow.resize(wm->window_width, wm->window_height);
			leftEyeView.width = wm->window_width;
			leftEyeView.height = wm->window_height;
			rightEyeView.x = leftEyeView.width;
			rightEyeView.width = wm->window_width - leftEyeView.width;
			rightEyeView.height = wm->window_height;
		}

		devices.updatePose();
		devices.updateState(vrContext.vrSystem);

		//Get time
		static double lastTime = 0.f;
		double currentTime = glfwGetTime();
		sceneTransform.updateTransformNew(
			currentTime - lastTime, 
			devices.controllers[0], grabPositionModelspace, b.grabLeft,
			devices.controllers[1], grabPositionModelspace, b.grabRight);	//Not using time yet
		lastTime = currentTime;

		//Update model transforms
		for (int i = 0; i < drawables.size(); i++) {
			drawables[i].setOrientation(sceneTransform.getOrientationQuat());
			drawables[i].setPosition(sceneTransform.getPos());
			drawables[i].setScale(vec3(sceneTransform.scale));
		}

		////////////
		// DRAWING
		///////////
		glLineWidth(10.f);
		glEnable(GL_MULTISAMPLE);
		glClearColor(0.f, 0.f, 0.f, 0.f);

		//Draw left eye
		fbLeftEyeDraw.use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		for (int i = 0; i < 2; i++)
			bpTexShader.draw(devices.hmd.leftEye, lightPos, devices.controllers[i]);
		for (int i = 0; i < drawables.size(); i++) {
			bpShader.draw(devices.hmd.leftEye, lightPos, drawables[i]);		//Add lightPos and colorMat checking
		}

		vrContext.submitFrame(fbLeftEyeDraw, vr::EVREye::Eye_Left);
		vrContext.submitFrame(fbRightEyeDraw, vr::EVREye::Eye_Right);

		//Draw right eye
		fbRightEyeDraw.use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		for (int i = 0; i < 2; i++)
			bpTexShader.draw(devices.hmd.rightEye, lightPos, devices.controllers[i]);
		for (int i = 0; i < drawables.size(); i++) {
			bpShader.draw(devices.hmd.rightEye, lightPos, drawables[i]);
		}

		glDisable(GL_MULTISAMPLE);

		//Draw window
		fbWindow.use();
		leftEyeView.use();
		glClearColor(1.0, 1.0, 1.0, 0.0);
		texShader.draw(cam, leftSquare);

		glEnable(GL_BLEND);

		if (frameTimeSamples > 30) {
			double currentTime = glfwGetTime();
			frameTime = currentTime - lastTime;
			frameTimeSamples = 0;
			lastTime = currentTime;
		}
		else {
			frameTimeSamples++;
		}

		//Write screenshot to file
		if (!screenshotPressed && controllers[VRControllerHand::LEFT].input.getActivation(SCREENSHOT_CONTROL)) {

			float aspectRatio = float(SCREENSHOT_WIDTH) / float(SCREENSHOT_HEIGHT);
			static mat4 screenshotProjection = glm::perspective(radians(80.f), aspectRatio, 0.01f, 10.f);

			glDisable(GL_BLEND);
			mat4 savedProjection = devices.hmd.leftEye.projection;
			devices.hmd.leftEye.projection = screenshotProjection;
			fbScreenshotDraw.use();
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			for (int i = 0; i < 2; i++)
				bpTexShader.draw(devices.hmd.leftEye, lightPos, devices.controllers[i]);
			for (int i = 0; i < drawables.size(); i++) {
				bpShader.draw(devices.hmd.leftEye, lightPos, drawables[i]);		//Add lightPos and colorMat checking
			}

			blit(fbScreenshotDraw, fbScreenshotRead);
			Texture screenshotTex = fbScreenshotRead.getTexture(GL_COLOR_ATTACHMENT0);

			string filename = findFilenameVariation("Screenshot.png");

			size_t imageSize = screenshotTex.getWidth()*screenshotTex.getHeight() * 4;
			unsigned char *data = new unsigned char[imageSize];
			glGetTextureImage(screenshotTex.getID(), 0, GL_RGBA, GL_UNSIGNED_BYTE, imageSize, data);
			stbi_flip_vertically_on_write(true);
			stbi_write_png(filename.c_str(), screenshotTex.getWidth(), screenshotTex.getHeight(), 4, data, 0);

			printf("Saved %s\n", filename.c_str());

			delete[] data;
			screenshotPressed = true;
			devices.hmd.leftEye.projection = savedProjection;
		}
		else if (!controllers[VRControllerHand::LEFT].input.getActivation(SCREENSHOT_CONTROL))
			screenshotPressed = false;

		glfwSwapBuffers(wm->window);
		glfwPollEvents();
	}

	glfwTerminate();
	vr::VR_Shutdown();
}

int main(int argc, char** argv)
{
	WindowManager wm(800, 400, "VR Segmenting");

	/*
	SlotMap<int> map;

	SlotMap<int>::Index a = map.add(1);
	SlotMap<int>::Index b = map.add(2);
	SlotMap<int>::Index c = map.add(3);
	map.remove(b);
	SlotMap<int>::Index d = map.add(4);
	SlotMap<int>::Index e = map.add(5);

	for (auto it = map.begin(); it != map.end(); ++it) {
		std::cout << (*it) << std::endl;
	}
	HalfEdgeMesh<glm::vec3> mesh;
	generateTetrahedron(mesh, glm::vec3(0, 0, 0), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));

	std::vector<glm::vec3> points;
	std::vector<int> indices;
	halfEdgeToFaceList(&points, &indices, mesh);
	*/

	char* loadFilename = "icosahedron.ply";	//"untrackedmodels/riccoSurface_take3.obj";	//"models/dragon.obj";
	char* saveFilename = "saved/default.clr";
	int multisampling = 8;
	switch (argc) {
	case 1:
		//Change to ricco
		loadFilename = "models/dragon.obj";
		//loadFilename = "untrackedmodels/Craspedia2.ply";	// "untrackedmodels/Helianthus4.ply";	//"untrackedmodels/GRCD2RNA.ply";	//"models/Cube.obj";	//"models/icosahedron.ply";
		saveFilename = "saved/default.clr";
		break;
	case 2:
		loadFilename = argv[1];
		saveFilename = argv[1];
		break;
	case 3:
		loadFilename = argv[1];
		saveFilename = argv[2];
		break;
	case 4:
		loadFilename = argv[1];
		saveFilename = argv[2];
		multisampling = std::stoi(argv[3]);
	}

	modelLoop(&wm, 8);		//paintingLoop(loadFilename, saveFilename, multisampling);
}