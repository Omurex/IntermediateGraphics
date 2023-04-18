#version 450                          
out vec4 FragColor;

in struct Vertex{
    vec3 WorldNormal;
    vec3 WorldPosition;
    vec2 UV;
} v_out;

in vec4 lightSpacePos; // This is the fragment's homogenous clip coordinates from the POV of the light.

uniform sampler2D _ShadowMap;

uniform float _MinBias;
uniform float _MaxBias;


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

    float constantCoefficient;
    float linearCoefficient;
    float quadraticCoefficient;
};


struct SpotLight
{
    vec3 position;
    vec3 color;
    float intensity;
    vec3 direction;

    float minAngleCos;
    float maxAngleCos;

    //float umbraAngle; // Outer angle where intensity reaches 0
    //float penumbraAngle; // Inner angle where intensity is max

    float attenuationExponent;
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


uniform float _Time;

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

uniform sampler2D _Texture;
uniform sampler2D _NoiseTexture;


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
    // Uncomment for phong specular lighting
//  float dotProduct = dot(reflectedLightDir, normalize(dirToViewer));
//  dotProduct = clamp(dotProduct, 0, 1);

    vec3 halfVector = normalize(dirToViewer + dirToLight);

    float dotProduct = dot(normal, halfVector);
    dotProduct = clamp(dotProduct, 0, 1);

    float dotProductPow = pow(dotProduct, shininess);

    return specularCoefficient * dotProductPow * lightIntensity;
}


float calcShadow(sampler2D shadowMap, vec4 lightSpacePos, vec3 normal, vec3 toLight)
{
    // Homogeneous Clip space to NDC coords [-w, w] to [-1, 1]
    vec3 sampleCoord = lightSpacePos.xyz / lightSpacePos.w;

    // Convert from [-1, 1] to [0, 1] for sampling
    sampleCoord = sampleCoord * .5f + .5f;

    float shadowMapDepth = texture(shadowMap, sampleCoord.xy).r;
    float myDepth = sampleCoord.z - max(_MaxBias * (1.0 - dot(normal, toLight)), _MinBias);
//
//    // step(a, b) returns 1.0 if a >= b, 0.0 otherwise
//    return step(shadowMapDepth, myDepth);

    float totalShadow = 0;
    vec2 texelOffset = 1.0 / textureSize(_ShadowMap, 0);

    for(int y = -1; y <= 1; y++)
    {
	    for(int x = -1; x <= 1; x++)
        {
		    vec2 uv = sampleCoord.xy + vec2(x * texelOffset.x, y * texelOffset.y);
            totalShadow += step(texture(_ShadowMap, uv).r, myDepth);
        }
    }

    return totalShadow / 9.0;
}


void main()
{
	// During lighting, sample from your shadow map texture
	// Transform lightSpacePos from homogeneous [-w,w] to NDC coordinates [-1,1], then convert to [0,1] range, then use .xy to sample from _ShadowMap.



    vec3 normal = normalize(v_out.WorldNormal);
    vec3 pos = v_out.WorldPosition;

    int numGeneralLights = clamp(_NumGeneralLights, 0, MAX_LIGHTS);
    int numDirectionalLights = clamp(_NumDirectionalLights, 0, MAX_LIGHTS);
    int numPointLights = clamp(_NumPointLights, 0, MAX_LIGHTS);
    int numSpotLights = clamp(_NumSpotLights, 0, MAX_LIGHTS);

    vec3 ambient = calculateAmbient(_Material.color, _Material.ambientCoefficient); // Not calculated using lights

    vec3 diffuseAndSpecularTotal = vec3(0);

    float shadow = 0;

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
    
        shadow = calcShadow(_ShadowMap, lightSpacePos, normal, -light.direction);
    }

    for(int i = 0; i < _NumPointLights; i++)
    {
        PointLight light = _PointLights[i];

        float dist = distance(pos, light.position);
        float intensityMult = 1 / (light.constantCoefficient + (dist * light.linearCoefficient) + (pow(dist, 2) * light.quadraticCoefficient));

        diffuseAndSpecularTotal += calculateDiffuse(normal, light.position - pos, light.color * light.intensity * intensityMult, _Material.diffuseCoefficient);
        diffuseAndSpecularTotal += calculateSpecular(light.position - pos, normal, _ViewerPosition - pos, _Material.shininess, light.color * light.intensity * intensityMult, _Material.specularCoefficient);
    }

    for(int i = 0; i < _NumSpotLights; i++)
    {
        SpotLight light = _SpotLights[i];

        vec3 dirToFrag = normalize((pos - light.position));
        float angleCos = dot(light.direction, dirToFrag);

        float intensityMult = clamp((angleCos - light.maxAngleCos) / (light.minAngleCos - light.maxAngleCos), 0, 1);
        intensityMult = pow(intensityMult, light.attenuationExponent);

        diffuseAndSpecularTotal += calculateDiffuse(normal, -dirToFrag, light.color * light.intensity * intensityMult, _Material.diffuseCoefficient);
        diffuseAndSpecularTotal += calculateSpecular(-dirToFrag, normal, _ViewerPosition - pos, _Material.shininess, light.color * light.intensity * intensityMult, _Material.specularCoefficient);
    }

    //vec3 color = ambient + diffuseAndSpecularTotal; 

    //FragColor = vec4(color, 1.0f);

    // Texture no lighting
    vec2 uv = v_out.UV;
    uv.x += _Time;


    vec4 color = texture(_Texture, uv) * (vec4(ambient, 1.0f) + (vec4(diffuseAndSpecularTotal, 1.0f) * (1.0 - shadow)));
    //color *= texture(_NoiseTexture, v_out.UV);
    //vec4 color = texture(_NoiseTexture, v_out.UV);

    FragColor = color;

    // Debug to show just uv gradient
    //FragColor = vec4(v_out.UV.x, v_out.UV.y, 0.0f, 1.0f);
}