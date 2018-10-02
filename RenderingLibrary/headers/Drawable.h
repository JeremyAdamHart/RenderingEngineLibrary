#pragma once

#include "Material.h"
#include "GLGeometry.h"
#include <glm/glm.hpp>
#include "Object.h"
#include <map>
#include <memory>

namespace renderlib {

class Drawable : public virtual Object{
protected:
	std::map<int, std::shared_ptr<Material>> material;
	std::shared_ptr<GLGeometryContainer> geometry;
	glm::vec3 scale;

public:
	Drawable(GLGeometryContainer *geometry, Material *material,
		glm::vec3 position=glm::vec3(0.f), glm::quat orientation=glm::quat());		//Only valid for objects allocated to heap
	Drawable(GLGeometryContainer *geometry, glm::vec3 position = glm::vec3(0.f), glm::quat orientation = glm::quat());

	Drawable(std::shared_ptr<GLGeometryContainer> geometry, std::shared_ptr<Material> material,
		glm::vec3 position = glm::vec3(0.f), glm::quat orientation = glm::quat());		//Only valid for objects allocated to heap
	Drawable(std::shared_ptr<GLGeometryContainer> geometry, glm::vec3 position = glm::vec3(0.f), glm::quat orientation = glm::quat());

	Drawable(glm::vec3 position = glm::vec3(0.f), glm::quat orientation = glm::quat());

	bool loadUniforms(int type, GLint *uniformLocations) const;

	virtual glm::mat4 getTransform() const;

	std::shared_ptr<Material> getMaterial(int type);
	std::shared_ptr<GLGeometryContainer> getGeometryPtr(){ return geometry; }

	void setPosition(glm::vec3 position);
	void setOrientation(glm::quat orientation);
	void setScale(glm::vec3 scale);

	void addMaterial(Material* newMaterial);
	bool removeMaterial(int type);

	void deleteMaterialsAndGeometry();
	void setGeometryContainer(GLGeometryContainer* newGeometry) { geometry = std::shared_ptr<GLGeometryContainer>(newGeometry); }
	GLGeometryContainer &getGeometry()  { return *geometry; }
};

}