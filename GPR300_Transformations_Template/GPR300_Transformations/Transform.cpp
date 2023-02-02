#include "Transform.h"

using namespace glm;


mat4 Transform::getModelMatrix()
{
	mat4 scaleMat = getScaleMatrix();
	mat4 rotMat = getRotationMatrix();
	mat4 transMat = getTranslationMatrix();

	return transMat;
}


mat4 Transform::getScaleMatrix()
{
	mat4 scaleMat = mat4(0);
	scaleMat[0][0] = scale.x;
	scaleMat[1][1] = scale.y;
	scaleMat[2][2] = scale.z;
	scaleMat[3][3] = 1;

	return scaleMat;
}


mat4 Transform::getXRotationMatrix()
{
	float theta = rotation.x;

	mat4 xRot = mat4(0);
	xRot[0][0] = 1;
	xRot[1][1] = cos(theta);
	xRot[1][2] = sin(theta);
	xRot[2][1] = -sin(theta);
	xRot[2][2] = cos(theta);
	xRot[3][3] = 1;

	return xRot;
}


mat4 Transform::getYRotationMatrix()
{
	float theta = rotation.y;

	mat4 yRot = mat4(0);
	yRot[0][0] = cos(theta);
	yRot[0][2] = -sin(theta);
	yRot[1][1] = 1;
	yRot[2][0] = sin(theta);
	yRot[2][2] = cos(theta);
	yRot[3][3] = 1;

	return yRot;
}


mat4 Transform::getZRotationMatrix()
{
	float theta = rotation.z;

	mat4 zRot = mat4(0);
	zRot[0][0] = cos(theta);
	zRot[0][1] = sin(theta);
	zRot[1][0] = -sin(theta);
	zRot[1][1] = cos(theta);
	zRot[2][2] = 1;
	zRot[3][3] = 1;

	return zRot;
}


mat4 Transform::getRotationMatrix()
{
	return getXRotationMatrix() * getYRotationMatrix() *
		getZRotationMatrix();
}


mat4 Transform::getTranslationMatrix()
{
	mat4 transMat = mat4(0);
	
	for (int i = 0; i < 3; ++i)
	{
		transMat[i][i] = 1;
	}

	transMat[3] = vec4(position, 1);

	return transMat;
}
