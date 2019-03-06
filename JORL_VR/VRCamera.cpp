#include "VRCamera.h"
#include "VRTools.h"

#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

VRCamera::VRCamera(): camMatrix(1.f){}

void VRCamera::setCameraMatrix(mat4 newCamMatrix) {
	camMatrix = newCamMatrix;
}

mat4 VRCamera::getCameraMatrix() const {
	return camMatrix;
}

vec3 VRCamera::getPosition() const {
	vec4 position = inverse(camMatrix)*vec4(0, 0, 0, 1);
	return vec3(position.x, position.y, position.z);
}

vec3 VRCamera::getDirection() const {
	vec4 direction = inverse(camMatrix)*vec4(0, 0, -1, 0);
	return vec3(direction);
}

VRCameraController::VRCameraController(vr::IVRSystem *vrDisplay) :
leftEyeTransform(1.f), rightEyeTransform(1.f)
{
	setEyeTransforms(vrDisplay);
	setProjection(vrDisplay);

/*	float pfLeft, pfRight, pfTop, pfBottom;

	vrDisplay->GetProjectionRaw(vr::Eye_Left, &pfLeft, &pfRight, &pfTop, &pfBottom);
	printf("Left angle = %f\nRight angle = %f\nTop Angle = %f\nBottom angle = %f\n",
		pfLeft, pfRight, pfTop, pfBottom);
*/
}

/*mat4 toMat4(const vr::HmdMatrix44_t &hmdMat) {
	return {
		hmdMat.m[0][0], hmdMat.m[1][0], hmdMat.m[2][0], hmdMat.m[3][0],
		hmdMat.m[0][1], hmdMat.m[1][1], hmdMat.m[2][1], hmdMat.m[3][1],
		hmdMat.m[0][2], hmdMat.m[1][2], hmdMat.m[2][2], hmdMat.m[3][2],
		hmdMat.m[0][3], hmdMat.m[1][3], hmdMat.m[2][3], hmdMat.m[3][3]};
}*/

void VRCameraController::setProjection(vr::IVRSystem *vrDisplay, float nearD, float farD) {
	mat4 newProjection = glm::ortho(-0.02f, 0.02f, -0.02f, 0.02f, nearD, farD);

	mat4 leftProjection = toMat4(
		vrDisplay->GetProjectionMatrix(vr::Eye_Left, nearD, farD));
	leftEye.setProjectionMatrix(leftProjection);
	mat4 rightProjection = toMat4(
		vrDisplay->GetProjectionMatrix(vr::Eye_Right, nearD, farD));
	rightEye.setProjectionMatrix(rightProjection);
}


void VRCameraController::setEyeTransforms(vr::IVRSystem *vrDisplay) {
	leftEyeTransform = inverse(toMat4(vrDisplay->GetEyeToHeadTransform(vr::Eye_Left)));
	rightEyeTransform = inverse(toMat4(vrDisplay->GetEyeToHeadTransform(vr::Eye_Right)));
}

void VRCameraController::updatePose(const vr::TrackedDevicePose_t& pose) {
	mat4 headTransform = inverse(toMat4(pose.mDeviceToAbsoluteTracking));

	leftEye.setCameraMatrix(leftEyeTransform*headTransform);
	rightEye.setCameraMatrix(rightEyeTransform*headTransform);
}