#include "VRController.h"
#include <glmSupport.h>
#include "VRTools.h"
#include "UndoStack.h"	//Included for ring stack -- Separate?
#include <filesystem>

using namespace renderlib;
using namespace glm;

const char* ViveControllerName = "vr_controller_vive_1_5";
const char* OculusTouchControllerName = "oculus_cv1_controller";	//Left and right controllers named differently
const char* WindowsControllerName = "Microsoft/Windows/OpenVR\\controller_1627_1118";
const char* IndexControllerName = "valve_controller_knu_1_0";

string getRenderModelErrorString(vr::EVRRenderModelError error) {
	switch (error) {
	case vr::VRRenderModelError_BufferTooSmall:
		return "BufferTooSmall";
	case vr::VRRenderModelError_InvalidArg:
		return "InvalidArg";
	case vr::VRRenderModelError_InvalidModel:
		return "InvalidModel";
	case vr::VRRenderModelError_InvalidTexture:
		return "InvalidTexture";
	case vr::VRRenderModelError_Loading:
		return "Loading";
	case vr::VRRenderModelError_MultipleShapes:
		return "MultipleShapes";
	case vr::VRRenderModelError_MultipleTextures:
		return "MultipleTextures";
	case vr::VRRenderModelError_NoShapes:
		return "NoShapes";
	case vr::VRRenderModelError_NotEnoughNormals:
		return "NotEnoughNormals";
	case vr::VRRenderModelError_NotEnoughTexCoords:
		return "NotEnoughTexCoords";
	case vr::VRRenderModelError_NotSupported:
		return "NotSupported";
	case vr::VRRenderModelError_TooManyVertices:
		return "TooManyVertices";
	}
}

VRAxis::VRAxis() {}
VRAxis::VRAxis(vr::EVRButtonId button) :button(button) {}
vec2 VRAxis::value() { return v; }
void VRAxis::update(const vr::VRControllerState_t &state) {
	v = vec2(
		state.rAxis[button - vr::k_EButton_Axis0].x,
		state.rAxis[button - vr::k_EButton_Axis0].y);
}

void VRAxis::set(glm::vec2 value) { v = value; }

VRButton::VRButton() :mask(0){}
VRButton::VRButton(vr::EVRButtonId button) :mask(vr::ButtonMaskFromId(button)) {}
bool VRButton::value() { return v; }
void VRButton::add(vr::EVRButtonId button) { mask |= vr::ButtonMaskFromId(button); }
void VRButton::update(const vr::VRControllerState_t &state) {
	v = mask & state.ulButtonPressed;
}

void VRButton::set(bool value) { v = value; }

VRTouch::VRTouch() :mask(0){}
VRTouch::VRTouch(vr::EVRButtonId button) :mask(vr::ButtonMaskFromId(button)) {}
bool VRTouch::value() { return v; }
void VRTouch::add(vr::EVRButtonId button) { mask |= vr::ButtonMaskFromId(button); }
void VRTouch::update(const vr::VRControllerState_t &state) {
	uint64_t bitMask = mask & state.ulButtonTouched;
	v = (mask & state.ulButtonTouched) != 0;
}

void VRTouch::set(bool value) { v = value; }

void VRControllerInterface::assignAxis(int action, vr::EVRButtonId button) {
//	buttons.erase(action);
//	touched.erase(action);
	axes[action] = VRAxis(button);

	actionTypes[action] = VRButtonType::AXIS;
}

void VRControllerInterface::assignButton(int action, vr::EVRButtonId button) {
//	axes.erase(action);
//	touched.erase(action);
	auto loc = buttons.find(action);
	if (loc != buttons.end())
		buttons[action] = VRButton(button);
	else
		buttons[action].add(button);

	actionTypes[action] = VRButtonType::BUTTON;
}

void VRControllerInterface::assignTouch(int action, vr::EVRButtonId button) {
//	axes.erase(action);
//	buttons.erase(action);
	auto loc = touched.find(action);
	if (loc != touched.end())
		touched[action] = VRTouch(button);
	else
		touched[action].add(button);

	actionTypes[action] = VRButtonType::TOUCHED;
}

