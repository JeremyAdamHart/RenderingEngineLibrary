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

sptr<CylinderGeometry::Type> createCylinderGeometry(glm::vec3 start, glm::vec3 end, float radius, unsigned int resolution)
{
	float cylinderLength = length(start - end);
	vec3 by = (start - end)/cylinderLength;
	vec3 bx = cross(by, vec3(0, 0, 1));
	if (length(bx) < 0.1)
		bx = cross(by, vec3(1, 0, 0));
	vec3 bz = normalize(cross(bx, by));
	bx = normalize(cross(by, bz));

	std::vector<vec3> vertices;
	std::vector<vec3> normals;
	std::vector<vec2> uvs;
	//Side faces
	for (int i = 0; i < resolution; i++) {
		float u0 = float(i) / float(resolution);
		float u1 = float(i+1) / float(resolution);
		vec3 circleVec0 = radius*(std::cosf(u0*2.f*glm::pi<float>())*bz + sin(u0*2.f*glm::pi<float>())*bx);
		vec3 circleVec1 = radius*(std::cosf(u1*2.f*glm::pi<float>())*bz + sin(u1*2.f*glm::pi<float>())*bx);
		vec3 p00 = start + circleVec0;
		vec3 p10 = start + circleVec1;
		vec3 p01 = end + circleVec0;
		vec3 p11 = end + circleVec1;

		vertices.push_back(p00);
		vertices.push_back(p10);
		vertices.push_back(p11);
		normals.push_back(normalize(circleVec0));
		normals.push_back(normalize(circleVec1));
		normals.push_back(normalize(circleVec1));
		uvs.push_back(vec2(u0, 0.f));
		uvs.push_back(vec2(u1, 0.f));
		uvs.push_back(vec2(u1, 1.f));
		
		vertices.push_back(p00);
		vertices.push_back(p11);
		vertices.push_back(p01);
		normals.push_back(normalize(circleVec0));
		normals.push_back(normalize(circleVec1));
		normals.push_back(normalize(circleVec0));
		uvs.push_back(vec2(u0, 0.f));
		uvs.push_back(vec2(u1, 1.f));
		uvs.push_back(vec2(u0, 1.f));

		vertices.push_back(start);
		vertices.push_back(p10);
		vertices.push_back(p00);
		normals.push_back(-by);
		normals.push_back(-by);
		normals.push_back(-by);
		uvs.push_back(vec2(u0, 0.f));
		uvs.push_back(vec2(u1, 0.f));
		uvs.push_back(vec2(u0, 0.f));

		vertices.push_back(end);
		vertices.push_back(p01);
		vertices.push_back(p11);
		normals.push_back(by);
		normals.push_back(by);
		normals.push_back(by);
		uvs.push_back(vec2(u0, 1.f));
		uvs.push_back(vec2(u1, 1.f));
		uvs.push_back(vec2(u1, 1.f));
	}

	return make<CylinderGeometry::Type>(GL_TRIANGLES, vertices.data(), normals.data(), uvs.data(), vertices.size());
}

sptr<CubeGeometry::Type> createCubeGeometry() {
	return nullptr;
}
	
}