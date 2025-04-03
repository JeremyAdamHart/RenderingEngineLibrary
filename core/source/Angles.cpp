#include <Angles.h>
#include <glm/gtc/constants.hpp>

Degrees::Degrees(float angle) : angle(angle) {}
Degrees::Degrees(Radians rad) : angle(float(rad) *180.f/ glm::pi<float>()) {}
Degrees::operator float() { return angle; }

Degrees operator+(Degrees a, Degrees b) {
	return Degrees(float(a) + float(b));
}

Degrees operator-(Degrees a, Degrees b) {
	return Degrees(float(a) - float(b));
}

Degrees operator*(Degrees a, float s) {
	return Degrees(float(a)*s);
}

Degrees operator*(float s, Degrees a) {
	return a * s;
}

Radians::Radians(float angle) : angle(angle) {}
Radians::Radians(Degrees deg) : angle(float(deg) *glm::pi<float>()/180.f) {}
Radians::operator float() { return angle; }

Radians operator+(Radians a, Radians b) {
	return Radians(float(a) + float(b));
}

Radians operator-(Radians a, Radians b) {
	return Radians(float(a) - float(b));
}

Radians operator*(Radians a, float s) {
	return Radians(float(a)*s);
}

Radians operator*(float s, Radians a) {
	return a * s;
}