#pragma once
#include "EW/Mesh.h"

using namespace ew;


// Resolution is how many double sets of triangles are in width and height
void createTerrainBase(int resolution, float width, float height, MeshData& meshData)
{
	meshData.vertices.clear();
	meshData.indices.clear();

	int numVertices = 4 * resolution * resolution; // * 4 for two triangles each unit
	int numIndeces = 6 * resolution * resolution; // * 6 for two sets of vertices each unit

	float halfWidth = width / 2.0f;
	float halfHeight = height / 2.0f;

	float triangleWidth = width / resolution;
	float triangleHeight = height / resolution;

	Vertex* vertices = new Vertex[numVertices];
	unsigned int* indices = new unsigned int[numIndeces];
	
		
	//	// front face
	//	0, 2, 1,
	//	0, 3, 2
	//};

	for (int y = 0; y < resolution; y++)
	{
		int yVertOffset = y * width * 4; // How much needs to be added from y when indexing vertices 
											// to make it line up properly with vertices array

		int yIndexOffset = y * width * 6;

		for (int x = 0; x < resolution; x++)
		{
			// Needs to go clockwise
			/*
			   1 ____ 2
				|   /|
				|  / |
				| /  |
				|/   |
			   0 ---- 3
			*/

			int vertOffset = yVertOffset + (x * 4);
			int indexOffset = yIndexOffset + (x * 6);

			// 0,1,2,3
			vertices[vertOffset + 0].position = glm::vec3(-halfWidth + (triangleWidth * x), 0, -halfHeight + (triangleHeight * y));
			vertices[vertOffset + 1].position = glm::vec3(-halfWidth + (triangleWidth * x), 0, -halfHeight + (triangleHeight * (y + 1)));
			vertices[vertOffset + 2].position = glm::vec3(-halfWidth + (triangleWidth * (x + 1)), 0, -halfHeight + (triangleHeight * (y + 1)));
			vertices[vertOffset + 3].position = glm::vec3(-halfWidth + (triangleWidth * (x + 1)), 0, -halfHeight + (triangleHeight * y));

			// 0-1-2
			indices[indexOffset + 0] = vertOffset + 0;
			indices[indexOffset + 1] = vertOffset + 1;
			indices[indexOffset + 2] = vertOffset + 2;

			// 0-2-3
			indices[indexOffset + 3] = vertOffset + 0;
			indices[indexOffset + 4] = vertOffset + 2;
			indices[indexOffset + 5] = vertOffset + 3;
		}
	}

	//Vertex vertices[4] = 
	//{
	//	//Front face
	//	{glm::vec3(-halfWidth, 0, -halfHeight), glm::vec3(0,1,0), glm::vec2(0, 1)}, //BL
	//	{glm::vec3(+halfWidth, 0, -halfHeight), glm::vec3(0,1,0), glm::vec2(1, 1)}, //BR
	//	{glm::vec3(+halfWidth, 0, +halfHeight), glm::vec3(0,1,0), glm::vec2(1, 0)}, //TR
	//	{glm::vec3(-halfWidth, 0, +halfHeight), glm::vec3(0,1,0), glm::vec2(0, 0)} //TL
	//};

	meshData.vertices.assign(&vertices[0], &vertices[numVertices]);
	meshData.indices.assign(&indices[0], &indices[numIndeces]);
}