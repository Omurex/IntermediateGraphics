#include "Camera.h"


glm::mat4 Camera::getViewMatrix()
{
	return glm::mat4();
}


glm::mat4 Camera::getProjectionMatrix()
{
	return glm::mat4();
}


glm::mat4 Camera::ortho(float height, float aspectRatio, float nearPlane, float farPlane)
{
	return glm::mat4();
}


glm::mat4 Camera::perspective(float fov, float aspectRatio, float nearPlane, float farPlane)
{
	return glm::mat4();
}
