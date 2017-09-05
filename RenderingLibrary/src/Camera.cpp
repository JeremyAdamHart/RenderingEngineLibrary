#include "Camera.h"

Camera::Camera() :projection(1.f){}

glm::mat4 Camera::getCameraMatrix() const { return glm::mat4(); }
glm::mat4 Camera::getProjectionMatrix() const { return projection; }
glm::mat4 Camera::getRotationMatrix() const { return glm::mat4(); }
void Camera::setProjectionMatrix(const glm::mat4 &projMatrix){ projection = projMatrix; }
glm::vec3 Camera::getPosition() const { return glm::vec3(); }