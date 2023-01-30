#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include <stdio.h>


struct Vec3
{
	float x, y, z = 0;

	Vec3(float _x = 0, float _y = 0, float _z = 0) { x = _x; y = _y; z = _z; }
};



struct Color
{
	float r, g, b, a = 1;

	Color(float _r = 1, float _g = 1, float _b = 1, float _a = 1) { r = _r; g = _g; b = _b; a = _a; }
};



struct Vertex
{
	Vec3 pos;
	Color col;

	Vertex(Vec3 _pos, Color _col) { pos = _pos; col = _col; }
	Vertex(float x, float y, float z, Color color) : Vertex(Vec3(x, y, z), color) {}

};



//void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void resizeFrameBufferCallback(GLFWwindow* window, int width, int height);

//TODO: Vertex shader source code
const char* vertexShaderSource = 
"#version 450								\n"
"layout (location = 0) in vec3 vPos;		\n"
"layout (location = 1) in vec4 vColor;		\n"
"out vec4 Color;							\n"
"out vec3 Position;							\n"
"uniform float _Time;						\n"
"void main() {								\n"
"	Color = vColor;							\n"
"	Position = vPos;						\n"
"	gl_Position = vec4(vPos.x * sin(_Time + vPos.x * 2 / (abs(vPos.y) + 1)), vPos.y + cos(_Time * vPos.x) / (abs(vPos.x) + 1), vPos.z, 1.0); 			\n"
"}											\0";

//TODO: Fragment shader source code
const char* fragmentShaderSource = 
"#version 450								\n"
"out vec4 FragColor;						\n"
"in vec4 Color;								\n"
"in vec3 Position;							\n"
"uniform float _Time;						\n"
"void main() {								\n"
"	float t = abs(sin(_Time + Position.x));				\n"
"	FragColor = Color * t;					\n"	
"}											\0";


const Vertex vertices[] =
{
	Vertex(-0.3, -.5, 0.0, Color(1.0, 0.0, 0.0, 1.0)),
	Vertex(0, -.5, 0.0, Color(0.0, .5f, .25f, 1.0)),
	Vertex(.4, 0.25, 0.0, Color(.8f, 0.0, 1.0, 1.0)),

	Vertex(0.6, 0.25, 0.0, Color(1.0, 0.0, 0.0, 1.0)),
	Vertex(-0.2, 0.25, 0.0, Color(0.0, 1.0, 0.0, 1.0)),
	Vertex(0.0, -0.25, 0.0, Color(0.0, 0.0, 1.0, 1.0)),
};


int main() {
	if (!glfwInit()) {
		printf("glfw failed to init");
		return 1;
	}

	GLFWwindow* window = glfwCreateWindow(640, 480, "OpenGLExample", 0, 0);
	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK) {
		printf("glew failed to init");
		return 1;
	}

	glfwSetFramebufferSizeCallback(window, resizeFrameBufferCallback);


	//TODO: Create and compile vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	//TODO: Get vertex shader compilation status and output info log
	GLint success;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if(!success)
	{
		// Dump logs into a char array - 512 is an arbitrary length
		GLchar infoLog[512];
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		printf("Failed to compile vertex shader: %s", infoLog);
	}
	
	//TODO: Create and compile fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	//TODO: Get fragment shader compilation status and output info log
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		// Dump logs into a char array - 512 is an arbitrary length
		GLchar infoLog[512];
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		printf("Failed to compile fragment shader: %s", infoLog);
	}

	//TODO: Create shader program
	GLuint shaderProgram = glCreateProgram();

	//TODO: Attach vertex and fragment shaders to shader program
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);

	//TODO: Link shader program
	// Create an empty shader program

	// Link program - will create an executable program with the attached shaders
	glLinkProgram(shaderProgram);

	//TODO: Check for link status and output errors
	// Logging
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if(!success)
	{
		GLchar infoLog[512];
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		printf("Failed to link shader program: %s", infoLog);
	}

	//TODO: Delete vertex + fragment shader objects
	//glDeleteShader(...)

	//TODO: Create and bind Vertex Array Object (VAO)
	// Create a new vertex array object
	unsigned int vertexArrayObject;
	glGenVertexArrays(1, &vertexArrayObject);

	// Bind this new VAO so that subsequent function calls will modify it
	glBindVertexArray(vertexArrayObject);

	//TODO: Create and bind Vertex Buffer Object (VBO), fill with vertexData
	// Generate a buffer with an id
	unsigned int vertexBufferObject;
	glGenBuffers(1, &vertexBufferObject);

	// Bind the new buffer. GL_ARRAY_BUFFER is the binding target for vertex attributes
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);

	// Create a new data store
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//TODO: Define vertex attribute layout
	// Define how our vertex data should be interpreted by the vertex shader
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (const void*) 0);
	//glEnableVertexAttribArray(0); // Enable this vertex attribute (required!)

	// Position (3 floats, xyz)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (const void*) 0);
	glEnableVertexAttribArray(0); // Enable this vertex attribute (required!)

	// Color (4 floats, rgba)
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (const void*)(sizeof(float)*3));
	glEnableVertexAttribArray(1); // Enable this vertex attribute (required!)

	while (!glfwWindowShouldClose(window)) {
		glClearColor(0.2f, 0.3f, 0.6f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//TODO:Use shader program
		glUseProgram(shaderProgram);

		// Set unfiroms
		float time = (float) glfwGetTime();
		glUniform1f(glGetUniformLocation(shaderProgram, "_Time"), time);
		
		//TODO: Draw triangle (3 indices!)
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

void resizeFrameBufferCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

