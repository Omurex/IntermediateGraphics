#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdio.h>
#include <vector>
#include <random>

#include "Transform.h"
#include "Camera.h"
#include "Cube.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "EW/Shader.h"
#include "EW/ShapeGen.h"

void resizeFrameBufferCallback(GLFWwindow* window, int width, int height);
void keyboardCallback(GLFWwindow* window, int keycode, int scancode, int action, int mods);

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

const int NUM_CUBES = 5;

glm::vec3 bgColor = glm::vec3(0);
float exampleSliderFloat = 0.0f;


float randRange(float min, float max)
{
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = max - min;
	float r = random * diff;
	return min + r;
}


int main() {
	if (!glfwInit()) {
		printf("glfw failed to init");
		return 1;
	}

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Transformations", 0, 0);
	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK) {
		printf("glew failed to init");
		return 1;
	}

	glfwSetFramebufferSizeCallback(window, resizeFrameBufferCallback);
	glfwSetKeyCallback(window, keyboardCallback);

	// Setup UI Platform/Renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	//Dark UI theme.
	ImGui::StyleColorsDark();

	Shader shader("shaders/vertexShader.vert", "shaders/fragmentShader.frag");

	std::srand(std::time(0));

	std::vector<Cube*> cubes;
	for (int i = 0; i < NUM_CUBES; i++)
	{
		glm::vec3 dimensions = glm::vec3(randRange(.5f, 3), randRange(.5f, 3), randRange(.5f, 3));
		glm::vec3 position = glm::vec3(randRange(-5, 5), randRange(-5, 5), randRange(-5, 5));
		glm::vec3 rotation = glm::vec3(randRange(0, 2 * glm::pi<float>()), randRange(0, 2 * glm::pi<float>()), randRange(0, 2 * glm::pi<float>()));
		cubes.push_back(new Cube(dimensions, position, rotation, glm::vec3(1, 1, 1)));
	}

	Camera camera(
		glm::vec3(-5, 1, -5), 
		glm::vec3(0, 0, 0)
	);

	float cameraRadius = 2;
	float cameraSpeed = 1;

	//Enable back face culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	//Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Enable depth testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	while (!glfwWindowShouldClose(window)) {
		glClearColor(bgColor.r, bgColor.g, bgColor.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		float time = (float)glfwGetTime();
		deltaTime = time - lastFrameTime;
		lastFrameTime = time;

		camera.position.x = glm::cos(time * cameraSpeed) * cameraRadius;
		camera.position.z = glm::sin(time * cameraSpeed) * cameraRadius;

		//Draw
		shader.use();

		for (Cube* cube : cubes)
		{
			// Pass in uniforms
			glm::mat4 mvpMatrix = camera.getProjectionMatrix((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT) * camera.getViewMatrix() * cube->getModelMatrix();

			shader.setMat4("_MVPMatrix", mvpMatrix);

			cube->draw();
		}

		//Draw UI
		ImGui::Begin("Settings");
		ImGui::BeginTabBar("TabBar");

		for (int i = 0; i < cubes.size(); i++)
		{
			const std::string tabName = "Cube" + std::to_string(i);

			if (ImGui::BeginTabItem(tabName.c_str()))
			{
				Cube* cube = cubes[i];

				ImGui::SliderFloat("ScaleX", &cube->transform.scale.x, .25, 4);
				ImGui::SliderFloat("ScaleY", &cube->transform.scale.y, .25, 4);
				ImGui::SliderFloat("ScaleZ", &cube->transform.scale.z, .25, 4);

				ImGui::SliderFloat("RotationX", &cube->transform.rotation.x, 0, 2 * glm::pi<float>());
				ImGui::SliderFloat("RotationY", &cube->transform.rotation.y, 0, 2 * glm::pi<float>());
				ImGui::SliderFloat("RotationZ", &cube->transform.rotation.z, 0, 2 * glm::pi<float>());
				ImGui::EndTabItem();
			}
		}
		
		if (ImGui::BeginTabItem("Camera"))
		{
			ImGui::SliderFloat("FOV", &camera.fov, 0.01f, 179.99f);
			ImGui::SliderFloat("OrthographicSize", &camera.orthographicSize, 0.01, 500);

			ImGui::SliderFloat("NearPlane", &camera.nearPlane, 0, camera.farPlane);
			ImGui::SliderFloat("FarPlane", &camera.farPlane, camera.nearPlane, 100);

			ImGui::SliderFloat("Height", &camera.position.y, -10, 10);
			ImGui::Checkbox("Orthographic", &camera.orthographic);

			ImGui::SliderFloat("CameraCircleRadius", &cameraRadius, 0.1, 10);
			ImGui::SliderFloat("CameraCircleSpeed", &cameraSpeed, 0, 10);

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwPollEvents();

		glfwSwapBuffers(window);
	}

	for (Cube* cube : cubes)
	{
		delete cube;
	}

	cubes.clear();

	glfwTerminate();
	return 0;
}

void resizeFrameBufferCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	SCREEN_WIDTH = width;
	SCREEN_HEIGHT = height;
}

void keyboardCallback(GLFWwindow* window, int keycode, int scancode, int action, int mods)
{
	if (keycode == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}