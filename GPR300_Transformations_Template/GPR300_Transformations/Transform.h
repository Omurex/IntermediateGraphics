#pragma once
#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


struct Transform
{
	glm::vec3 position;
	glm::vec3 rotation; // Euler angles
	glm::vec3 scale;

	glm::mat4 getModelMatrix();

	Transform(glm::vec3 _position = glm::vec3(0), glm::vec3 _rotation = glm::vec3(0), glm::vec3 _scale = glm::vec3(1))
		: position(_position), rotation(_rotation), scale(_scale)
	{}

	private:
	glm::mat4 getScaleMatrix();

	glm::mat4 getXRotationMatrix();
	glm::mat4 getYRotationMatrix();
	glm::mat4 getZRotationMatrix();
	glm::mat4 getRotationMatrix();

	glm::mat4 getTranslationMatrix();
};