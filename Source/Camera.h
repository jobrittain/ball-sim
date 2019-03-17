#pragma once
#include "Entity.h"
#include <AntTweakBar.h>
#include <mutex>

class Camera : public Entity
{
private:
	float _movementSpeed;
	float _keyMovementSpeed;
	float _zoomSpeed;
	float _keyZoomSpeed;

	std::mutex posLock;

public:
	Camera();
	~Camera();

	void SetMovementSpeed(float speed);
	void SetZoomSpeed(float speed);

	DirectX::SimpleMath::Matrix GetViewMatrix();

	void MoveUp(float deltaTime);
	void MoveDown(float deltaTime);
	void MoveLeft(float deltaTime);
	void MoveRight(float deltaTime);
	void MoveForwards(float deltaTime);
	void MoveBackwards(float deltaTime);
	void ZoomIn(float deltaTime);
	void ZoomOut(float deltaTime);

	void MouseMove(int x, int y);
	void MouseZoom(int scroll);

	void RegisterTwBarVariables(TwBar * bar)
	{
		TwAddVarRO(bar, "Camera X", TW_TYPE_FLOAT, &_position.x, "");
		TwAddVarRO(bar, "Camera Y", TW_TYPE_FLOAT, &_position.y, "");
		TwAddVarRO(bar, "Camera Z", TW_TYPE_FLOAT, &_position.z, "");
	}
};

