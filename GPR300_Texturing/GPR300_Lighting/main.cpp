#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdio.h>
#include <vector>

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
const float CAMERA_MOVE_SPEED = 5.0f;
const float CAMERA_ZOOM_SPEED = 3.0f;

Camera camera((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT);

glm::vec3 bgColor = glm::vec3(0);

bool wireFrame = false;

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


GLuint createTexture(const char* filePath)
{
	int width, height, numComponents;
	unsigned char* textureData = stbi_load(filePath, &width, &height, &numComponents, 0);

	if (textureData == nullptr) return -1;

	// Generate a texture name
	GLuint texture;
	glGenTextures(1, &texture);

	glActiveTexture(GL_TEXTURE0);

	// Bind our name to GL_TEXTURE_2D to make it a 2D texture
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
	glGenerateMipmap(GL_TEXTURE_2D);

	return texture;
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

	ew::MeshData cubeMeshData;
	ew::createCube(1.0f, 1.0f, 1.0f, cubeMeshData);
	ew::MeshData sphereMeshData;
	ew::createSphere(0.5f, 64, sphereMeshData);
	ew::MeshData cylinderMeshData;
	ew::createCylinder(1.0f, 0.5f, 64, cylinderMeshData);
	ew::MeshData planeMeshData;
	ew::createPlane(1.0f, 1.0f, planeMeshData);

	ew::Mesh cubeMesh(&cubeMeshData);
	ew::Mesh sphereMesh(&sphereMeshData);
	ew::Mesh planeMesh(&planeMeshData);
	ew::Mesh cylinderMesh(&cylinderMeshData);

	//Enable back face culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	//Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Enable depth testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	//Initialize shape transforms
	ew::Transform cubeTransform;
	ew::Transform sphereTransform;
	ew::Transform planeTransform;
	ew::Transform cylinderTransform;

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

	GLuint texture = createTexture("PavingStones070_1K_Color.png");
	glActiveTexture(GL_TEXTURE0);
	litShader.setInt("_ColorTexture", 0);


	//pointLights[1].transform.position = glm::vec3(-2, 1, -2);
	//pointLights[1].color = glm::vec3(0, 1, 0);


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

		//Draw
		litShader.use();
		litShader.setMat4("_Projection", camera.getProjectionMatrix());
		litShader.setMat4("_View", camera.getViewMatrix());

		litShader.setVec3("_ViewerPosition", camera.getPosition());

		litShader.setVec3("_Material.color", material.color);
		litShader.setFloat("_Material.ambientCoefficient", material.ambientCoefficient);
		litShader.setFloat("_Material.diffuseCoefficient", material.diffuseCoefficient);
		litShader.setFloat("_Material.specularCoefficient", material.specularCoefficient);
		litShader.setFloat("_Material.shininess", material.shininess);

		litShader.setInt("_NumGeneralLights", generalLights.size());
		litShader.setInt("_NumDirectionalLights", directionalLights.size());
		litShader.setInt("_NumPointLights", pointLights.size());
		litShader.setInt("_NumSpotLights", spotLights.size());


		//Set general lighting uniforms
		for (size_t i = 0; i < generalLights.size(); i++)
		{
			litShader.setVec3("_GeneralLights[" + std::to_string(i) + "].position", generalLights[i].transform.position);
			litShader.setFloat("_GeneralLights[" + std::to_string(i) + "].intensity", generalLights[i].intensity);
			litShader.setVec3("_GeneralLights[" + std::to_string(i) + "].color", generalLights[i].color);
		}

		//Set directional lighting uniforms
		for (size_t i = 0; i < directionalLights.size(); i++)
		{
			litShader.setVec3("_DirectionalLights[" + std::to_string(i) + "].direction", glm::normalize(directionalLights[i].direction));
			litShader.setFloat("_DirectionalLights[" + std::to_string(i) + "].intensity", directionalLights[i].intensity);
			litShader.setVec3("_DirectionalLights[" + std::to_string(i) + "].color", directionalLights[i].color);
		}

		//Set point lighting uniforms
		for (size_t i = 0; i < pointLights.size(); i++)
		{
			litShader.setVec3("_PointLights[" + std::to_string(i) + "].position", pointLights[i].transform.position);
			litShader.setFloat("_PointLights[" + std::to_string(i) + "].intensity", pointLights[i].intensity);
			litShader.setVec3("_PointLights[" + std::to_string(i) + "].color", pointLights[i].color);

			litShader.setFloat("_PointLights[" + std::to_string(i) + "].constantCoefficient", pointLights[i].constantCoefficient);
			litShader.setFloat("_PointLights[" + std::to_string(i) + "].linearCoefficient", pointLights[i].linearCoefficient);
			litShader.setFloat("_PointLights[" + std::to_string(i) + "].quadraticCoefficient", pointLights[i].quadraticCoefficient);
		}

		//Set spot lighting uniforms
		for (size_t i = 0; i < spotLights.size(); i++)
		{
			litShader.setVec3("_SpotLights[" + std::to_string(i) + "].position", spotLights[i].transform.position);
			litShader.setVec3("_SpotLights[" + std::to_string(i) + "].color", spotLights[i].color);
			litShader.setFloat("_SpotLights[" + std::to_string(i) + "].intensity", spotLights[i].intensity);
			litShader.setVec3("_SpotLights[" + std::to_string(i) + "].direction", glm::normalize(spotLights[i].direction));

			float minAngleCos = glm::cos(glm::radians(spotLights[i].penumbraAngle));
			float maxAngleCos = glm::cos(glm::radians(spotLights[i].umbraAngle));

			litShader.setFloat("_SpotLights[" + std::to_string(i) + "].minAngleCos", minAngleCos);
			litShader.setFloat("_SpotLights[" + std::to_string(i) + "].maxAngleCos", maxAngleCos);

			litShader.setFloat("_SpotLights[" + std::to_string(i) + "].attenuationExponent", spotLights[i].attenuationExponent);
		}


		//Draw cube
		litShader.setMat4("_Model", cubeTransform.getModelMatrix());
		cubeMesh.draw();

		//Draw sphere
		litShader.setMat4("_Model", sphereTransform.getModelMatrix());
		sphereMesh.draw();

		//Draw cylinder
		litShader.setMat4("_Model", cylinderTransform.getModelMatrix());
		cylinderMesh.draw();

		//Draw plane
		litShader.setMat4("_Model", planeTransform.getModelMatrix());
		planeMesh.draw();

		//Draw light as a small sphere using unlit shader, ironically.
		unlitShader.use();
		unlitShader.setMat4("_Projection", camera.getProjectionMatrix());
		unlitShader.setMat4("_View", camera.getViewMatrix());

		for (int i = 0; i < generalLights.size(); i++) // Draw general lights
		{
			unlitShader.setMat4("_Model", generalLights[i].transform.getModelMatrix());
			unlitShader.setVec3("_Color", generalLights[i].color);
			sphereMesh.draw();
		}

		for (int i = 0; i < pointLights.size(); i++)
		{
			unlitShader.setMat4("_Model", pointLights[i].transform.getModelMatrix());
			unlitShader.setVec3("_Color", pointLights[i].color);
			sphereMesh.draw();
		}

		for (int i = 0; i < spotLights.size(); i++)
		{
			unlitShader.setMat4("_Model", spotLights[i].transform.getModelMatrix());
			unlitShader.setVec3("_Color", spotLights[i].color);
			sphereMesh.draw();
		}

		//Draw UI
		ImGui::Begin("Settings");
		ImGui::BeginTabBar("TabBar");

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
		camera.setFov(fov);
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