float VRControllerInterface::getScalar(int action) {
	return axes.at(action).value().x;
}

glm::vec2 VRControllerInterface::getAxis(int action) {
	return axes.at(action).value();
}

bool VRControllerInterface::getActivation(int action) {
	bool buttonValue = (buttons.find(action) != buttons.end()) ? buttons[action].value() : false;
	bool touchValue = (touched.find(action) != touched.end()) ? touched[action].value() : false;

	return buttonValue || touchValue;
}

void VRControllerInterface::setScalar(int action, float value){
	axes[action].set(vec2(value, 0));
}

void VRControllerInterface::setAxis(int action, vec2 value)
{
	axes[action].set(value);
}

void VRControllerInterface::setActivation(int action, bool value)
{
	auto buttonLoc = buttons.find(action);
	if (buttonLoc != buttons.end()) buttonLoc->second.set(value);
	auto touchLoc = touched.find(action);
	if (touchLoc != touched.end()) touchLoc->second.set(value);
}

void VRControllerInterface::updateState(const vr::VRControllerState_t &state) {
	for (auto &axis : axes)
		axis.second.update(state);
	for (auto &button : buttons)
		button.second.update(state);
	for (auto &touch : touched)
		touch.second.update(state);
}

VRController::VRController() :renderModel(nullptr) {

}

VRController::VRController(vr::TrackedDeviceIndex_t index, vr::IVRSystem *vrSystem, TextureManager *texManager)
{

	char nameBuffer[1024];
	vrSystem->GetStringTrackedDeviceProperty(index,
		vr::Prop_RenderModelName_String, nameBuffer, 1024);

	vr::EVRRenderModelError error = vr::VRRenderModels()->LoadRenderModel_Async(
		nameBuffer, &renderModel);

	/*int32_t whichHand = vrSystem->GetInt32TrackedDeviceProperty(index, vr::ETrackedDeviceProperty::Prop_ControllerRoleHint_Int32);
	hand = (whichHand == vr::ETrackedControllerRole::TrackedControllerRole_LeftHand) ? 
		VRControllerHand::LEFT : VRControllerHand::RIGHT;
		*/

	if (strcmp(nameBuffer, ViveControllerName) == 0)
		type = VRControllerType::VIVE;
	else if (string(nameBuffer).find(WindowsControllerName) != string::npos)
		type = VRControllerType::WINDOWS;
	else if (string(nameBuffer).find(IndexControllerName) != string::npos)
		type = VRControllerType::INDEX;
	else
		type = VRControllerType::OCULUS_TOUCH;

	printf("Detected controller: %s\n", nameBuffer);

	while (error == vr::VRRenderModelError_Loading)
		error = vr::VRRenderModels()->LoadRenderModel_Async(
			nameBuffer, &renderModel);

	if (error == vr::VRRenderModelError_None)
		openvrRenderModelToDrawable(this, renderModel, texManager);
	else {
		printf("RenderModelError - %s\n", getRenderModelErrorString(error).c_str());
	}
}

void VRController::updatePose(const vr::TrackedDevicePose_t &pose) {
	vr::HmdMatrix34_t poseMatrix = pose.mDeviceToAbsoluteTracking;
	position = getTranslation(poseMatrix);
	orientation = normalize(quat_cast(getRotation(poseMatrix)));
}

void VRController::updateState(const vr::VRControllerState_t &state) {

	input.updateState(state);
}

void VRController::loadModelMatrixOldOpenGL() const {
	mat4 transform = getTransform();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glMultMatrixf(&transform[0][0]);

}

VRSceneTransform::VRSceneTransform() :
	Object(vec3(0.f), quat()), scale(1.f),
	velocity(0.f), angularVelocity(), //controllers(nullptr), 
	rotationMode(HANDLEBAR), rotationOrigin(ORIGIN_CONTROLLER),
	lastPosition(2, vec3(0.f)), lastOrientation(2, quat()) {}

