#version 450
out vec4 FragColor;

void main()
{
	// Depth is automatically written
	FragColor = vec4(vec3(gl_FragCoord.z), 1.0);
}