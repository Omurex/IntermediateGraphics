#pragma once
#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


class Camera
{
	public:

	glm::vec3 position;
	glm::vec3 target; // World position to look at

	float fov;
	float orthographicSize;
	bool orthographic;

	glm::mat4 getViewMatrix();
	glm::mat4 getProjectionMatrix();

	glm::mat4 ortho(float height, float aspectRatio, float nearPlane, float farPlane);
	glm::mat4 perspective(float fov, float aspectRatio, float nearPlane, float farPlane);
};