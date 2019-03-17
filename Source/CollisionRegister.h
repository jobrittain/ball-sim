#pragma once
#include "Ball.h"
#include <array>

struct CollisionPoint
{
	Ball* ballFirst;
	Ball* ballSecond;
	DirectX::SimpleMath::Vector3 contactNormal;
};

class CollisionRegister
{
private:
	std::array<CollisionPoint, 5000> _points;
	unsigned _pointCount;

public:
	CollisionRegister();
	~CollisionRegister();

	void Add(CollisionPoint point);
	void Clear();
	unsigned GetCollisionCount();
	CollisionPoint& GetPoint(unsigned index);
};

