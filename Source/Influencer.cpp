#include "Influencer.h"


Influencer::Influencer() : _influentialForce(0), _online(false)
{
}


Influencer::~Influencer()
{
}

void Influencer::SetRadius(float radius)
{
	_radius = radius;
}

void Influencer::SetSightRadius(float sightRadius)
{
	_sightRadius = sightRadius;
}

void Influencer::SetMovementSpeed(float speed)
{
	_movementSpeed = speed;
	_keyMovementSpeed = speed + 100;
}

void Influencer::SetInfluenceRate(float influenceRate)
{
	_influenceRate = influenceRate;
}

void Influencer::SetOnlineState(bool state)
{
	_online = state;
}

void Influencer::SetForce(float force)
{
	_influentialForce = force;
}

float Influencer::GetForce()
{
	return _influentialForce;
}

float Influencer::GetSightRadius()
{
	return _sightRadius;
}

bool Influencer::IsOnline()
{
	return _online;
}

bool Influencer::BallInSight(DirectX::SimpleMath::Vector3 ballPosition, float ballRadius)
{
	DirectX::SimpleMath::Vector3 iPosition;
	{
		std::lock_guard<std::mutex> lock(posLock);
		iPosition = _position;
	}

	auto dist = DirectX::SimpleMath::Vector3::Distance(ballPosition, iPosition);

	if (dist < (ballRadius + _sightRadius))
	{
		return true;
	}
	return false;
}

void Influencer::MoveUp(float deltaTime)
{
	std::lock_guard<std::mutex> lock(posLock);
	_position.y += _keyMovementSpeed * deltaTime;
}

void Influencer::MoveDown(float deltaTime)
{
	std::lock_guard<std::mutex> lock(posLock);
	_position.y -= _keyMovementSpeed * deltaTime;
}

void Influencer::MoveLeft(float deltaTime)
{
	std::lock_guard<std::mutex> lock(posLock);
	_position.x -= _keyMovementSpeed * deltaTime;
}

void Influencer::MoveRight(float deltaTime)
{
	std::lock_guard<std::mutex> lock(posLock);
	_position.x += _keyMovementSpeed * deltaTime;
}

void Influencer::MoveForwards(float deltaTime)
{
	std::lock_guard<std::mutex> lock(posLock);
	_position.z -= _keyMovementSpeed * deltaTime;
}

void Influencer::MoveBackwards(float deltaTime)
{
	std::lock_guard<std::mutex> lock(posLock);
	_position.z += _keyMovementSpeed * deltaTime;
}

void Influencer::MouseMove(int x, int y)
{
	std::lock_guard<std::mutex> lock(posLock);
	_position.x += x * _movementSpeed;
	_position.z += y * _movementSpeed;
}

void Influencer::Attract(float deltaTime)
{
	_influentialForce += _influenceRate * deltaTime;
}

void Influencer::Repel(float deltaTime)
{
	_influentialForce -= _influenceRate * deltaTime;
}

void Influencer::CancelForce()
{
	_influentialForce = 0;
}
