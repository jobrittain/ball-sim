#include "Box.h"



Box::Box()
{
}


Box::~Box()
{
}

void Box::SetWidth(float width)
{
}

void Box::SetHeight(float height)
{
	_height = height;
}

void Box::SetDepth(float depth)
{
	_depth = depth;
}

float Box::GetWidth()
{
	return _width;
}

float Box::GetHeight()
{
	return _height;
}

float Box::GetDepth()
{
	return _depth;
}