VRSceneTransform::VRSceneTransform(vector<VRController> *controllers) : VRSceneTransform() {}
/*
	Object(vec3(0.f), quat()), scale(1.f),
	velocity(0.f), angularVelocity(), controllers(controllers), rotationMode(HANDLEBAR), rotationOrigin(ORIGIN_CONTROLLER)
	{
	linkControllers(controllers);
}

void VRSceneTransform::linkControllers(vector<VRController> *newControllers) {
	controllers = newControllers;
	lastPosition.resize(controllers->size());
	lastOrientation.resize(controllers->size());

	for (int i = 0; i < controllers->size(); i++) {
		lastPosition[i] = controllers->at(i).getPos();
		lastOrientation[i] = controllers->at(i).getOrientationQuat();
	}
}
*/
void VRSceneTransform::setPosition(glm::vec3 newPos) { position = newPos; }

mat4 VRSceneTransform::getTransform() const {
	mat4 rigidTransform = Object::getTransform();
	mat4 scaleMatrix = mat4(
		scale, 0, 0, 0,
		0, scale, 0, 0,
		0, 0, scale, 0,
		0, 0, 0, 1);

	return rigidTransform*scaleMatrix;
}

void VRSceneTransform::multMatrixOldOpenGL() {
	/*	mat4 rigidTransform = getTransform();
	mat4 scaleMatrix = mat4(
	scale, 0, 0, 0,
	0, scale, 0, 0,
	0, 0, scale, 0,
	0, 0, 0, 1);
	*/
	mat4 transform = getTransform();	//rigidTransform*scaleMatrix;

	glMatrixMode(GL_MODELVIEW_MATRIX);
	glMultMatrixf(&transform[0][0]);
}

vec3 makePerpendicular(vec3 vector, vec3 perpendicularTo) {
	vec3 proj = dot(vector, perpendicularTo) / dot(perpendicularTo, perpendicularTo)*perpendicularTo;
	vec3 perpendicular = vector - proj;
	float magnitude = length(perpendicular);
	if (magnitude > 0.000001)
		return perpendicular / magnitude;
	else
		return vec3(0);
}

vec3 toVec3(quat q) { return vec3(q.x, q.y, q.z); }

quat projectedQuaternionDiffOriginal(quat a, quat b, vec3 projAxis) {
	quat diffQuat = normalize(b)*inverse(normalize(a));
	float sinTheta = dot(normalize(projAxis), toVec3(diffQuat));
	float w = sqrt(1 - sinTheta*sinTheta);
	return quat(w, float(sinTheta)*normalize(projAxis));
}

quat projectedQuaternionDiff(quat a, quat b, vec3 projAxis) {
	quat diffQuat = normalize(b)*inverse(normalize(a));
	float theta = asin(length(toVec3(diffQuat)));	//acos(diffQuat.w);
	float w = dot(normalize(projAxis), normalize(toVec3(diffQuat)));
	return quat(cos(w*theta), sin(w*theta)*normalize(projAxis));
}

quat quaternionDiff(quat a, quat b) {
	return normalize(normalize(b)*inverse(normalize(a)));
}

