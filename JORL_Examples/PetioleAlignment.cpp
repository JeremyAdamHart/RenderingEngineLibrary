#include "PetioleAlignment.h"

#include <cstdio>

using namespace glm;

quat petioleBend(glm::vec3 tangent, glm::vec3 normal, glm::vec3 targetVector, float maxAngle) {
	float minCos = cos(maxAngle);

	vec3 tangentPlaneStart = normal;
	vec3 tangentPlaneFinish = renderlib::projectToPlane(tangent, targetVector);

	vec3 rotationVector = normalize(cross(tangentPlaneFinish - tangentPlaneStart, tangent));
	vec3 rotationPlaneStart = renderlib::projectToPlane(rotationVector, normal);
	vec3 rotationPlaneFinish = renderlib::projectToPlane(rotationVector, targetVector);

	vec3 rotationVectorPerpendicular = cross(rotationVector, rotationPlaneStart);

	//Calculate sin sign
	float sign = (dot(rotationVectorPerpendicular, rotationPlaneFinish) > 0) ?
		1.f :
		-1.f;

	float rotationCos = glm::max(
		dot(rotationPlaneStart, rotationPlaneFinish) / dot(rotationPlaneStart, rotationPlaneStart),
		minCos);

	float halfAngleCos = glm::sign(0.5f + rotationCos * 0.5f)*sqrt(0.5f + rotationCos * 0.5f);
	float halfAngleSin = sqrt(1.f - halfAngleCos * halfAngleCos);
	
	return quat(halfAngleCos, sign*halfAngleSin*rotationVector);
}

PetioleInfo petioleRotation(glm::vec3 location, glm::vec3 tangent, glm::vec3 normal, float targetAngleFromVertical, float maxPetioleAngle)
{
	//Find vector specified angle from vertical in plane of vertical and normal
	/*glm::vec3 targetVector = normalize(
		normalize(vec3(normal.x, 0, normal.z))*sin(targetAngleFromVertical) +
		vec3(0, cos(targetAngleFromVertical), 0));
		*/
	vec3 targetVector;
	vec3 peak = normalize(tangent.y*tangent + normal.y*normal);
	if (peak.y < cos(targetAngleFromVertical)) {
		targetVector = normalize(peak);
		targetVector = normalize(
			normalize(vec3(targetVector.x, 0, targetVector.z))*sin(targetAngleFromVertical) +
			vec3(0, cos(targetAngleFromVertical), 0));
	}
	else {
		float height = cos(targetAngleFromVertical);
		vec3 center = peak * (height / peak.y);
		vec2 toCenter = vec2(center.x, center.z);
		float dist = sqrt(1.f - height*height - dot(toCenter, toCenter));
		vec3 toEdge = normalize(vec3(-center.z, 0, center.x))*dist;
		targetVector = (distance(normal, center + toEdge) > distance(normal, center - toEdge)) ?
			targetVector = center - toEdge :
			targetVector = center + toEdge;
	}
	//targetVector = angleAxis(maxPetioleAngle, vec3(0, 1, 0))*targetVector;
	/*

	vec3 peak = normalize(tangent.y*tangent + normal.y*normal);
			if (peak.y < cos(angleFromVertical)) {
				yellowPoints.push_back((tangent.y*tangent + normal.y*normal));
				yellowPoints.push_back(2.f*yellowPoints.back());
			}
			else {
				float height = cos(angleFromVertical);
				vec3 center = peak * (height / peak.y);
				vec2 toCenter = vec2(center.x, center.z);
				float dist = sqrt(sin(angleFromVertical)*sin(angleFromVertical) - dot(toCenter, toCenter));
				vec3 toEdge = normalize(vec3(-center.z, 0, center.x))*dist;
				yellowPoints.push_back(center + toEdge);
				yellowPoints.push_back(yellowPoints.back()*2.f);

				yellowPoints.push_back(center - toEdge);
				yellowPoints.push_back(yellowPoints.back()*2.f);
			}
			yellowGeometry->loadBuffers(yellowPoints.data(), yellowPoints.size());



	////////////////////
	vec3 tangentPlaneStart = normal;
	vec3 tangentPlaneFinish = renderlib::projectToPlane(tangent, targetVector);

	vec3 rotationVector = normalize(cross(tangentPlaneFinish - tangentPlaneStart, tangent));
	vec3 rotationPlaneStart = renderlib::projectToPlane(rotationVector, normal);
	vec3 rotationPlaneFinish =renderlib::projectToPlane(rotationVector, targetVector);

	vec3 rotationVectorPerpendicular = cross(rotationVector, rotationPlaneStart);

	//Calculate sin sign
	float sign = (dot(rotationVectorPerpendicular, rotationPlaneFinish) > 0) ?
		1.f :
		-1.f;

	float rotationCos = dot(rotationPlaneStart, rotationPlaneFinish)/ dot(rotationPlaneStart, rotationPlaneStart);
	float halfAngleCos = glm::sign(0.5f + rotationCos*0.5f)*sqrt(0.5f + rotationCos*0.5f);	//cos(acos(rotationCos)*0.5f);
	float halfAngleSin = sqrt(1.f - halfAngleCos * halfAngleCos);
	//quat rotation(rotationCos, sign*rotationSin*rotationVector);
	quat rotation(halfAngleCos, sign*halfAngleSin*rotationVector);
	*/

	quat rotation = petioleBend(tangent, normal, targetVector, glm::pi<float>());
	quat initialOrientation = mat3(cross(tangent, normal), normal, -tangent);

	

	float initialLambertion = glm::max(dot(normal, targetVector), 0.f);
	float u = initialLambertion;

	quat finalOrientation = rotation*initialOrientation;

	return { location, location + tangent, location + tangent + finalOrientation*vec3(0, 0, -1), finalOrientation };
}
