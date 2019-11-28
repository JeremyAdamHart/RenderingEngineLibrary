#include "VRTools.h"
#include <TextureCreation.h>
#include <TextureMat.h>
#include <ShadedMat.h>
#include <TemplatedGeometry.h>

using namespace renderlib;

void openvrRenderModelToDrawable(Drawable *drawable, 
	vr::RenderModel_t *openvrModel, TextureManager *texManager)
{

	//Extract data into individual arrays
	vec3 *positions = new vec3[openvrModel->unVertexCount];
	vec3 *normals = new vec3[openvrModel->unVertexCount];
	vec2 *texCoords = new vec2[openvrModel->unVertexCount];

	for (int i = 0; i < openvrModel->unVertexCount; i++) {
		positions[i] = toVec3(openvrModel->rVertexData[i].vPosition);
		normals[i] = toVec3(openvrModel->rVertexData[i].vNormal);
		texCoords[i] = { openvrModel->rVertexData[i].rfTextureCoord[0],
						openvrModel->rVertexData[i].rfTextureCoord[1] };
	}

	unsigned int *indices = new unsigned int[openvrModel->unTriangleCount * 3];
	for (int i = 0; i < openvrModel->unTriangleCount * 3; i++) {
		indices[i] = openvrModel->rIndexData[i];
	}

	auto geom = new TexNormalIndexGeometry(GL_TRIANGLES, 
		indices, openvrModel->unTriangleCount*3, 
		positions, normals, texCoords, openvrModel->unVertexCount);

	vr::RenderModel_TextureMap_t *openvrTex;
	vr::EVRRenderModelError error;
	do {
		error = vr::VRRenderModels()->LoadTexture_Async(
			openvrModel->diffuseTextureId, &openvrTex);
	}while (error == vr::VRRenderModelError_Loading);

	Texture tex = createTexture2D(
		TexInfo(GL_TEXTURE_2D, { openvrTex->unWidth, openvrTex->unHeight },
			0, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE),
		texManager, (unsigned char*)openvrTex->rubTextureMapData);

	drawable->addMaterial(new TextureMat(tex));
	drawable->addMaterial(new ShadedMat(0.7, 0.7, 0.5, 0.3));
	drawable->setGeometryContainer(geom);

	//Cleanup
	delete[] positions;
	delete[] normals;
	delete[] texCoords;
	delete[] indices;
	vr::VRRenderModels()->FreeTexture(openvrTex);
}

vec3 toVec3(vr::HmdVector3_t vec) {
	return vec3(vec.v[0], vec.v[1], vec.v[2]);
}

vec2 toVec2(vr::HmdVector2_t vec) {
	return vec2(vec.v[0], vec.v[1]);
}

mat4 toMat4(const vr::HmdMatrix44_t &hmdMat) {
	return{
		hmdMat.m[0][0], hmdMat.m[1][0], hmdMat.m[2][0], hmdMat.m[3][0],
		hmdMat.m[0][1], hmdMat.m[1][1], hmdMat.m[2][1], hmdMat.m[3][1],
		hmdMat.m[0][2], hmdMat.m[1][2], hmdMat.m[2][2], hmdMat.m[3][2],
		hmdMat.m[0][3], hmdMat.m[1][3], hmdMat.m[2][3], hmdMat.m[3][3] };
}

mat4 toMat4(const vr::HmdMatrix34_t &hmdMat) {
	return{
		hmdMat.m[0][0], hmdMat.m[1][0], hmdMat.m[2][0], 0.f,
		hmdMat.m[0][1], hmdMat.m[1][1], hmdMat.m[2][1], 0.f,
		hmdMat.m[0][2], hmdMat.m[1][2], hmdMat.m[2][2], 0.f,
		hmdMat.m[0][3], hmdMat.m[1][3], hmdMat.m[2][3], 1.f };
}

vec3 getTranslation(vr::HmdMatrix34_t matrix) {
	return{ matrix.m[0][3], matrix.m[1][3], matrix.m[2][3] };
}
mat3 getRotation(vr::HmdMatrix34_t hmdMat) {
	return mat3(hmdMat.m[0][0], hmdMat.m[1][0], hmdMat.m[2][0],
		hmdMat.m[0][1], hmdMat.m[1][1], hmdMat.m[2][1],
		hmdMat.m[0][2], hmdMat.m[1][2], hmdMat.m[2][2]);
}