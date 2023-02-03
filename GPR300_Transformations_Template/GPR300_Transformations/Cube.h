#pragma once

#include "Transform.h"
#include "EW/ShapeGen.h"

#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

struct Cube
{
	Transform transform;
	Mesh* mesh = nullptr;

	Cube(vec3 dimensions)
	{
		MeshData data;
		createCube(dimensions.x, dimensions.y, dimensions.z, data);

		mesh = new Mesh(&data);
	}

	Cube(vec3 dimensions, vec3 position, vec3 rotation, vec3 scale) : Cube(dimensions)
	{
		transform.position = position;
		transform.rotation = rotation;
		transform.scale = scale;
	}


	void draw()
	{
		mesh->draw();
	}


	mat4 getModelMatrix()
	{
		return transform.getModelMatrix();
	}


	~Cube()
	{
		if (mesh != nullptr) delete mesh; 
	}
};