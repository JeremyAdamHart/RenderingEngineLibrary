#pragma once
#include <map>
#include <openvr/openvr.h>

#include "Framebuffer.h"
#include "Texture.h"

namespace renderlib {

vr::IVRSystem *initVR();

class VRContext {

	std::map<vr::EVREye, Framebuffer> eyeFramebuffers;

public:
	vr::IVRSystem *vrSystem;
	
	VRContext(TextureManager* texManager);

	unsigned int renderTargetWidth();
	unsigned int renderTargetHeight();
	
	Texture getTexture(vr::EVREye eye);

	void submitFrame(const Framebuffer& frame, vr::EVREye eye);

};


}