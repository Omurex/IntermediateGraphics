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
    float specularCoefficient;

    float shininess;
};

#define MAX_LIGHTS 1
uniform Light _Lights[MAX_LIGHTS];

uniform vec3 _ViewerPosition;


                        // ambientIntensity is same as material base color
vec3 calculateAmbient(vec3 ambientIntensity, float ambientCoefficient)
{
    return ambientIntensity * ambientCoefficient;
}


vec3 calculateDiffuse(vec3 normal, vec3 dirFromLight, vec3 lightIntensity, float diffuseCoefficient)
{
    dirFromLight = normalize(dirFromLight);
    float dotProduct = dot(dirFromLight, normal);

    return diffuseCoefficient * dotProduct * lightIntensity;
}


vec3 calculateSpecular(vec3 dirFromLight, vec3 normal, vec3 dirToViewer, float shininess, vec3 lightIntensity, float specularCoefficient)
{
    vec3 reflectedLightDir = normalize(reflect(dirFromLight, normal));

    float dotProduct = dot(reflectedLightDir, normalize(dirToViewer));
    dotProduct = clamp(dotProduct, 0, 1);

    float dotProductPow = pow(dotProduct, shininess);

    return specularCoefficient * dotProductPow * lightIntensity;
}


void main(){      
    vec3 normal = normalize(v_out.WorldNormal);
    vec3 pos = v_out.WorldPosition;

    vec3 ambient = calculateAmbient(_Lights[0].color, _Lights[0].ambientCoefficient);
    vec3 diffuse = calculateDiffuse(normal, _Lights[0].position - pos, _Lights[0].color, _Lights[0].diffuseCoefficient);
    vec3 specular = calculateSpecular(pos - _Lights[0].position, normal, _ViewerPosition - pos, _Lights[0].shininess, _Lights[0].color, _Lights[0].specularCoefficient);
    vec3 color = ambient + diffuse + specular; 

    FragColor = vec4(color, 1.0f);
}
