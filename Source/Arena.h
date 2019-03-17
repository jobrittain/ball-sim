#pragma once
#include "Entity.h"
class Arena : public Entity
{
private:
	float _height;
	float _radius;

public:
	Arena();
	~Arena();

	void SetHeight(float height);
	void SetRadius(float radius);

	float GetHeight();
	float GetRadius();
};

