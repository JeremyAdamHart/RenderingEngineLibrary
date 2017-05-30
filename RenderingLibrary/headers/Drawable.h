#pragma once

#include "Material.h"
#include "GLGeometry.h"
#include <glm/glm.hpp>
#include "Object.h"
#include <map>

class Drawable : public virtual Object{
protected:
	std::map<int, Material*> material;
	GLGeometryContainer *geometry;

public:
	Drawable(Material *material, GLGeometryContainer *geometry,
		glm::vec3 position=glm::vec3(0.f), glm::quat &orientation=glm::quat());

	bool loadUniforms(int type, GLint *uniformLocations) const;

	Material *getMaterial(int type);
	GLGeometryContainer *getGeometryPtr(){ return geometry; }

	void addMaterial(Material* newMaterial);
	bool removeMaterial(int type);

	void setGeometryContainer(GLGeometryContainer* newGeometry) { geometry = newGeometry; }
	const GLGeometryContainer &getGeometry() const { return *geometry; }
};