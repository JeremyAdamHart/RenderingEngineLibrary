#include "PetioleAlignment.h"

#include <cstdio>

using namespace glm;

PetioleInfo petioleRotation(glm::vec3 location, glm::vec3 tangent, glm::vec3 normal, float targetAngleFromVertical, float maxPetioleAngle)
{
	//Find vector specified angle from vertical in plane of vertical and normal
	glm::vec3 targetVector = normalize(
		normalize(vec3(normal.x, 0, normal.z))*sin(targetAngleFromVertical) +
		vec3(0, cos(targetAngleFromVertical), 0));

	

	vec3 tangentPlaneStart = normal;
	vec3 tangentPlaneFinish = renderlib::projectToPlane(tangent, targetVector);

	vec3 rotationVector = normalize(cross(tangentPlaneFinish - tangentPlaneStart, tangent));
	vec3 rotationPlaneStart = normalize(renderlib::projectToPlane(rotationVector, normal));
	vec3 rotationPlaneFinish = normalize(renderlib::projectToPlane(rotationVector, targetVector));

	float rotationCos = (2.f - dot(
		rotationPlaneStart - rotationPlaneFinish,
		rotationPlaneStart - rotationPlaneFinish))*0.5f;
	float rotationSin = sqrt(1.f - rotationCos * rotationCos);

	printf("rotationCos = %f\n", rotationCos);

	vec3 rotationVectorPerpendicular = cross(rotationVector, rotationPlaneStart);

	//Calculate sin sign
	float sign = (dot(rotationVectorPerpendicular, rotationPlaneFinish) > 0) ?
		1.f :
		-1.f;

	float halfAngleCos = cos(acos(rotationCos)*0.5f);
	float halfAngleSin = sqrt(1.f - halfAngleCos * halfAngleCos);
	//quat rotation(rotationCos, sign*rotationSin*rotationVector);
	quat rotation(halfAngleCos, sign*halfAngleSin*rotationVector);
	quat initialOrientation = mat3(cross(tangent, normal), normal, -tangent);
	quat finalOrientation = rotation*initialOrientation;

	printf("angle from vertical = %f\n",
		acos(dot(normalize(finalOrientation*vec3(0, 1, 0)*inverse(finalOrientation)), vec3(0, 1, 0))));

	return { location, location + tangent, location + tangent + rotation * tangent*inverse(rotation), finalOrientation };


	/*glm::vec3 targetVector =
		normalize(vec3(normal.x, 0, normal.z))*sin(targetAngleFromVertical) +
		vec3(0, cos(targetAngleFromVertical), 0);

	vec3 tangentPlaneStart = renderlib::projectToPlane(tangent, normal);
	vec3 tangentPlaneFinish = renderlib::projectToPlane(normal, targetVector);

	vec3 rotationVector = normalize(cross(tangentPlaneFinish - tangentPlaneStart, tangent));
	vec3 rotationPlaneStart = renderlib::projectToPlane(rotationVector, normal);
	vec3 rotationPlaneFinish = renderlib::projectToPlane(rotationVector, targetVector);
	
	float rotationCos = 1.f - dot(
		normalize(rotationPlaneStart - rotationPlaneFinish), 
		normalize(rotationPlaneStart - rotationPlaneFinish));
	float rotationSin = sqrt(1.f - rotationCos * rotationCos);

	vec3 rotationVectorPerpendicular = cross(rotationVector, rotationPlaneStart);

	//Calculate sin sign
	float sign = (dot(rotationVectorPerpendicular, rotationPlaneFinish) > 0) ?
		1.f :
		-1.f;
	
	quat initialOrientation = mat3(cross(tangent, normal), normal, -tangent);
	quat finalOrientation = quat(rotationCos, sign*rotationSin*rotationVector)*initialOrientation;
	*/
	 

}
