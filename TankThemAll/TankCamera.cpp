#include "stdafx.h"
#include "TankCamera.h"
using namespace std;
using namespace BasicEngine;
using  namespace Rendering;


TankCamera::~TankCamera()
{
}

void TankCamera::SetTankOffset(glm::vec3 offset)
{
	this->offset = offset;
	
}


glm::vec3 TankCamera::GetTranslation()
{
	return this->offset - this->Position;
}