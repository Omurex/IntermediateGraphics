//depthOnly.vert

uniform mat4 _Model;
uniform mat4 _View; // Use your lookAt function, pointing in the direction of the light
uniform mat4 _Projection // Use your othographic function. Make sure the frustum is large enough to see the full scene;


int main()
{
	gl_Position = _Projection * _View * _Model * vec4(vPos,1);
}
