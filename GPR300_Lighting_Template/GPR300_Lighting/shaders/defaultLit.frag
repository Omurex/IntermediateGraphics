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

    // Move these to material
    float ambientCoefficient;
    float diffuseCoefficient; // Between 0 and 1
};

#define MAX_LIGHTS 1
uniform Light _Lights[MAX_LIGHTS];


vec3 calculateAmbient(vec3 ambientIntensity, float ambientCoefficient)
{
    return ambientIntensity * ambientCoefficient;
}


vec3 calculateDiffuse(vec3 normal, vec3 lightPos, vec3 worldPos, vec3 lightIntensity, float diffuseCoefficient)
{
    vec3 toLight = normalize(lightPos - worldPos);
    float dotProduct = dot(toLight, normal);

    return diffuseCoefficient * dotProduct * lightIntensity;
}


void main(){      
    vec3 normal = normalize(v_out.WorldNormal);

    vec3 ambient = calculateAmbient(_Lights[0].color, _Lights[0].ambientCoefficient);
    vec3 diffuse = calculateDiffuse(normal, _Lights[0].position, v_out.WorldPosition, _Lights[0].color, _Lights[0].diffuseCoefficient);
    vec3 color = ambient + diffuse; 

    FragColor = vec4(color, 1.0f);
}
