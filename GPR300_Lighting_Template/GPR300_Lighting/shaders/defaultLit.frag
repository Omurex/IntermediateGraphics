#version 450                          
out vec4 FragColor;

in struct Vertex{
    vec3 WorldNormal;
    vec3 WorldPosition;
}v_out;

struct Light{
    vec3 position;
    float intensity;
    vec3 color;

    float ambientCoefficient;
};

#define MAX_LIGHTS 1
uniform Light _Lights[MAX_LIGHTS];


vec3 calculateAmbient(vec3 color, float ambientCoefficient)
{
    return color * ambientCoefficient;
}


void main(){      
    vec3 normal = normalize(v_out.WorldNormal);

    vec3 ambient = calculateAmbient(_Lights[0].color, _Lights[0].ambientCoefficient);
    vec3 color = ambient; 

    FragColor = vec4(color, 1.0f);
}
