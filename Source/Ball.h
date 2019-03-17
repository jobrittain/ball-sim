#pragma once
#include "Entity.h"
#include <d3d11.h>
#include <SimpleMath.h>

enum BallMass { Heavy = 20U, Medium = 10U, Light = 5U };

class Ball : public Entity
{
private:
	static unsigned _count;

	unsigned _id;

	bool _owned;
	bool _toSend;
	bool _contended;
	bool _inSight;
	bool _valid;
	
	BallMass _mass;

	DirectX::SimpleMath::Vector3 _velocity;
	DirectX::SimpleMath::Vector3 _newVelocity;

	DirectX::SimpleMath::Vector3 _newPosition;

	DirectX::SimpleMath::Vector3 _rotation;

public:

	Ball();
	~Ball();

	void SetVelocity(DirectX::SimpleMath::Vector3 velocity);
	void SetNewPosition(DirectX::SimpleMath::Vector3 position);
	void SetNewVelocity(DirectX::SimpleMath::Vector3 velocity);
	void SetRotation(DirectX::SimpleMath::Vector3 rotation);
	void SetMass(BallMass mass);

	void SetOwnedState(bool state);
	void SetContendedState(bool state);
	void SetSendState(bool state);
	void SetInSight(bool state);
	void SetValidState(bool state);

	DirectX::SimpleMath::Vector3 GetNewPosition() const;
	DirectX::SimpleMath::Vector3 GetVelocity() const;
	DirectX::SimpleMath::Vector3 GetNewVelocity() const;
	DirectX::SimpleMath::Vector3 GetRotation() const;
	BallMass GetMass();
	
	bool IsOwned();
	bool IsContended();
	bool ToBeSent();
	bool IsInSight();
	bool IsValid();

	unsigned GetID() const;


	

	void ResetPosition();

	void Update();
};

