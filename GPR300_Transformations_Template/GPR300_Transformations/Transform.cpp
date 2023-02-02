#include "Transform.h"


glm::mat4 Transform::getModelMatrix()
{
	glm::mat4 scaleMat = glm::mat4(0);
	scaleMat[0][0] = scale.x;
	scaleMat[1][1] = scale.y;
	scaleMat[2][2] = scale.z;
	scaleMat[3][3] = 1;

	glm::mat4 rotMat;
	glm::mat4 transMat;

	return scaleMat;
}