//Not currently incorporating time - FIX
void VRSceneTransform::updateTransform(float deltaTime, const VRController& controllerA, const VRController& controllerB, vec3 grabPositionModelspace) {
	vector<VRController> controllers = {controllerA, controllerB};
	static RingStack<vec3> lastVelocities(5);
	static vec3 rotationCenter = vec3(0.f, 0.f, 0.f);
	float scaleChange = 1.f;
	static int lastGripsPressed = 0;

	vector<vec3> grabPositions;

	//Get indices of controllers which have the grip pressed
	std::vector<int> gripsPressed;
	for (int i = 0; i < controllers.size(); i++) {
		vec3 grabPosition = vec3(controllers[i].getTransform()*vec4(grabPositionModelspace, 1.f));
		grabPositions.push_back(grabPosition);

		if(controllers[i].input.getActivation(TRANSFORM_CONTROL))
			gripsPressed.push_back(i);
	}

	switch (gripsPressed.size()) {
	case 1:
	{
		int index = gripsPressed[0];
		velocity = (grabPositions[index] - lastPosition[index])/deltaTime;
		angularVelocity = slerp(angularVelocity, quat(), 0.1f);
		lastVelocities.push(velocity);
	//	angularVelocity = quat();
		break;
	}
	case 2:
	{
		int indexA = gripsPressed[0];
		int indexB = gripsPressed[1];

		velocity = vec3(0.f);
		angularVelocity = quat();

		vec3 axisA = lastPosition[indexA] - lastPosition[indexB];
		vec3 axisB = grabPositions[indexA] //controllers->at(indexA).getPos()
			- grabPositions[indexB];	//controllers->at(indexB).getPos();
		float lengthA = length(axisA);
		float lengthB = length(axisB);

		quat lastOrntnA = lastOrientation[indexA];
		quat lastOrntnB = lastOrientation[indexB];
		quat currOrntnA = controllers[indexA].getOrientationQuat();
		quat currOrntnB = controllers[indexB].getOrientationQuat();

		if (rotationMode == HANDLEBAR) {
			axisA = axisA / lengthA;
			axisB = axisB / lengthB;

			vec3 lastRotationCenter = rotationCenter;
			rotationCenter = 0.5f*(grabPositions[0] + grabPositions[1]);	//0.5f*(controllers->at(0).getPos() + controllers->at(1).getPos());

			if (lastGripsPressed == 2) {
				velocity = (rotationCenter - lastRotationCenter)/deltaTime;
			}

			vec3 rotAxis = cross(axisA, axisB);
			if (length(rotAxis) > 0.0001f) {
				float angle = asin(length(rotAxis));

				//Rolling
				quat orntnDiffA = projectedQuaternionDiff(lastOrntnA, currOrntnA, axisB);
				quat orntnDiffB = projectedQuaternionDiff(lastOrntnB, currOrntnB, axisB);

				angularVelocity = normalize(0.5f*(orntnDiffB+orntnDiffA))*angleAxis(angle, normalize(rotAxis));
			}
		}
		else if(rotationMode == ONE_HAND_PLUS_SCALE){
			quat orntnDiffB = quaternionDiff(lastOrntnB, currOrntnB);
			angularVelocity = orntnDiffB;
			rotationCenter = grabPositions[indexB];		//controllers->at(indexB).getPos();

		}
		
		scaleChange = lengthB / lengthA;
		scale *= scaleChange;		//Rescale model
		lastVelocities.push(velocity);
		break;
	}
	default:
	{
		if (lastVelocities.size() > 0) {
			vec3 averageVelocity(0.f);
			int originalSize = lastVelocities.size();
			for (int i = 0; i < originalSize; i++) {
				averageVelocity += lastVelocities.last();
				lastVelocities.pop();
			}
			velocity = averageVelocity / float(originalSize);
		}
		velocity *= 0.98f;
		angularVelocity = quat();
	}
	}

	//Integrate velocities
	//Scene centered rotation
	if (rotationOrigin == ORIGIN_CONTROLLER) {
		position = vec3(
			toMat4(normalize(angularVelocity))*vec4(scaleChange*(position - rotationCenter), 1))
			+ rotationCenter;
	}
	position += velocity*deltaTime;
	orientation = normalize(angularVelocity*orientation);

	//Save positions
	for (int i = 0; i < lastPosition.size(); i++) {
		lastPosition[i] = grabPositions[i];		//controllers->at(i).getPos();
		lastOrientation[i] = controllers[i].getOrientationQuat();
	}

	lastGripsPressed = gripsPressed.size();
}

