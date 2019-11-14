#include <stdio.h>

#include "VRContext.h"

#include "Framebuffer.h"
#include "TextureCreation.h"

namespace renderlib {

vr::IVRSystem *initVR() {

	vr::EVRInitError error = vr::VRInitError_None;
	vr::IVRSystem *vrDisplay = vr::VR_Init(&error, vr::VRApplication_Scene);

	if (error != vr::VRInitError_None)
	{
		vrDisplay = nullptr;
		printf("[Error] %s\n", VR_GetVRInitErrorAsSymbol(error));
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

VRContext::VRContext(TextureManager *texManager) : vrSystem(initVR()) {
	if (vrSystem) {
		unsigned int TEX_WIDTH = 800;
		unsigned int TEX_HEIGHT = 800;
		vrSystem->GetRecommendedRenderTargetSize(&TEX_WIDTH, &TEX_HEIGHT);

		eyeFramebuffers[vr::EVREye::Eye_Left] = createNewFramebuffer(TEX_WIDTH, TEX_HEIGHT);
		eyeFramebuffers[vr::EVREye::Eye_Right] = createNewFramebuffer(TEX_WIDTH, TEX_HEIGHT);

		if (!eyeFramebuffers[vr::EVREye::Eye_Left].addTexture(
			createTexture2D(TEX_WIDTH, TEX_HEIGHT, texManager), GL_COLOR_ATTACHMENT0)) {
			printf("FBO creation failed\n");
		}
		else if (!eyeFramebuffers[vr::EVREye::Eye_Right].addTexture(
			createTexture2D(TEX_WIDTH, TEX_HEIGHT, texManager), GL_COLOR_ATTACHMENT0)) {
			printf("FBO creation failed\n");
		}
	}
}

unsigned int VRContext::renderTargetWidth() { return eyeFramebuffers[vr::EVREye::Eye_Right].vp.width; }
unsigned int VRContext::renderTargetHeight() { return eyeFramebuffers[vr::EVREye::Eye_Right].vp.height; }

void VRContext::submitFrame(const Framebuffer& frame, vr::EVREye eye) {
	blit(frame, eyeFramebuffers[eye]);
		
	vr::Texture_t frameTexture = {
		(void*)(uintptr_t)GLuint(eyeFramebuffers[eye].getTexture(GL_COLOR_ATTACHMENT0).getID()),
		vr::TextureType_OpenGL, vr::ColorSpace_Gamma };

	vr::VRCompositor()->Submit(eye, &frameTexture);
}

Texture VRContext::getTexture(vr::EVREye eye) {
	return eyeFramebuffers[eye].getTexture(GL_COLOR_ATTACHMENT0);
}

IndexedViewportVRContext::IndexedViewportVRContext(TextureManager *texManager) : vrSystem(initVR()) {
	if (vrSystem) {
		unsigned int TEX_WIDTH = 800;
		unsigned int TEX_HEIGHT = 800;
		vrSystem->GetRecommendedRenderTargetSize(&TEX_WIDTH, &TEX_HEIGHT);
		printf("RecommendedRenderTargetSize = %d %d\n", TEX_WIDTH, TEX_HEIGHT);
		framebuffer = IndexedFramebuffer(TEX_WIDTH*2, TEX_HEIGHT);
		framebuffer.addViewport(TEX_WIDTH, TEX_HEIGHT, 0, 0);
		framebuffer.addViewport(TEX_WIDTH, TEX_HEIGHT, TEX_WIDTH, 0);

		if (!framebuffer.addTexture(
			createTexture2D(TEX_WIDTH*2, TEX_HEIGHT, texManager), GL_COLOR_ATTACHMENT0)) {
			printf("FBO creation failed\n");
		}
	}
}

unsigned int IndexedViewportVRContext::renderTargetWidth() { return framebuffer.vp.width/2; }
unsigned int IndexedViewportVRContext::renderTargetHeight() { return framebuffer.vp.height; }

void submitFrame(const Framebuffer& frame, vr::EVREye eye) {

}

void IndexedViewportVRContext::submitFrame(const IndexedFramebuffer& frame) {
	blit(frame, framebuffer);
	vr::Texture_t frameTexture = {
		(void*)(uintptr_t)GLuint(framebuffer.getTexture(GL_COLOR_ATTACHMENT0).getID()),
		vr::TextureType_OpenGL, vr::ColorSpace_Gamma };

	vr::VRTextureBounds_t leftEyeBounds = { 0.f, 0.f, 0.5f, 1.f };
	vr::VRTextureBounds_t rightEyeBounds = { 0.5f, 0.f, 1.f, 1.f };

	vr::VRCompositor()->Submit(vr::EVREye::Eye_Left, &frameTexture, &leftEyeBounds);
	vr::VRCompositor()->Submit(vr::EVREye::Eye_Right, &frameTexture, &rightEyeBounds);
}

void IndexedViewportVRContext::submitFrameDirect(const IndexedFramebuffer& frame) {
	vr::Texture_t frameTexture = {
		(void*)(uintptr_t)GLuint(frame.getTexture(GL_COLOR_ATTACHMENT0).getID()),
		vr::TextureType_OpenGL, vr::ColorSpace_Gamma };

	vr::VRTextureBounds_t leftEyeBounds = { 0.f, 0.f, 0.5f, 1.f };
	vr::VRTextureBounds_t rightEyeBounds = { 0.5f, 0.f, 1.f, 1.f };

	vr::VRCompositor()->Submit(vr::EVREye::Eye_Left, &frameTexture);	// , &leftEyeBounds);
	vr::VRCompositor()->Submit(vr::EVREye::Eye_Right, &frameTexture);	// , &rightEyeBounds);
}

Texture IndexedViewportVRContext::getTexture() {
	return framebuffer.getTexture(GL_COLOR_ATTACHMENT0);
}








}