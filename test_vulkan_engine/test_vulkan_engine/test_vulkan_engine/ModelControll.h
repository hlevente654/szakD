#pragma once
class ModelControll
{
public:
	int modelId;

	void keyControl(bool* keys, float deltaTime, float angleSpeed);
};

