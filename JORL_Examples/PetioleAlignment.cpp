#include "PetioleAlignment.h"

#include <cstdio>
#include <optional>

using namespace glm;

quat rotationBetween(glm::vec3 v1, glm::vec3 v2, float minCos) {
	vec3 rotationVector = normalize(cross(v1, v2));
	vec3 rotationVectorPerpendicular = cross(rotationVector, v1);

	float sign = (dot(rotationVectorPerpendicular, v2) > 0) ?
		1.f :
		-1.f;

	float rotationCos = glm::max(
		dot(v1, v2),
		minCos);

	float halfAngleCos = glm::sign(0.5f + rotationCos * 0.5f)*sqrt(0.5f + rotationCos * 0.5f);
	float halfAngleSin = sqrt(1.f - halfAngleCos * halfAngleCos);

	return quat(halfAngleCos, sign*halfAngleSin*rotationVector);
}

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

//bx and by are on first great circle, latitude height 
std::pair<glm::vec3, std::optional<glm::vec3>> findClosestPointBetweenGreatCircleAndLatitudeLine(glm::vec3 bx, glm::vec3 by, float latitudeHeight) {
	glm::vec3 peak = normalize(bx.y*bx + by.y*by);
	std::pair<glm::vec3, std::optional<glm::vec3>> ret;

	if (peak.y <= latitudeHeight)
		ret = { peak, {} };
	else{
		vec3 center = peak * (latitudeHeight / peak.y);
		vec2 toCenter = vec2(center.x, center.z);
		float dist = sqrt(1.f - latitudeHeight * latitudeHeight - dot(toCenter, toCenter));
		vec3 toEdge = normalize(vec3(-center.z, 0, center.x))*dist;
		ret.first = center + toEdge;
		ret.second = center - toEdge;
	}

	return ret;
}

PetioleInfo petioleRotation(glm::vec3 location, glm::vec3 tangent, glm::vec3 normal, float segmentLength, float targetAngleFromVertical, float maxBendAngle, float maxTwistAngle)
{
	vec3 targetVector;
	vec3 right = normalize(cross(normal, tangent));
	
	auto twistIntersection = findClosestPointBetweenGreatCircleAndLatitudeLine(normal, right, cos(targetAngleFromVertical));
	targetVector = (!twistIntersection.second ||
		distance(twistIntersection.first, normal) <
		distance(*twistIntersection.second, normal)) ?
		twistIntersection.first :
		*twistIntersection.second;

	quat twistRotation = rotationBetween(normal, targetVector, cos(maxTwistAngle));
	normal = normalize(twistRotation * normal);

	///*
	auto intersections = findClosestPointBetweenGreatCircleAndLatitudeLine(tangent, normal, cos(targetAngleFromVertical));

	if (intersections.second) {
		targetVector = (distance(normal, intersections.first) < distance(normal, *intersections.second)) ?
			intersections.first :
			*intersections.second;
	}
	else {
		targetVector = intersections.first;
		targetVector = normalize(
			normalize(vec3(targetVector.x, 0, targetVector.z))*sin(targetAngleFromVertical) +
			vec3(0, cos(targetAngleFromVertical), 0));
	}
	//*/
	quat rotation = petioleBend(tangent, normal, targetVector, maxBendAngle);
	quat initialOrientation = mat3(cross(tangent, normal), normal, -tangent);

	float initialLambertion = glm::max(dot(normal, targetVector), 0.f);
	float u = initialLambertion;

	quat finalOrientation = rotation*initialOrientation;

	return { 
		location, 
		location + tangent*segmentLength, 
		location + tangent*segmentLength + finalOrientation*vec3(0, 0, -segmentLength), 
		finalOrientation };
}
