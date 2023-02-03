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

	float nearPlane;
	float farPlane;

	Camera(glm::vec3 _position, glm::vec3 _target, float _fov = 60, float _orthographicSize = 20, bool _orthographic = false, float _nearPlane = 0, float _farPlane = 100)
		: position(_position), target(_target), fov(_fov), orthographicSize(_orthographicSize), orthographic(_orthographic), nearPlane(_nearPlane), farPlane(_farPlane)
	{}

	glm::mat4 getViewMatrix(glm::vec3 up = glm::vec3(0, 1, 0));
	glm::mat4 getProjectionMatrix(float aspectRatio);

	glm::mat4 ortho(float height, float aspectRatio, float nearPlane, float farPlane);
	glm::mat4 perspective(float fov, float aspectRatio, float nearPlane, float farPlane);
};