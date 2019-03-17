#include "Arena.h"



Arena::Arena()
{
}


Arena::~Arena()
{
}

void Arena::SetHeight(float height)
{
	_height = height;
}

void Arena::SetRadius(float radius)
{
	_radius = radius;
}

float Arena::GetHeight()
{
	return _height;
}

float Arena::GetRadius()
{
	return _radius;
}
