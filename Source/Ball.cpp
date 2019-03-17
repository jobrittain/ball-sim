#include "Ball.h"
#include <mutex>

using namespace DirectX::SimpleMath;

unsigned Ball::_count = 0;

std::mutex veloLock;
std::mutex roteLock;
std::mutex massLock;

Ball::Ball()
{
	_id = _count;
	++_count;
	_toSend = false;
	_contended = false;
	_valid = true;
}

Ball::~Ball()
{
}

void Ball::SetVelocity(DirectX::SimpleMath::Vector3 velocity)
{
	_velocity = velocity;
}

void Ball::SetNewPosition(DirectX::SimpleMath::Vector3 position)
{
	_newPosition = position;
}

void Ball::SetNewVelocity(DirectX::SimpleMath::Vector3 velocity)
{
	_newVelocity = velocity;
}

void Ball::SetRotation(DirectX::SimpleMath::Vector3 rotation)
{
	_rotation = rotation;
}

void Ball::SetMass(BallMass mass)
{
	_mass = mass;
}

DirectX::SimpleMath::Vector3 Ball::GetNewPosition() const
{
	return _newPosition;
}

DirectX::SimpleMath::Vector3 Ball::GetVelocity() const
{
	return _velocity;
}

DirectX::SimpleMath::Vector3 Ball::GetNewVelocity() const
{
	return _newVelocity;
}

DirectX::SimpleMath::Vector3 Ball::GetRotation() const
{
	return _rotation;
}

BallMass Ball::GetMass()
{
	return _mass;
}

unsigned Ball::GetID() const
{
	return _id;
}

bool Ball::IsOwned()
{
	return _owned;
}

bool Ball::IsContended()
{
	return _contended;
}

bool Ball::ToBeSent()
{
	return _toSend;
}

bool Ball::IsInSight()
{
	return _inSight;
}

bool Ball::IsValid()
{
	return _valid;
}

void Ball::SetOwnedState(bool state)
{
	_owned = state;
}

void Ball::SetContendedState(bool state)
{
	_contended = state;
}

void Ball::SetSendState(bool state)
{
	_toSend = state;
}

void Ball::SetInSight(bool state)
{
	_inSight = state;
}

void Ball::SetValidState(bool state)
{
	_valid = state;
}

void Ball::ResetPosition()
{
	_newPosition = _position;
}

void Ball::Update()
{
	_velocity = _newVelocity;
	_position = _newPosition;
}
