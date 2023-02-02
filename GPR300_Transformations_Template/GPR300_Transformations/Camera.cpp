#include "Camera.h"


using namespace glm;


glm::mat4 Camera::getViewMatrix(vec3 up)
{
	vec3 forward = target - position;
	vec3 right = cross(forward, up);
	vec3 newUp = cross(right, forward);

	forward *= -1; // To change to RH, flip the z axis (forward)

	mat4 viewMatrix = mat4(0);
	viewMatrix[0] = vec4(right, 0);
	viewMatrix[1] = vec4(newUp, 0);
	viewMatrix[2] = vec4(forward, 0);
	viewMatrix[3][3] = 1;

	viewMatrix = transpose(viewMatrix);

	return viewMatrix;
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
