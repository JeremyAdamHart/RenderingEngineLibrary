#pragma once

#include <vector>
#include <map>
#include <openvr/openvr.h>
#include "VRController.h"		//Move VRController hand elsewhere
#include "Texture.h"

template<typename Headset, typename Controller>
class VRDeviceManager {
	vr::TrackedDevicePose_t poses[vr::k_unMaxTrackedDeviceCount];
	std::map<VRControllerHand, int> handDeviceMap;
	int hmdIndex;

public:
	static const int NUM_CONTROLLERS = 2;

	Headset hmd;
	Controller controllers [NUM_CONTROLLERS];

	VRDeviceManager(vr::IVRSystem *vrSystem, renderlib::TextureManager *tm, 
		            std::map<VRControllerHand, int> handDeviceMap = {})
		: hmd(vrSystem), handDeviceMap(handDeviceMap) 
	{
		for (int i = 0; i < vr::k_unMaxTrackedDeviceCount; i++) {
			vr::TrackedDeviceClass deviceClass = vrSystem->GetTrackedDeviceClass(i);

			switch (deviceClass) {
			case vr::TrackedDeviceClass_HMD:
				hmdIndex = i;
				break;
			case vr::TrackedDeviceClass_Controller:
				//int32_t ovrHand = vrSystem->GetInt32TrackedDeviceProperty(i, vr::ETrackedDeviceProperty::Prop_ControllerRoleHint_Int32);
				//VRControllerHand hand = (ovrHand == vr::ETrackedControllerRole::TrackedControllerRole_LeftHand) ?
				//	VRControllerHand::LEFT : VRControllerHand::RIGHT;
				VRControllerHand hand;
				for (auto handIndex : handDeviceMap) {
					if (handIndex.second == i)
						hand = handIndex.first;
				}
				Controller controller(i, vrSystem, tm);
				//if (handDeviceMap.find(hand) != handDeviceMap.end())
				//	hand = (hand == VRControllerHand::LEFT) ? VRControllerHand::RIGHT : VRControllerHand::LEFT;
				//handDeviceMap[hand] = i;
				controllers[hand] = controller;
				
				break;
			}
		}
	}

	void updatePose() {
		vr::VRCompositor()->WaitGetPoses(poses, vr::k_unMaxTrackedDeviceCount, NULL, 0);
		//vr::VRCompositor()->GetLastPoses(poses, vr::k_unMaxTrackedDeviceCount, NULL, 0);

		hmd.updatePose(poses[hmdIndex]);
		for (auto pair : handDeviceMap) {
			auto hand = pair.first;
			auto device = pair.second;
			controllers[hand].updatePose(poses[device]);
		}
	}

	void updateState(vr::IVRSystem *vrSystem) {
		for (auto pair : handDeviceMap) {
			auto hand = pair.first;
			auto device = pair.second;
			vr::VRControllerState_t state;
			vrSystem->GetControllerState(device, &state, sizeof(state));
			controllers[hand].input.updateState(state);
		}
	}

};