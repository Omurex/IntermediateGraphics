#version 450                          
layout (location = 0) in vec3 vPos;  
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vUV;
layout (location = 3) in vec3 vTangent;

uniform mat4 _Model;
uniform mat4 _View;
uniform mat4 _Projection;

out struct Vertex{
    vec3 WorldNormal;
    vec3 WorldPosition;
    vec2 UV;
    mat3 TBNMatrix;
} v_out;

void main()
{    
    v_out.WorldPosition = vec3(_Model * vec4(vPos,1));
    v_out.WorldNormal = transpose(inverse(mat3(_Model))) * vNormal;
    v_out.UV = vUV;

    vec3 bitangent = cross(vTangent, vNormal);
    v_out.TBNMatrix[0] = vTangent;
    v_out.TBNMatrix[1] = bitangent;
    v_out.TBNMatrix[2] = normalize(v_out.WorldNormal);

    gl_Position = _Projection * _View * _Model * vec4(vPos,1);
}
