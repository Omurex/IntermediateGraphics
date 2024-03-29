#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdio.h>
#include <vector>

#include <time.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "EW/Shader.h"
#include "EW/EwMath.h"
#include "EW/Camera.h"
#include "EW/Mesh.h"
#include "EW/Transform.h"
#include "EW/ShapeGen.h"

#include "TerrainGeneration.hpp"

void processInput(GLFWwindow* window);
void resizeFrameBufferCallback(GLFWwindow* window, int width, int height);
void keyboardCallback(GLFWwindow* window, int keycode, int scancode, int action, int mods);
void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void mousePosCallback(GLFWwindow* window, double xpos, double ypos);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

float lastFrameTime;
float deltaTime;

int SCREEN_WIDTH = 1080;
int SCREEN_HEIGHT = 720;

double prevMouseX;
double prevMouseY;
bool firstMouseInput = false;

/* Button to lock / unlock mouse
* 1 = right, 2 = middle
* Mouse will start locked. Unlock it to use UI
* */
const int MOUSE_TOGGLE_BUTTON = 1;
const float MOUSE_SENSITIVITY = 0.1f;
const float CAMERA_MOVE_SPEED = 15.0f;
const float CAMERA_ZOOM_SPEED = 3.0f;

Camera camera((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT);

glm::vec3 bgColor = glm::vec3(0);

bool wireFrame = false;


//Initialize shape transforms
ew::Transform cubeTransform;
ew::Transform sphereTransform;
ew::Transform planeTransform;
ew::Transform cylinderTransform;
ew::Transform terrainTransform;


ew::MeshData cubeMeshData;
ew::MeshData sphereMeshData;
ew::MeshData cylinderMeshData;
ew::MeshData planeMeshData;
ew::MeshData terrainMeshData;

ew::Mesh cubeMesh;
ew::Mesh sphereMesh;
ew::Mesh planeMesh;
ew::Mesh cylinderMesh;
ew::Mesh terrainMesh;


std::vector<glm::vec3> terrainColArray =
{
	glm::vec3(0, 0.031, 0.678), // Deep Water
	glm::vec3(0, 0.608, 0.961), // Water
	glm::vec3(1, 0.945, 0.475), // Sand
	glm::vec3(0.631, 0.427, 0.192), // Light Dirt
	glm::vec3(0, 0.431, 0.027), // Dark Grass
	glm::vec3(0.329, 0.878, 0.349), // Grass
	glm::vec3(0.627, 0.71, 0.467), // Dead Grass
	glm::vec3(0.361, 0.361, 0.361), // Dark Rock
	glm::vec3(1, 1, 1) // Snow
};


std::vector<float> terrainColThresholds =
{
	.04, // Deep Water
	.10, // Water
	.18, // Sand
	.28, // Light Dirt
	.4, // Dark Grass
	.5, // Grass
	.55, // Dead Grass
	.6, // Dark Rock
	1 // Snow
};

const int MAX_TERRAIN_COLORS = 32;

float terrainBlendThreshold = .06f;

int terrainResolution = 1000;

float terrainWidth = 1000;
float terrainLength = 1000;

float localMinHeight = -20;
float localMaxHeight = 120;

float heightmapBlurAmount = 3;
float heightmapRedistribution = 4;

float terrainNoiseInfluence = .4;


struct GeneralLight 
{
	ew::Transform transform;

	glm::vec3 color = glm::vec3(1);
	float intensity = 1;


	GeneralLight()
	{
		transform.scale = glm::vec3(.5);
		transform.position = glm::vec3(0, 5, 0);
	}
};


struct DirectionalLight
{
	glm::vec3 direction = glm::vec3(0, -1, 0);

	glm::vec3 color = glm::vec3(1, 1, 1);
	float intensity = 1;
};


struct PointLight
{
	ew::Transform transform;

	glm::vec3 color = glm::vec3(1, 0, 0);
	float intensity = 1;

	float constantCoefficient = .3;
	float linearCoefficient = .3;
	float quadraticCoefficient = .3;


	PointLight()
	{
		transform.scale = glm::vec3(.6);
		transform.position = glm::vec3(2, 0, 2);
	}
};


struct SpotLight
{
	ew::Transform transform;
	glm::vec3 color = glm::vec3(0, 0, 1);
	glm::vec3 direction = glm::vec3(0, -1, 0);
	float intensity = 1;

	float penumbraAngle = 32; // Inner angle where intensity is max
	float umbraAngle = 39; // Outer angle where intensity reaches 0

	float attenuationExponent = 1; // Controls interpolation between penumbra and umbra lighting intensity


	SpotLight()
	{
		transform.scale = glm::vec3(.25);
		transform.position = glm::vec3(1, 5, 1);
	}
};


struct Material
{
	glm::vec3 color;

	// Between 0 and 1
	float ambientCoefficient;
	float diffuseCoefficient;
	float specularCoefficient;

	float shininess;
};


std::vector<GeneralLight> generalLights(0);
std::vector<DirectionalLight> directionalLights(1);
std::vector<PointLight> pointLights(0);
std::vector<SpotLight> spotLights(0);

Material material;


GLuint createTexture(const char* filePath, GLuint textureNum)
{
	int width, height, numComponents;
	unsigned char* textureData = stbi_load(filePath, &width, &height, &numComponents, 0);

	if (textureData == nullptr) return -1;

	// Generate a texture name
	GLuint texture;
	glGenTextures(1, &texture);

	glActiveTexture(textureNum);

	// Bind our name to GL_TEXTURE_2D to make it a 2D texture
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	return texture;
}

   

void generateTerrain()
{
	terrainMeshData.indices.clear();
	terrainMeshData.vertices.clear();

	TerrainInfo terrainInfo = TerrainInfo(terrainResolution, terrainWidth, terrainLength, localMinHeight, localMaxHeight);
	NoiseInfo noiseInfo = NoiseInfo("TerrainGenerationImages/TerrainGenerationNoise.png", heightmapBlurAmount, heightmapRedistribution);

	createTerrain(terrainInfo, noiseInfo, terrainMeshData);

	terrainMesh.initialize(&terrainMeshData);
}



void drawScene(Shader &shader, glm::mat4 view, glm::mat4 projection, float time)
{
	//Draw
	shader.use();
	shader.setMat4("_Projection", projection);
	shader.setMat4("_View", view);

	shader.setVec3("_ViewerPosition", camera.getPosition());

	shader.setVec3("_Material.color", material.color);
	shader.setFloat("_Material.ambientCoefficient", material.ambientCoefficient);
	shader.setFloat("_Material.diffuseCoefficient", material.diffuseCoefficient);
	shader.setFloat("_Material.specularCoefficient", material.specularCoefficient);
	shader.setFloat("_Material.shininess", material.shininess);

	shader.setInt("_NumGeneralLights", generalLights.size());
	shader.setInt("_NumDirectionalLights", directionalLights.size());
	shader.setInt("_NumPointLights", pointLights.size());
	shader.setInt("_NumSpotLights", spotLights.size());

	//shader.setFloat("_Time", time * .05f);
	shader.setFloat("_Time", 0);

	//Set general lighting uniforms
	for (size_t i = 0; i < generalLights.size(); i++)
	{
		shader.setVec3("_GeneralLights[" + std::to_string(i) + "].position", generalLights[i].transform.position);
		shader.setFloat("_GeneralLights[" + std::to_string(i) + "].intensity", generalLights[i].intensity);
		shader.setVec3("_GeneralLights[" + std::to_string(i) + "].color", generalLights[i].color);
	}

	//Set directional lighting uniforms
	for (size_t i = 0; i < directionalLights.size(); i++)
	{
		shader.setVec3("_DirectionalLights[" + std::to_string(i) + "].direction", glm::normalize(directionalLights[i].direction));
		shader.setFloat("_DirectionalLights[" + std::to_string(i) + "].intensity", directionalLights[i].intensity);
		shader.setVec3("_DirectionalLights[" + std::to_string(i) + "].color", directionalLights[i].color);
	}

	//Set point lighting uniforms
	for (size_t i = 0; i < pointLights.size(); i++)
	{
		shader.setVec3("_PointLights[" + std::to_string(i) + "].position", pointLights[i].transform.position);
		shader.setFloat("_PointLights[" + std::to_string(i) + "].intensity", pointLights[i].intensity);
		shader.setVec3("_PointLights[" + std::to_string(i) + "].color", pointLights[i].color);

		shader.setFloat("_PointLights[" + std::to_string(i) + "].constantCoefficient", pointLights[i].constantCoefficient);
		shader.setFloat("_PointLights[" + std::to_string(i) + "].linearCoefficient", pointLights[i].linearCoefficient);
		shader.setFloat("_PointLights[" + std::to_string(i) + "].quadraticCoefficient", pointLights[i].quadraticCoefficient);
	}

	//Set spot lighting uniforms
	for (size_t i = 0; i < spotLights.size(); i++)
	{
		shader.setVec3("_SpotLights[" + std::to_string(i) + "].position", spotLights[i].transform.position);
		shader.setVec3("_SpotLights[" + std::to_string(i) + "].color", spotLights[i].color);
		shader.setFloat("_SpotLights[" + std::to_string(i) + "].intensity", spotLights[i].intensity);
		shader.setVec3("_SpotLights[" + std::to_string(i) + "].direction", glm::normalize(spotLights[i].direction));

		float minAngleCos = glm::cos(glm::radians(spotLights[i].penumbraAngle));
		float maxAngleCos = glm::cos(glm::radians(spotLights[i].umbraAngle));

		shader.setFloat("_SpotLights[" + std::to_string(i) + "].minAngleCos", minAngleCos);
		shader.setFloat("_SpotLights[" + std::to_string(i) + "].maxAngleCos", maxAngleCos);

		shader.setFloat("_SpotLights[" + std::to_string(i) + "].attenuationExponent", spotLights[i].attenuationExponent);
	}


	////Draw cube
	//shader.setMat4("_Model", cubeTransform.getModelMatrix());
	//cubeMesh.draw();

	////Draw sphere
	//shader.setMat4("_Model", sphereTransform.getModelMatrix());
	//sphereMesh.draw();

	////Draw cylinder
	//shader.setMat4("_Model", cylinderTransform.getModelMatrix());
	//cylinderMesh.draw();

	////Draw plane
	//shader.setMat4("_Model", planeTransform.getModelMatrix());
	//planeMesh.draw();

	shader.setMat4("_Model", terrainTransform.getModelMatrix());
	terrainMesh.draw();
}








int main() {
	if (!glfwInit()) {
		printf("glfw failed to init");
		return 1;
	}

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Lighting", 0, 0);
	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK) {
		printf("glew failed to init");
		return 1;
	}

	glfwSetFramebufferSizeCallback(window, resizeFrameBufferCallback);
	glfwSetKeyCallback(window, keyboardCallback);
	glfwSetScrollCallback(window, mouseScrollCallback);
	glfwSetCursorPosCallback(window, mousePosCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);

	//Hide cursor
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Setup UI Platform/Renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	//Dark UI theme.
	ImGui::StyleColorsDark();

	//Used to draw shapes. This is the shader you will be completing.
	Shader litShader("shaders/defaultLit.vert", "shaders/defaultLit.frag");

	//Used to draw light sphere
	Shader unlitShader("shaders/defaultLit.vert", "shaders/unlit.frag");

	// Used for shadow mapping / depth
	Shader depthShader("shaders/depthOnly.vert", "shaders/depthOnly.frag");

	// Used for coloring triangles based on height
	Shader terrainShader("shaders/terrainShader.vert", "shaders/terrainShader.frag");

	// Debug
	Shader debugShader("shaders/debug.vert", "shaders/debug.frag");

	ew::createCube(1.0f, 1.0f, 1.0f, cubeMeshData);
	ew::createSphere(0.5f, 64, sphereMeshData);
	ew::createCylinder(1.0f, 0.5f, 64, cylinderMeshData);
	ew::createPlane(1.0f, 1.0f, planeMeshData);



	terrainTransform.position = glm::vec3(0, -20, 0);
	generateTerrain();



	cubeMesh.initialize(&cubeMeshData);
	sphereMesh.initialize(&sphereMeshData);
	planeMesh.initialize(&planeMeshData);
	cylinderMesh.initialize(&cylinderMeshData);

	//Enable back face culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	//Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	//Enable depth testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	cubeTransform.position = glm::vec3(-2.0f, 0.0f, 0.0f);
	sphereTransform.position = glm::vec3(0.0f, 0.0f, 0.0f);

	planeTransform.position = glm::vec3(0.0f, -1.0f, 0.0f);
	planeTransform.scale = glm::vec3(10.0f);

	cylinderTransform.position = glm::vec3(2.0f, 0.0f, 0.0f);

	material.color = glm::vec3(1);
	material.ambientCoefficient = .1f;
	material.diffuseCoefficient = .5f;
	material.specularCoefficient = .5f;
	material.shininess = 8;

	//GLuint texture = createTexture("PavingStones070_1K_Color.png", GL_TEXTURE0);
	//GLuint texture = createTexture("terrainTexture.png", GL_TEXTURE0);
	GLuint texture = createTexture("TerrainGenerationImages/TerrainTexture.png", GL_TEXTURE0);
	GLuint noiseTexture = createTexture("TerrainGenerationImages/TerrainHeightmapNoise.png", GL_TEXTURE1);

	//GLuint noise = createTexture("noiseTexture.png", GL_TEXTURE1);

	//GLuint texture = createTexture("TempTexture.png");
	
	//glActiveTexture(GL_TEXTURE0);
	//litShader.setInt("_ColorTexture", 0);

	terrainShader.setInt("_Texture", 0);
	terrainShader.setInt("_NoiseTexture", 1);


	/*glActiveTexture(GL_TEXTURE1);
	litShader.setInt("_NoiseTexture", 1);*/

	//pointLights[1].transform.position = glm::vec3(-2, 1, -2);
	//pointLights[1].color = glm::vec3(0, 1, 0);




	// DEPTH STUFF
	
	const int depthInt = 5;
	 
	// Make depthBuffer fbo
	unsigned int depthFBO;
	glGenFramebuffers(1, &depthFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
	glActiveTexture(GL_TEXTURE0 + depthInt);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER));


	// Make depth buffer
	unsigned int depthBuffer;
	glGenTextures(1, &depthBuffer);
	glActiveTexture(GL_TEXTURE0 + depthInt); // Set active then bind
	glBindTexture(GL_TEXTURE_2D, depthBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, 2048, 2048, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthBuffer, 0);

	glEnable(GL_DEPTH_TEST);
	// End make depth buffer

	glBindFramebuffer(GL_FRAMEBUFFER, 0); // Put back to backbuffer (which draws automatically)


	ew::MeshData rectangleMeshData;
	ew::createQuad(2, 2, rectangleMeshData);

	ew::Mesh rectangle = ew::Mesh(&rectangleMeshData);


	while (!glfwWindowShouldClose(window)) {
		processInput(window);
		glClearColor(bgColor.r, bgColor.g, bgColor.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		float time = (float)glfwGetTime();
		deltaTime = time - lastFrameTime;
		lastFrameTime = time;

		//UPDATE
		//cubeTransform.rotation.x += deltaTime;

		
		// Draw depth to shadow map from light's pov
		glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, 2048, 2048);

		//glm::mat4 lightView = glm::lookAt(-directionalLights[0].direction * 7.0f, glm::vec3(0), glm::vec3(0, 1, 0));

		//float width = 10;
		//float right = width * 0.5f;
		//float left = -right;
		//float top = 10 * 0.5f;
		//float bottom = -top;
		//glm::mat4 lightProj = glm::ortho(left, right, bottom, top, 0.001f, 30.0f);

		//glCullFace(GL_FRONT);
		//drawScene(depthShader, lightView, lightProj, time);

		GLint programIndex = 0;
		glGetIntegerv(GL_CURRENT_PROGRAM, &programIndex);

		terrainShader.setVec3("_ModelWorldPos", terrainTransform.position);
		terrainShader.setFloat("_LocalMinHeight", localMinHeight);
		terrainShader.setFloat("_LocalMaxHeight", localMaxHeight);

		int numElements = terrainColArray.size(); // Can use for both colorArray and thresholds because they should always match

		terrainShader.setInt("_NumLoadedTerrainColors", numElements);
		terrainShader.setFloat("_TerrainColorBlendThreshold", terrainBlendThreshold);
		terrainShader.setFloat("_TerrainNoiseInfluence", terrainNoiseInfluence);

		terrainShader.setVec2("_TerrainDimensions", glm::vec2(terrainWidth, terrainLength));

		glUniform3fv(glGetUniformLocation(programIndex, "_TerrainColorArray"), numElements, &terrainColArray[0].x);
		glUniform1fv(glGetUniformLocation(programIndex, "_TerrainColorThresholds"), numElements, &terrainColThresholds[0]);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

		glCullFace(GL_BACK);
		drawScene(terrainShader, camera.getViewMatrix(), camera.getProjectionMatrix(), time);



		//Draw UI
		ImGui::Begin("Settings");
		ImGui::BeginTabBar("TabBar");

		if (ImGui::BeginTabItem("Terrain Generation"))
		{
			ImGui::BeginTabBar("");
			if (ImGui::BeginTabItem("Terrain Info"))
			{
				ImGui::DragFloat3("Terrain Position", &terrainTransform.position.x, .1);
				ImGui::SliderFloat("Terrain Width", &terrainWidth, .01, 5000);
				ImGui::SliderFloat("Terrain Length", &terrainLength, .01, 5000);
				ImGui::SliderInt("Terrain Resolution", &terrainResolution, 1, 4000);

				if (ImGui::Button("Regenerate Terrain"))
				{
					generateTerrain();
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Heightmap Info"))
			{
				ImGui::SliderFloat("Blur Amount", &heightmapBlurAmount, 0, 30);
				ImGui::SliderFloat("Redistribution", &heightmapRedistribution, 0, 10);

				if (ImGui::Button("Regenerate Terrain"))
				{
					generateTerrain();
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Color Info"))
			{
				ImGui::SliderFloat("Blend Threshold", &terrainBlendThreshold, 0, 1);
				ImGui::SliderFloat("Noise Influence", &terrainNoiseInfluence, 0, 1);

				for (int i = 0; i < terrainColArray.size(); i++)
				{
					const std::string colorLabel = "Color " + std::to_string(i);
					ImGui::ColorEdit3(colorLabel.c_str(), &terrainColArray[i].r);

					const std::string thresholdLabel = "Theshold " + std::to_string(i);

					float min;
					float max;

					if (i == 0)
					{
						min = 0;
					}
					else
					{
						min = terrainColThresholds[i - 1];
					}

					if (i == terrainColArray.size() - 1)
					{
						min = 1;
						max = 1;
					}
					else
					{
						max = terrainColThresholds[i + 1];
					}

					ImGui::SliderFloat(thresholdLabel.c_str(), &terrainColThresholds[i], min, max);

					ImGui::NewLine();
				}

				if (terrainColArray.size() > 1) // Button to remove
				{
					if (ImGui::Button("Remove Color"))
					{
						terrainColArray.pop_back();
						terrainColThresholds.pop_back();

						terrainColThresholds[terrainColArray.size() - 1] = 1;
					}
				}

				if (terrainColArray.size() < MAX_TERRAIN_COLORS)
				{
					if (ImGui::Button("Add Color"))
					{
						terrainColArray.push_back(terrainColArray[terrainColArray.size() - 1]);

						float lower;

						if (terrainColThresholds.size() > 1)
						{
							lower = terrainColThresholds[terrainColThresholds.size() - 2];
						}
						else
						{
							lower = 0;
						}

						terrainColThresholds[terrainColThresholds.size() - 1] = glm::mix(lower, 1.0f, .5);
						terrainColThresholds.push_back(1);
					}
				}

				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Material"))
		{
			ImGui::ColorEdit3("Color", &material.color.r);
			ImGui::SliderFloat("AmbientCoefficient", &material.ambientCoefficient, 0, 1);
			ImGui::SliderFloat("DiffuseCoefficient", &material.diffuseCoefficient, 0, 1);
			ImGui::SliderFloat("SpecularCoefficient", &material.specularCoefficient, 0, 1);
			ImGui::SliderFloat("Shininess", &material.shininess, 0, 512);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("GeneralLights"))
		{
			ImGui::BeginTabBar("GeneralLightsTabBar");

			for (int i = 0; i < generalLights.size(); i++)
			{
				const std::string tabName = "Light" + std::to_string(i);
				if (ImGui::BeginTabItem(tabName.c_str()))
				{
					ImGui::ColorEdit3("Light Color", &generalLights[i].color.r);
					ImGui::DragFloat3("Light Position", &generalLights[i].transform.position.x, .1);
					ImGui::SliderFloat("Light Intensity", &generalLights[i].intensity, -1, 3);
					ImGui::EndTabItem();
				}
			}
			ImGui::EndTabBar();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("DirectionalLights"))
		{
			ImGui::BeginTabBar("DirectionalLightsTabBar");

			for (int i = 0; i < directionalLights.size(); i++)
			{
				const std::string tabName = "Light" + std::to_string(i);
				if (ImGui::BeginTabItem(tabName.c_str()))
				{
					ImGui::ColorEdit3("Light Color", &directionalLights[i].color.r);
					ImGui::DragFloat3("Light Direction", &directionalLights[i].direction.x, 1);
					ImGui::SliderFloat("Light Intensity", &directionalLights[i].intensity, -1, 3);
					ImGui::EndTabItem();
				}
			}
			ImGui::EndTabBar();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("PointLights"))
		{
			ImGui::BeginTabBar("PointLightsTabBar");

			for (int i = 0; i < pointLights.size(); i++)
			{
				const std::string tabName = "Light" + std::to_string(i);
				if (ImGui::BeginTabItem(tabName.c_str()))
				{
					ImGui::ColorEdit3("Light Color", &pointLights[i].color.r);
					ImGui::DragFloat3("Light Position", &pointLights[i].transform.position.x, 1);
					ImGui::SliderFloat("Light Intensity", &pointLights[i].intensity, -1, 3);

					ImGui::SliderFloat("Constant Coefficient", &pointLights[i].constantCoefficient, .1, 1);
					ImGui::SliderFloat("Linear Coefficient", &pointLights[i].linearCoefficient, 0, 1);
					ImGui::SliderFloat("Quadratic Coefficient", &pointLights[i].quadraticCoefficient, 0, 1);
					ImGui::EndTabItem();
				}
			}
			ImGui::EndTabBar();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("SpotLights"))
		{
			ImGui::BeginTabBar("SpotLightsTabBar");

			for (int i = 0; i < spotLights.size(); i++)
			{
				const std::string tabName = "Light" + std::to_string(i);
				if (ImGui::BeginTabItem(tabName.c_str()))
				{
					ImGui::ColorEdit3("Light Color", &spotLights[i].color.r);
					ImGui::DragFloat3("Light Position", &spotLights[i].transform.position.x, 1);
					ImGui::DragFloat3("Light Direction", &spotLights[i].direction.x, 1);
					ImGui::SliderFloat("Light Intensity", &spotLights[i].intensity, -1, 3);

					ImGui::SliderFloat("Penumbra Angle", &spotLights[i].penumbraAngle, 0, spotLights[i].umbraAngle);
					ImGui::SliderFloat("Umbra Angle", &spotLights[i].umbraAngle, spotLights[i].penumbraAngle, 179.99);

					ImGui::SliderFloat("Attenuation Exponent", &spotLights[i].attenuationExponent, 0, 3);

					ImGui::EndTabItem();
				}
			}
			ImGui::EndTabBar();
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwPollEvents();

		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}


//Author: Eric Winebrenner
void resizeFrameBufferCallback(GLFWwindow* window, int width, int height)
{
	SCREEN_WIDTH = width;
	SCREEN_HEIGHT = height;
	camera.setAspectRatio((float)SCREEN_WIDTH / SCREEN_HEIGHT);
	glViewport(0, 0, width, height);
}
//Author: Eric Winebrenner
void keyboardCallback(GLFWwindow* window, int keycode, int scancode, int action, int mods)
{
	if (keycode == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	//Reset camera
	if (keycode == GLFW_KEY_R && action == GLFW_PRESS) {
		camera.setPosition(glm::vec3(0, 0, 5));
		camera.setYaw(-90.0f);
		camera.setPitch(0.0f);
		firstMouseInput = false;
	}
	if (keycode == GLFW_KEY_1 && action == GLFW_PRESS) {
		wireFrame = !wireFrame;
		glPolygonMode(GL_FRONT_AND_BACK, wireFrame ? GL_LINE : GL_FILL);
	}
}
//Author: Eric Winebrenner
void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (abs(yoffset) > 0) {
		float fov = camera.getFov() - (float)yoffset * CAMERA_ZOOM_SPEED;
		//camera.setFov(fov);
	}
}
//Author: Eric Winebrenner
void mousePosCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED) {
		return;
	}
	if (!firstMouseInput) {
		prevMouseX = xpos;
		prevMouseY = ypos;
		firstMouseInput = true;
	}
	float yaw = camera.getYaw() + (float)(xpos - prevMouseX) * MOUSE_SENSITIVITY;
	camera.setYaw(yaw);
	float pitch = camera.getPitch() - (float)(ypos - prevMouseY) * MOUSE_SENSITIVITY;
	pitch = glm::clamp(pitch, -89.9f, 89.9f);
	camera.setPitch(pitch);
	prevMouseX = xpos;
	prevMouseY = ypos;
}
//Author: Eric Winebrenner
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	//Toggle cursor lock
	if (button == MOUSE_TOGGLE_BUTTON && action == GLFW_PRESS) {
		int inputMode = glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED;
		glfwSetInputMode(window, GLFW_CURSOR, inputMode);
		glfwGetCursorPos(window, &prevMouseX, &prevMouseY);
	}
}

//Author: Eric Winebrenner
//Returns -1, 0, or 1 depending on keys held
float getAxis(GLFWwindow* window, int positiveKey, int negativeKey) {
	float axis = 0.0f;
	if (glfwGetKey(window, positiveKey)) {
		axis++;
	}
	if (glfwGetKey(window, negativeKey)) {
		axis--;
	}
	return axis;
}

//Author: Eric Winebrenner
//Get input every frame
void processInput(GLFWwindow* window) {

	float moveAmnt = CAMERA_MOVE_SPEED * deltaTime;

	//Get camera vectors
	glm::vec3 forward = camera.getForward();
	glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
	glm::vec3 up = glm::normalize(glm::cross(forward, right));

	glm::vec3 position = camera.getPosition();
	position += forward * getAxis(window, GLFW_KEY_W, GLFW_KEY_S) * moveAmnt;
	position += right * getAxis(window, GLFW_KEY_D, GLFW_KEY_A) * moveAmnt;
	position += up * getAxis(window, GLFW_KEY_Q, GLFW_KEY_E) * moveAmnt;
	camera.setPosition(position);
}
