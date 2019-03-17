#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <d3d11.h>
#include <SimpleMath.h>

class Entity
{
protected:
	DirectX::SimpleMath::Vector3 _position;

public:
	Entity();
	~Entity();

	void SetPosition(DirectX::SimpleMath::Vector3 position);

	DirectX::SimpleMath::Vector3 GetPosition() const;
};

