#pragma once

#include <TemplatedGeometry.h>
#include <ElementGeometry.h>
#include <Drawable.h>

namespace renderlib {
struct SphereGeometry {
	using Indices = unsigned int;
	using Type = ElementGeometryT<Indices, glm::vec3, glm::vec3, glm::vec2>;
};

struct PlaneGeometry {
	using Type = GeometryT<glm::vec3, glm::vec3, glm::vec2>;
};

enum class Orientation {
	PositiveZ,
	NegativeZ,
	PositiveX,
	NegativeX,
	PositiveY,
	NegativeY
};

constexpr quat axisRotation(const Orientation orientation) {
	switch (orientation) {
	case Orientation::PositiveX:
		return glm::angleAxis(-glm::pi<float>()*0.5f, glm::vec3(0, 0, 1));
	case Orientation::PositiveY:
		return glm::quat();
	case Orientation::PositiveZ:
		return glm::angleAxis(glm::pi<float>()*0.5f, glm::vec3(1, 0, 0));
	case Orientation::NegativeX:
		return glm::angleAxis(glm::pi<float>()*0.5f, glm::vec3(0, 0, 1));
	case Orientation::NegativeY:
		return glm::angleAxis(glm::pi<float>(), glm::vec3(1, 0, 0));
	case Orientation::NegativeZ:
		return glm::angleAxis(glm::pi<float>()*0.5f, glm::vec3(1, 0, 0));
	}
}

struct CubeGeometry {
	using Type = GeometryT<glm::vec3, glm::vec3, glm::vec2>;
};

sptr<SphereGeometry::Type> createSphereGeometry(unsigned int azimuthResolution = 40, unsigned int altitudeResolution = 20, float orientation=1.f);
//sptr<ElementGeometry> createSphereGeometry(unsigned int azimuthResolution = 40, unsigned int altitudeResolution = 20, float orientation = 1.f);
sptr<PlaneGeometry::Type> createPlaneGeometry(Orientation orientation=Orientation::PositiveY);
sptr<CubeGeometry::Type> createCubeGeometry();

class LineDrawable : public Drawable {
private:
	glm::vec3 points[2];
public:

	void update();
	glm::vec3& a();
	glm::vec3& b();
};

}
