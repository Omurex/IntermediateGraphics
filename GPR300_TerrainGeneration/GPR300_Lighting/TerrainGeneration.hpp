#pragma once
#include "EW/Mesh.h"
#include "SimplexNoise.h"
#include "CImg-v.3.2.3/CImg.h"
#include <iostream>

using namespace ew;
using namespace cimg_library;

typedef CImg<unsigned char> Image;


struct TerrainInfo
{
	int resolution;

	float width;
	float length;

	float minHeight;
	float maxHeight;


	TerrainInfo(int _resolution, float _width, float _length, float _minHeight, float _maxHeight, float _heightMapBlur = 0) :
		resolution(_resolution), width(_width), length(_length), minHeight(_minHeight), maxHeight(_maxHeight) {}
};


struct NoiseInfo
{
	std::string path;
	
	float blur;
	float redistribution; // To make flat valleys, we can raise the elevation to a power

	NoiseInfo(std::string _path, float _blur, float _redistribution) :
		path(_path), blur(_blur), redistribution(_redistribution) {}
};


float getHeight(const Image& heightMap, const NoiseInfo& noiseInfo, float uvX, float uvY, float minHeight, float maxHeight)
{
	int rValue = heightMap((int)(uvX * heightMap.width()), (int)(uvY * heightMap.height()), 0); // Red component at uv

	float portion = (float) rValue / 255.0;

	portion = glm::pow(portion, noiseInfo.redistribution);

	return (portion * (maxHeight - minHeight)) + minHeight;

}


// Resolution is how many double sets of triangles are in width and height
void generateTerrainFromHeightmap(const TerrainInfo& terrainInfo, const NoiseInfo& noiseInfo, const Image& heightMap, MeshData& meshData)
{
	meshData.vertices.clear();
	meshData.indices.clear();

	int resolution = terrainInfo.resolution;
	float width = terrainInfo.width;
	float length = terrainInfo.length;
	float minHeight = terrainInfo.minHeight;
	float maxHeight = terrainInfo.maxHeight;

	int numVertices = 4 * resolution * resolution; // * 4 for two triangles each unit
	int numIndeces = 6 * resolution * resolution; // * 6 for two sets of vertices each unit

	float halfWidth = width / 2.0f;
	float halfHeight = length / 2.0f;

	float triangleWidth = width / resolution;
	float triangleHeight = length / resolution;

	Vertex* vertices = new Vertex[numVertices];
	unsigned int* indices = new unsigned int[numIndeces];

	for (int y = 0; y < resolution - 1; y++)
	{
		int yVertOffset = y * resolution * 4; // How much needs to be added from y when indexing vertices 
		// to make it line up properly with vertices array

		int yIndexOffset = y * resolution * 6;

		for (int x = 0; x < resolution - 1; x++)
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


			glm::vec2 heightMapUV = glm::vec2((float) x / resolution, (float) y / resolution);

			glm::vec2 nextHeightMapUV = glm::vec2((float) (x + 1) / resolution, (float) (y + 1) / resolution);
			int nextRValue = heightMap((int)(nextHeightMapUV.x * heightMap.width()), (int)(nextHeightMapUV.y * heightMap.height()), 0); // Red component at uv

			int vertOffset = yVertOffset + (x * 4);
			int indexOffset = yIndexOffset + (x * 6);

			// 0,1,2,3
			float height = getHeight(heightMap, noiseInfo, heightMapUV.x, heightMapUV.y, minHeight, maxHeight);
			vertices[vertOffset + 0].position = glm::vec3(-halfWidth + (triangleWidth * x), height, -halfHeight + (triangleHeight * y));

			height = getHeight(heightMap, noiseInfo, heightMapUV.x, nextHeightMapUV.y, minHeight, maxHeight);
			vertices[vertOffset + 1].position = glm::vec3(-halfWidth + (triangleWidth * x), height, -halfHeight + (triangleHeight * (y + 1)));

			height = getHeight(heightMap, noiseInfo, nextHeightMapUV.x, nextHeightMapUV.y, minHeight, maxHeight);
			vertices[vertOffset + 2].position = glm::vec3(-halfWidth + (triangleWidth * (x + 1)), height, -halfHeight + (triangleHeight * (y + 1)));

			height = getHeight(heightMap, noiseInfo, nextHeightMapUV.x, heightMapUV.y, minHeight, maxHeight);
			vertices[vertOffset + 3].position = glm::vec3(-halfWidth + (triangleWidth * (x + 1)), height, -halfHeight + (triangleHeight * y));

			/*if (height > maxHeight * .8 || height < maxHeight * .2)
			{
				std::cout << height << std::endl;
			}*/

			// Debug
			vertices[vertOffset + 0].uv = glm::vec2(0, 0);
			vertices[vertOffset + 1].uv = glm::vec2(0, 1);
			vertices[vertOffset + 2].uv = glm::vec2(1, 1);
			vertices[vertOffset + 3].uv = glm::vec2(1, 0);

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

	// Load meshData
	meshData.vertices.assign(&vertices[0], &vertices[numVertices]);
	meshData.indices.assign(&indices[0], &indices[numIndeces]);
}


Image readHeightMap(const NoiseInfo& noiseInfo)
{
	Image heightMapImage(noiseInfo.path.c_str());

	heightMapImage.blur(noiseInfo.blur);

	//heightMapImage.display();

	return heightMapImage;
}


void createTerrain(const TerrainInfo& terrainInfo, const NoiseInfo& noiseInfo, MeshData& meshData)
{
	Image heightMap = readHeightMap(noiseInfo);
	generateTerrainFromHeightmap(terrainInfo, noiseInfo, heightMap, meshData);
}