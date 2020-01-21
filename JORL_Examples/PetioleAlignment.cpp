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

	printf("tangentPlaneStart dot tangent = %f\n", dot(tangentPlaneStart, tangent));
	printf("tangentPlaneFinish dot tangent = %f\n", dot(tangentPlaneFinish, tangent));

	vec3 rotationVector = normalize(cross(tangentPlaneFinish - tangentPlaneStart, tangent));
	vec3 rotationPlaneStart = renderlib::projectToPlane(rotationVector, normal);
	vec3 rotationPlaneFinish =renderlib::projectToPlane(rotationVector, targetVector);

	printf("rotationPlaneStart dot rotationVector = %f\n", dot(rotationPlaneStart, rotationVector));
	printf("rotationPlaneFinish dot rotationVector = %f\n", dot(rotationPlaneFinish, rotationVector));

	printf("Length start = %f, Length finish = %f\n", length(rotationPlaneStart), length(rotationPlaneFinish));

	vec3 rotationVectorPerpendicular = cross(rotationVector, rotationPlaneStart);

	//Calculate sin sign
	float sign = (dot(rotationVectorPerpendicular, rotationPlaneFinish) > 0) ?
		1.f :
		-1.f;

	float rotationCos = dot(rotationPlaneStart, rotationPlaneFinish)/ dot(rotationPlaneStart, rotationPlaneStart);
	float halfAngleCos = glm::sign(rotationCos)*sqrt(0.5f + rotationCos*0.5f);	//cos(acos(rotationCos)*0.5f);
	float halfAngleSin = sqrt(1.f - halfAngleCos * halfAngleCos);
	//quat rotation(rotationCos, sign*rotationSin*rotationVector);
	quat rotation(halfAngleCos, sign*halfAngleSin*rotationVector);
	quat initialOrientation = mat3(cross(tangent, normal), normal, -tangent);
	quat finalOrientation = rotation*initialOrientation;

	printf("angle from vertical = %f\n",
		acos(dot(normalize(finalOrientation*vec3(0, 1, 0)), vec3(0, 1, 0))));

	return { location, location + tangent, location + tangent + rotation * tangent, finalOrientation };

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
