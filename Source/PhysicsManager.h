#pragma once
#include <memory>
#include <vector>
#include "Ball.h"
#include "Box.h"
#include "CollisionRegister.h"
#include "Influencer.h"
#include "Arena.h"

#include <SimpleMath.h>
#include <AntTweakBar.h>

class PhysicsManager
{
private:

	float _gravity = -9.81f;
	float _elasticity = 0.85f;
	float _friction = 0.01f;
	
	DirectX::SimpleMath::Vector3 _floorSurfaceNormal;

	float _lastDeltaTime;

	std::mutex* _ballsPhysicsLock;
	std::mutex* _ballsResetLock;

	std::vector<Ball>* _balls;
	std::vector<Ball *> _ballsInSight;
	Influencer* _influencer;
	Arena* _arena;

	Influencer* _peerInfluencer;
	bool* _isConnected;

	float _floorSurfacePosition;
	float _ballRadius;
	float _influencerRadius;

	CollisionRegister _collisionRegister;

public:
	PhysicsManager();
	~PhysicsManager();

	void SetFriction(float friction);
	void SetElasticity(float elasticity);
	void SetBalls(std::vector<Ball>& balls, float ballRadius, std::mutex& ballsPhysicsLock, std::mutex& ballsResetLock);
	void SetInfluencer(Influencer& influencer, float influencerRadius);
	void SetPeerInfluencer(Influencer& peerInfluencer);
	void SetArena(Arena& arena);

	void RegisterTwBarVariables(TwBar* bar)
	{
		TwAddVarRW(bar, "Elasticity", TW_TYPE_FLOAT, &_elasticity, "min=0 max=2 step=0.01");
		TwAddVarRW(bar, "Friction", TW_TYPE_FLOAT, &_friction, "min=0 max=1 step=0.01");
	}
	
	void Update(float deltaTime);
};

