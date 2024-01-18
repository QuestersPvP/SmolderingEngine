#pragma once

#include "../Object.h"

class GameObject : public Object
{
public:
	GameObject();

//#pragma region Getter/Setters
//	void SetModel(vkglTF::Model _model) { model = _model; };
//	vkglTF::Model GetModel() { return model; };
//#pragma endregion


	/* Variables */
vkglTF::Model model;
private:
};

