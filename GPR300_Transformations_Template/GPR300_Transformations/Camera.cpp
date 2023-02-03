#include "Camera.h"


using namespace glm;


glm::mat4 Camera::getViewMatrix(vec3 up)
{
	vec3 forward = normalize(target - position);
	vec3 right = normalize(cross(forward, up));
	vec3 newUp = normalize(cross(right, forward));

	forward *= -1; // To change to RH, flip the z axis (forward)

	mat4 viewMatrix = identity<mat4>();
	viewMatrix[0] = vec4(right, 0);
	viewMatrix[1] = vec4(newUp, 0);
	viewMatrix[2] = vec4(forward, 0);

	viewMatrix = transpose(viewMatrix);

	mat4 transMatrix = mat4(0);
	for (int i = 0; i < 3; i++)
	{
		transMatrix[i][i] = 1;
	}

	transMatrix[3] = vec4(-position, 1);

	return viewMatrix * transMatrix;
}


glm::mat4 Camera::getProjectionMatrix(float aspectRatio)
{
	if (orthographic)
	{
		return ortho(orthographicSize, aspectRatio, nearPlane, farPlane);
	}
	else
	{
		return perspective(fov, aspectRatio, nearPlane, farPlane);
	}
}


glm::mat4 Camera::ortho(float height, float aspectRatio, float nearPlane, float farPlane)
{
	float width = height * aspectRatio;

	mat4 orthographicMat = identity<mat4>();
	orthographicMat[0][0] = 2 / width; // (r - l) == width
	orthographicMat[1][1] = 2 / height; // (t - b) == height
	orthographicMat[2][2] = -2 / (farPlane - nearPlane);
	orthographicMat[3][2] = -(farPlane + nearPlane) / (farPlane - nearPlane);

	return orthographicMat;
}


glm::mat4 Camera::perspective(float fov, float aspectRatio, float nearPlane, float farPlane)
{
	float c = tan((radians<float>(fov) / 2.0f));

	mat4 perspectiveMat = identity<mat4>();
	perspectiveMat[0][0] = 1 / (c * aspectRatio);
	perspectiveMat[1][1] = 1 / c;
	perspectiveMat[2][2] = -(farPlane + nearPlane) / (farPlane - nearPlane);
	perspectiveMat[2][3] = -1;
	perspectiveMat[3][2] = -(2 * farPlane * nearPlane) / (farPlane - nearPlane);

	return perspectiveMat;
}
