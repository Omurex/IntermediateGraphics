#version 450                          
layout (location = 0) in vec3 vPos;  
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vUV;

uniform mat4 _Model;
uniform mat4 _View; // Use your lookAt function, pointing in the direction of the light
uniform mat4 _Projection; // Use your othographic function. Make sure the frustum is large enough to see the full scene;


void main()
{
	gl_Position = _Projection * _View * _Model * vec4(vPos,1);
}
