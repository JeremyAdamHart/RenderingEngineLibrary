#pragma once

#include <openvr/openvr.h>
#include <glm/glm.hpp>
#include <Drawable.h>
#include <Texture.h>

glm::vec3 toVec3(vr::HmdVector3_t vec);
glm::vec2 toVec2(vr::HmdVector2_t vec);
glm::vec3 getTranslation(vr::HmdMatrix34_t matrix);
glm::mat3 getRotation(vr::HmdMatrix34_t matrix);
glm::mat4 toMat4(const vr::HmdMatrix44_t &hmdMat);
glm::mat4 toMat4(const vr::HmdMatrix34_t &hmdMat);

void openvrRenderModelToDrawable(renderlib::Drawable *drawable,
	vr::RenderModel_t *openvrModel, renderlib::TextureManager *texManager);

