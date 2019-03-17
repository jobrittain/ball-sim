#pragma once
#include "Entity.h"
#include <AntTweakBar.h>
#include <mutex>

class Influencer : public Entity
{
private:
	float _radius;
	float _sightRadius;
	float _movementSpeed;
	float _keyMovementSpeed;
	float _influenceRate;
	float _influentialForce;

	bool _online;

	std::mutex posLock;

public:
	Influencer();
	~Influencer();

	void SetRadius(float radius);
	void SetSightRadius(float sightRadius);
	void SetMovementSpeed(float speed);
	void SetInfluenceRate(float influenceRate);
	void SetOnlineState(bool state);
	void SetForce(float force);

	float GetForce();
	float GetSightRadius();
	bool IsOnline();

	bool BallInSight(DirectX::SimpleMath::Vector3 ballPosition, float ballRadius);

	void MoveUp(float deltaTime);
	void MoveDown(float deltaTime);
	void MoveLeft(float deltaTime);
	void MoveRight(float deltaTime);
	void MoveForwards(float deltaTime);
	void MoveBackwards(float deltaTime);
	void MouseMove(int x, int y);

	void Attract(float deltaTime);
	void Repel(float deltaTime);
	void CancelForce();

	void RegisterTwBarVariables(TwBar* bar)
	{
		TwAddVarRO(bar, "Influence Force", TW_TYPE_FLOAT, &_influentialForce, "");
	}
};

