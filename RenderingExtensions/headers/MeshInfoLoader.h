// File originally partially created by Troy Alderson for use in CPSC453 assignments. Reused with permission from Ben Stephenson
#pragma once

#define NOMINMAX
#include <glm/glm.hpp>
#include <vector>
#include "ElementGeometry.h"

namespace renderlib {

using namespace std;
using namespace glm;

class MeshInfoLoader {
public:
	MeshInfoLoader() {}
	MeshInfoLoader(const char* filename);

	bool loadModel(const char* filename);
	bool loadModelPly(const char* filename);

	void clearData();

	vec3 getCenter();
	float getBoundingRadius();
	float getBottom();

	vector<vec3> vertices, normals;
	vector<vec2> uvs;
	vector<unsigned int> indices;

protected:

private:

	vec3 max, min;
};

ElementGeometry *objToElementGeometry(char *filename);

}