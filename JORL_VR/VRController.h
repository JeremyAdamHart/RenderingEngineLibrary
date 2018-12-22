#pragma once

#include <openvr/openvr.h>
#include <Drawable.h>
#include <Texture.h>
#include <glm/glm.hpp>
#include <Object.h>

#define OculusTouch_EButton_A vr::k_EButton_A
#define OculusTouch_EButton_B vr::k_EButton_ApplicationMenu
#define OculusTouch_EButton_X vr::k_EButton_A
#define OculusTouch_EButton_Y vr::k_EButton_ApplicationMenu
#define OculusTouch_EJoystick vr::k_EButton_Axis0
#define OculusTouch_ETrigger vr::k_EButton_Axis1

#define Windows_EButton_Touchpad vr::k_EButton_SteamVR_Touchpad
#define Windows_ETrigger vr::k_EButton_SteamVR_Trigger
#define Windows_EJoystick vr::k_EButton_Axis2		//Guess
#define Windows_EButton_ApplicationMenu vr::k_EButton_ApplicationMenu

enum VRControllerType {
	OCULUS_TOUCH, VIVE, WINDOWS, UNKNOWN
};

enum VRButtonType {
	BUTTON, AXIS, TOUCHED
};

class VRAxis {
	vr::EVRButtonId button;
	glm::vec2 v;
public:
	VRAxis();
	VRAxis(vr::EVRButtonId);
	glm::vec2 value();
	void update(const vr::VRControllerState_t &state);
};

class VRButton {
	uint64_t mask;
	bool v;
public:
	VRButton();
	VRButton(vr::EVRButtonId button);
	bool value();
	void add(vr::EVRButtonId button);
	void update(const vr::VRControllerState_t &state);
};

class VRTouch {
	uint64_t mask;
	bool v;
public:
	VRTouch();
	VRTouch(vr::EVRButtonId button);
	bool value();
	void add(vr::EVRButtonId button);
	void update(const vr::VRControllerState_t &state);
};

class VRControllerInterface {
	std::map<int, VRButton> buttons;
	std::map<int, VRAxis> axes;
	std::map<int, VRTouch> touched;

	std::map<int, VRButtonType> actionTypes;
public:
	void assignAxis(int action, vr::EVRButtonId button);			//Only one axis can be paired with a button
	void assignButton(int action, vr::EVRButtonId button);			//Multiple buttons can be assigned to same action
	void assignTouch(int action, vr::EVRButtonId button);			//Multiple buttons can be assigned to same action

	float getScalar(int action);	//0 or 1 if BUTTON or TOUCHED, 0 to 1 if AXIS
	glm::vec2 getAxis(int action);	//Always returns {0, 0} if not AXIS
	bool getActivation(int action);		//True if activated

	void updateState(const vr::VRControllerState_t &state);
};

enum VRControllerHand : int {
	LEFT=0,
	RIGHT
};

class VRController : public renderlib::Drawable {
public:
	VRController();
	VRController(vr::TrackedDeviceIndex_t index, vr::IVRSystem *vrSystem,
		vr::TrackedDevicePose_t pose, renderlib::TextureManager *texManager);

	vr::TrackedDeviceIndex_t index;
	vr::RenderModel_t* renderModel;

	enum {
		TRIGGER_AXIS = 0,
		TRACKPAD_AXIS,
		AXIS_COUNT
	};
	enum {
		TRACKPAD_BUTTON = 0,
		TRIGGER_BUTTON,
		GRIP_BUTTON,
		MENU_BUTTON,
		BUTTON_COUNT
	};

//	void setControl(int actionID, vr::EVRButtonId button, )

	VRControllerInterface input;
	VRControllerType type;
	VRControllerHand hand;

	glm::vec2 axes[AXIS_COUNT];
	bool buttons[BUTTON_COUNT];
	bool trackpadTouched;



	void updatePose(const vr::TrackedDevicePose_t &pose);
	void updateState(const vr::VRControllerState_t &state);
	void loadModelMatrixOldOpenGL() const;
};

class VRSceneTransform :public renderlib::Object {
public:
	VRSceneTransform();
	VRSceneTransform(std::vector<VRController> *controllers);

	//Rotation modes
	enum {
		HANDLEBAR,
		ONE_HAND_PLUS_SCALE,
		ORIGIN_CONTROLLER,
		ORIGIN_MODEL
	};

	enum : int {
		TRANSFORM_CONTROL=0,
		ACTION_COUNT
	};

	float scale;

	glm::vec3 velocity;
	glm::quat angularVelocity;
	int rotationMode;
	int rotationOrigin;

	void setPosition(glm::vec3 position);

	glm::mat4 getTransform() const override;
	void updateTransform(float deltaTime, glm::vec3 grabPositionModelspace);
	bool multMatrixPreviewTransform(float modelScale);
	void multMatrixOldOpenGL();
	void linkControllers(std::vector<VRController> *newControllers);
private:
	std::vector<VRController> *controllers;
	std::vector<glm::vec3> lastPosition;
	std::vector<glm::quat> lastOrientation;
};