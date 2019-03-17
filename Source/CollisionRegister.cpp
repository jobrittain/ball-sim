#include "CollisionRegister.h"



CollisionRegister::CollisionRegister() : _pointCount(0), _points()
{
}

CollisionRegister::~CollisionRegister()
{
}

void CollisionRegister::Add(CollisionPoint point)
{
	_points[_pointCount] = point;
	_pointCount++;
}

void CollisionRegister::Clear()
{
	_pointCount = 0;
}

unsigned CollisionRegister::GetCollisionCount()
{
	return _pointCount;
}

CollisionPoint & CollisionRegister::GetPoint(unsigned index)
{
	return _points[index];
}