void VRSceneTransform::updateTransformNew(float deltaTime,
	const VRController& controllerA, glm::vec3 grabPositionModelspaceA, vr::VRActionHandle_t gripButtonA,
	const VRController& controllerB, glm::vec3 grabPositionModelspaceB, vr::VRActionHandle_t gripButtonB) 
{
	glm::vec3 grabPositionModelspace[2] = { grabPositionModelspaceA, grabPositionModelspaceB };
	vr::VRActionHandle_t gripButtons[2] = { gripButtonA, gripButtonB };
	vector<VRController> controllers = { controllerA, controllerB };
	static RingStack<vec3> lastVelocities(5);
	static vec3 rotationCenter = vec3(0.f, 0.f, 0.f);
	float scaleChange = 1.f;
	static int lastGripsPressed = 0;

	vector<vec3> grabPositions;

	//Get indices of controllers which have the grip pressed
	std::vector<int> gripsPressed;
	for (int i = 0; i < controllers.size(); i++) {
		vec3 grabPosition = vec3(controllers[i].getTransform()*vec4(grabPositionModelspace[i], 1.f));
		grabPositions.push_back(grabPosition);

		if (getBool(gripButtons[i]))
			gripsPressed.push_back(i);
	}

	switch (gripsPressed.size()) {
	case 1:
	{
		int index = gripsPressed[0];
		velocity = (grabPositions[index] - lastPosition[index]) / deltaTime;
		angularVelocity = slerp(angularVelocity, quat(), 0.1f);
		lastVelocities.push(velocity);
		//	angularVelocity = quat();
		break;
	}
	case 2:
	{
		int indexA = gripsPressed[0];
		int indexB = gripsPressed[1];

		velocity = vec3(0.f);
		angularVelocity = quat();

		vec3 axisA = lastPosition[indexA] - lastPosition[indexB];
		vec3 axisB = grabPositions[indexA] //controllers->at(indexA).getPos()
			- grabPositions[indexB];	//controllers->at(indexB).getPos();
		float lengthA = length(axisA);
		float lengthB = length(axisB);

		quat lastOrntnA = lastOrientation[indexA];
		quat lastOrntnB = lastOrientation[indexB];
		quat currOrntnA = controllers[indexA].getOrientationQuat();
		quat currOrntnB = controllers[indexB].getOrientationQuat();

		if (rotationMode == HANDLEBAR) {
			axisA = axisA / lengthA;
			axisB = axisB / lengthB;

			vec3 lastRotationCenter = rotationCenter;
			rotationCenter = 0.5f*(grabPositions[0] + grabPositions[1]);	//0.5f*(controllers->at(0).getPos() + controllers->at(1).getPos());

			if (lastGripsPressed == 2) {
				velocity = (rotationCenter - lastRotationCenter) / deltaTime;
			}

			vec3 rotAxis = cross(axisA, axisB);
			if (length(rotAxis) > 0.0001f) {
				float angle = asin(length(rotAxis));

				//Rolling
				quat orntnDiffA = projectedQuaternionDiff(lastOrntnA, currOrntnA, axisB);
				quat orntnDiffB = projectedQuaternionDiff(lastOrntnB, currOrntnB, axisB);

				angularVelocity = normalize(0.5f*(orntnDiffB + orntnDiffA))*angleAxis(angle, normalize(rotAxis));
			}
		}
		else if (rotationMode == ONE_HAND_PLUS_SCALE) {
			quat orntnDiffB = quaternionDiff(lastOrntnB, currOrntnB);
			angularVelocity = orntnDiffB;
			rotationCenter = grabPositions[indexB];		//controllers->at(indexB).getPos();

		}

		scaleChange = lengthB / lengthA;
		scale *= scaleChange;		//Rescale model
		lastVelocities.push(velocity);
		break;
	}
	default:
	{
		if (lastVelocities.size() > 0) {
			vec3 averageVelocity(0.f);
			int originalSize = lastVelocities.size();
			for (int i = 0; i < originalSize; i++) {
				averageVelocity += lastVelocities.last();
				lastVelocities.pop();
			}
			velocity = averageVelocity / float(originalSize);
		}
		velocity *= 0.98f;
		angularVelocity = quat();
	}
	}

	//Integrate velocities
	//Scene centered rotation
	if (rotationOrigin == ORIGIN_CONTROLLER) {
		position = vec3(
			toMat4(normalize(angularVelocity))*vec4(scaleChange*(position - rotationCenter), 1))
			+ rotationCenter;
	}
	position += velocity * deltaTime;
	orientation = normalize(angularVelocity*orientation);

	//Save positions
	for (int i = 0; i < lastPosition.size(); i++) {
		lastPosition[i] = grabPositions[i];		//controllers->at(i).getPos();
		lastOrientation[i] = controllers[i].getOrientationQuat();
	}

	lastGripsPressed = gripsPressed.size();
}

