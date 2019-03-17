#pragma once
#include "Entity.h"
class Box : public Entity
{
private:
	float _width;
	float _height;
	float _depth;

public:
	Box();
	~Box();

	void SetWidth(float width);
	void SetHeight(float height);
	void SetDepth(float depth);

	float GetWidth();
	float GetHeight();
	float GetDepth();
};

