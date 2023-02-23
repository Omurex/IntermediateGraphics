#version 450                          
out vec4 FragColor;

in struct Vertex{
    vec3 WorldNormal;
    vec3 WorldPosition;
}v_out;

struct GeneralLight
{
    vec3 position;
    vec3 color;
    float intensity;
};


struct DirectionalLight
{
    vec3 direction;
    vec3 color;
    float intensity;
};


struct PointLight
{
    vec3 position;
    vec3 color;
    float intensity;
};


struct SpotLight
{
    vec3 position;
    vec3 direction;

    float umbraAngle; // Outer angle where intensity reaches 0
    float penumbraAngle; // Inner angle where intensity is max
};


struct Material
{
    vec3 color;

    // Between 0 and 1
	float ambientCoefficient;
	float diffuseCoefficient;
	float specularCoefficient;

	float shininess;
};


#define MAX_LIGHTS 8
uniform GeneralLight _GeneralLights[MAX_LIGHTS];
uniform int _NumGeneralLights = 0;

uniform DirectionalLight _DirectionalLights[MAX_LIGHTS];
uniform int _NumDirectionalLights = 0;

uniform PointLight _PointLights[MAX_LIGHTS];
uniform int _NumPointLights = 0;

uniform SpotLight _SpotLights[MAX_LIGHTS];
uniform int _NumSpotLights = 0;

uniform Material _Material;

uniform vec3 _ViewerPosition;


                        // ambientIntensity is same as material base color
vec3 calculateAmbient(vec3 ambientIntensity, float ambientCoefficient)
{
    return ambientIntensity * ambientCoefficient;
}


vec3 calculateDiffuse(vec3 normal, vec3 dirToLight, vec3 lightIntensity, float diffuseCoefficient)
{
    dirToLight = normalize(dirToLight);
    float dotProduct = dot(dirToLight, normal);
    dotProduct = clamp(dotProduct, 0, 1);

    return diffuseCoefficient * dotProduct * lightIntensity;
}


vec3 calculateSpecular(vec3 dirToLight, vec3 normal, vec3 dirToViewer, float shininess, vec3 lightIntensity, float specularCoefficient)
{
//    float dotProduct = dot(reflectedLightDir, normalize(dirToViewer));
//    dotProduct = clamp(dotProduct, 0, 1);

    vec3 halfVector = normalize(dirToViewer + dirToLight);

    float dotProduct = dot(normal, halfVector);
    dotProduct = clamp(dotProduct, 0, 1);

    float dotProductPow = pow(dotProduct, shininess);

    return specularCoefficient * dotProductPow * lightIntensity;
}


void main(){      
    vec3 normal = normalize(v_out.WorldNormal);
    vec3 pos = v_out.WorldPosition;

    int numGeneralLights = clamp(_NumGeneralLights, 0, MAX_LIGHTS);
    int numDirectionalLights = clamp(_NumDirectionalLights, 0, MAX_LIGHTS);
    int numPointLights = clamp(_NumPointLights, 0, MAX_LIGHTS);
    int numSpotLights = clamp(_NumSpotLights, 0, MAX_LIGHTS);

    vec3 ambient = calculateAmbient(_Material.color, _Material.ambientCoefficient); // Not calculated using lights

    vec3 diffuseAndSpecularTotal = vec3(0);

    for(int i = 0; i < _NumGeneralLights; i++)
    {
        GeneralLight light = _GeneralLights[i];
        diffuseAndSpecularTotal += calculateDiffuse(normal, light.position - pos, light.color * light.intensity, _Material.diffuseCoefficient);
        diffuseAndSpecularTotal += calculateSpecular(light.position - pos, normal, _ViewerPosition - pos, _Material.shininess, light.color * light.intensity, _Material.specularCoefficient);
    }

    for(int i = 0; i < _NumDirectionalLights; i++)
    {
        DirectionalLight light = _DirectionalLights[i];
        diffuseAndSpecularTotal += calculateDiffuse(normal, -light.direction, light.color * light.intensity, _Material.diffuseCoefficient);
        diffuseAndSpecularTotal += calculateSpecular(-light.direction, normal, _ViewerPosition - pos, _Material.shininess, light.color * light.intensity, _Material.specularCoefficient);
    }
    
    vec3 color = ambient + diffuseAndSpecularTotal; 

    FragColor = vec4(color, 1.0f);
}