/*
bool VRSceneTransform::multMatrixPreviewTransform(float modelScale) {
	std::vector<int> gripsPressed;
	for (int i = 0; i < controllers->size(); i++) {
		if (controllers->at(i).input.getActivation(TRANSFORM_CONTROL))	//buttons[VRController::GRIP_BUTTON])
			gripsPressed.push_back(i);
	}
	if (gripsPressed.size() < 2)
		return false;

	vec3 controllerPos[2];
	controllerPos[0] = controllers->at(gripsPressed[0]).getPos();
	controllerPos[1] = controllers->at(gripsPressed[1]).getPos();
	float controllerDist = length(controllerPos[0] - controllerPos[1]);
	float newScale = controllerDist / (2.f*modelScale);
	vec3 controllerMidpoint = (controllerPos[0] + controllerPos[1])*0.5f;

	mat4 translationToMidpoint = translateMatrix(controllerMidpoint);
	mat4 scaleBetweenControllers = scaleMatrix(newScale);

	mat4 modelMatrix = translationToMidpoint*scaleBetweenControllers*getOrientationMat4();

	glMatrixMode(GL_MODELVIEW);
	//	glPushMatrix();
	glMultMatrixf(&modelMatrix[0][0]);

	return true;
}
*/

void setActionPath(const char * actionFilename)
{
	namespace fs = std::filesystem;
	fs::path filepath = actionFilename;
	auto result = vr::VRInput()->SetActionManifestPath(fs::absolute(filepath).string().c_str());
	if (result != vr::VRInputError_None) {
		printf("Failed to set action manifest path with error: %d\n", result);
	}

}

void updateActionSet(vr::VRActionSetHandle_t actionSetHandle) {
	vr::VRActiveActionSet_t actionSet = { 0 };
	actionSet.ulActionSet = actionSetHandle;
	vr::VRInput()->UpdateActionState(&actionSet, sizeof(actionSet), 1);
}

float getScalar(vr::VRActionHandle_t action)
{
	vr::InputAnalogActionData_t actionData;
	auto result = vr::VRInput()->GetAnalogActionData(action, &actionData, sizeof(actionData), vr::k_ulInvalidInputValueHandle);
	if (result != vr::VRInputError_None) {
		printf("Failed to get scalar with error: %d\n", result);
	}
	return (actionData.bActive) ? actionData.x : 0.f;

}

glm::vec2 getAxis(vr::VRActionHandle_t action)
{
	vr::InputAnalogActionData_t actionData;
	auto result = vr::VRInput()->GetAnalogActionData(action, &actionData, sizeof(actionData), vr::k_ulInvalidInputValueHandle);
	if (result != vr::VRInputError_None) {
		printf("Failed to get vector with error: %d\n", result);
	}
	return (actionData.bActive) ? glm::vec2(actionData.x, actionData.y) : glm::vec2(0.f);
}

bool getBool(vr::VRActionHandle_t action)
{
	vr::InputDigitalActionData_t actionData;
	auto result = vr::VRInput()->GetDigitalActionData(action, &actionData, sizeof(actionData), vr::k_ulInvalidInputValueHandle);
	if (result != vr::VRInputError_None) {
		printf("Failed to get bool with error: %d\n", result);
	}
	return actionData.bActive && actionData.bState;
}

bool getActivation(vr::VRActionHandle_t action)
{
	vr::InputDigitalActionData_t actionData;
	auto result = vr::VRInput()->GetDigitalActionData(action, &actionData, sizeof(actionData), vr::k_ulInvalidInputValueHandle);
	if (result != vr::VRInputError_None) {
		printf("Failed to get activation with error: %d\n", result);
	}
	return actionData.bActive && actionData.bState && actionData.bChanged;
}

vr::VRActionHandle_t getActionHandle(const char * name)
{
	vr::VRActionHandle_t handle = 0;
	auto result = vr::VRInput()->GetActionHandle(name, &handle);
	if (result != vr::VRInputError_None)
		printf("Failed to get action handle with error: %d\n", result);
	return handle;
}

vr::VRActionSetHandle_t getActionSetHandle(const char * name)
{
	vr::VRActionSetHandle_t handle = 0;
	auto result = vr::VRInput()->GetActionSetHandle(name, &handle);
	if (result != vr::VRInputError_None)
		printf("Failed to get action set handle with error: %d\n", result);
	return handle;
}
