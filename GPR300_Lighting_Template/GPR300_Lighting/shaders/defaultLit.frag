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


vec3 calculateDiffuse(vec3 normal, vec3 dirFromLight, vec3 lightIntensity, float diffuseCoefficient)
{
    dirFromLight = normalize(dirFromLight);
    float dotProduct = dot(dirFromLight, normal);

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

    int numGeneralLights = clamp(0, MAX_LIGHTS, _NumGeneralLights);
    int numDirectionalLights = clamp(0, MAX_LIGHTS, _NumDirectionalLights);
    int numPointLights = clamp(0, MAX_LIGHTS, _NumPointLights);
    int numSpotLights = clamp(0, MAX_LIGHTS, _NumSpotLights);

    vec3 ambient = calculateAmbient(_Material.color, _Material.ambientCoefficient); // Not calculated using lights

    vec3 diffuseAndSpecularTotal = vec3(0);

    for(int i = 0; i < _NumGeneralLights; i++)
    {
        diffuseAndSpecularTotal += calculateDiffuse(normal, _GeneralLights[i].position - pos, _GeneralLights[i].color * _GeneralLights[i].intensity, _Material.diffuseCoefficient);
        diffuseAndSpecularTotal += calculateSpecular(_GeneralLights[i].position - pos, normal, _ViewerPosition - pos, _Material.shininess, _GeneralLights[i].color * _GeneralLights[i].intensity, _Material.specularCoefficient);
    }
    
    vec3 color = ambient + diffuseAndSpecularTotal; 

    FragColor = vec4(color, 1.0f);
}
