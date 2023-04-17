#version 450                          
layout (location = 0) in vec3 vPos;  
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vUV;

out struct Vertex{
    vec2 UV;
} v_out;

void main()
{    
    v_out.UV = vUV;

    gl_Position = vec4(vPos,1);
}
