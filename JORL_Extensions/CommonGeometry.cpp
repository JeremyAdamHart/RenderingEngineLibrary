#include "CommonGeometry.h"
#include <vector>

namespace renderlib {

using namespace glm;

sptr<SphereGeometry::Type> createSphereGeometry(unsigned int azimuthDivisions, unsigned int altitudeDivisions, float orientation)
{
	std::vector<vec3> positions;
	std::vector<vec3> normals;
	std::vector<vec2> uvs;
	std::vector<SphereGeometry::Indices> indices;

	auto sphereEquation = [](float u, float v){ 
		return vec3(sin(v*glm::pi<float>())*cos(u*2.f*glm::pi<float>()), -cos(v*glm::pi<float>()), -sin(v*glm::pi<float>())*sin(u*2.f*glm::pi<float>()));
	};

	float u = 0.f;
	float uStep = 1.f / float(azimuthDivisions);
	for (int i = 0; i < azimuthDivisions+1; i++) {
		float v = 0.f;
		float vStep = 1.f / float(altitudeDivisions-1);
		for (int j = 0; j < altitudeDivisions; j++) {
			positions.push_back(sphereEquation(u, v));
			normals.push_back(orientation*normalize(positions.back()));
			uvs.push_back(vec2(u, v));

			if (i < azimuthDivisions && j < altitudeDivisions-1) {
				// d---c
				// |   |
				// a---b
				
				SphereGeometry::Indices a = i * altitudeDivisions + j;
				SphereGeometry::Indices b = (orientation > 0) ?
					(i + 1) * altitudeDivisions + j :
					i *altitudeDivisions + j + 1;
				SphereGeometry::Indices c = (i+1) *altitudeDivisions + j+1;
				SphereGeometry::Indices d = (orientation > 0) ?
					i * altitudeDivisions + j+1 :
					(i+1)*altitudeDivisions + j;

				indices.push_back(a);
				indices.push_back(b);
				indices.push_back(c);

				indices.push_back(a);
				indices.push_back(c);
				indices.push_back(d);
			}
			v += vStep;
		}
		u += uStep;
	}

	return make<SphereGeometry::Type>(
		GL_TRIANGLES,
		indices.data(), indices.size(),
		positions.data(), normals.data(), uvs.data(), positions.size()
		);
}

sptr<PlaneGeometry::Type> createPlaneGeometry(Orientation orientation) {
	quat rot = axisRotation(orientation);
	std::vector<vec3> positions = {
		rot*vec3(-1, 0, -1),
		rot*vec3(1, 0, -1),
		rot*vec3(1, 0, 1),
		rot*vec3(-1, 0, 1)
	};
	std::vector<vec3> normals = {
		rot*vec3(0, 1, 0),
		rot*vec3(0, 1, 0),
		rot*vec3(0, 1, 0),
		rot*vec3(0, 1, 0)
	};
	std::vector<vec2> uvs = {
		vec2(0, 0),
		vec2(1, 0),
		vec2(1, 1),
		vec2(0, 1)
	};

	return make<PlaneGeometry::Type>(
		GL_TRIANGLE_FAN, 
		positions.data(), 
		normals.data(), 
		uvs.data(), 
		positions.size());
}

sptr<CubeGeometry::Type> createCubeGeometry() {
}
	
}