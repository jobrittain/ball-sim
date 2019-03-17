#include "PhysicsManager.h"
#include <cstdlib>


using namespace DirectX::SimpleMath;

struct State
{
	Vector3 p;      // position
	Vector3 v;      // velocity
};

struct Derivative
{
	Vector3 dx;      // dx/dt = velocity
	Vector3 dv;      // dv/dt = acceleration
};

Derivative Evaluate(const State &initial,
	float t,
	float dt,
	const Derivative &d,
	Vector3 accel)
{
	State state;
	state.p = initial.p + d.dx*dt;
	state.v = initial.v + d.dv*dt;

	Derivative output;
	output.dx = state.v;
	output.dv = accel;
	return output;
}

void Integrate(State &state,
	float odt,
	float dt,
	Vector3 accel)
{
	Derivative a, b, c, d;

	a = Evaluate(state, odt, 0.0f, Derivative(), accel);
	b = Evaluate(state, odt, dt*0.5f, a, accel);
	c = Evaluate(state, odt, dt*0.5f, b, accel);
	d = Evaluate(state, odt, dt, c, accel);

	Vector3 dxdt = 1.0f / 6.0f *
		(a.dx + 2.0f*(b.dx + c.dx) + d.dx);

	Vector3 dvdt = 1.0f / 6.0f *
		(a.dv + 2.0f*(b.dv + c.dv) + d.dv);

	state.p = state.p + dxdt * dt;
	state.v = state.v + dvdt * dt;
}

PhysicsManager::PhysicsManager() : _lastDeltaTime(0.f)
{
}

PhysicsManager::~PhysicsManager()
{
}

void PhysicsManager::SetFriction(float friction)
{
	_friction = friction;
}

void PhysicsManager::SetElasticity(float elasticity)
{
	_elasticity = elasticity;
}

void PhysicsManager::SetBalls(std::vector<Ball>& balls, float ballRadius, std::mutex & ballsPhysicsLock, std::mutex & ballsResetLock)
{
	_balls = &balls;
	_ballsPhysicsLock = &ballsPhysicsLock;
	_ballsResetLock = &ballsResetLock;
	_ballRadius = ballRadius;
}

void PhysicsManager::SetInfluencer(Influencer& influencer, float influencerRadius)
{
	_influencer = &influencer;
	_influencerRadius = influencerRadius;
}

void PhysicsManager::SetArena(Arena& arena)
{
	_arena = &arena;
	_floorSurfacePosition = arena.GetPosition().y + (arena.GetHeight() / 2);
	_floorSurfaceNormal = Vector3(0.f, 1.f, 0.f);
}

void PhysicsManager::SetPeerInfluencer(Influencer & peerInfluencer)
{
	_peerInfluencer = &peerInfluencer;
}

void PhysicsManager::Update(float deltaTime)
{
	Vector3 influencerPosition = _influencer->GetPosition();
	float influentialForce = _influencer->GetForce();

	auto arenaPos = _arena->GetPosition();
	auto arenaRad = _arena->GetRadius();

	{
		std::lock_guard<std::mutex> lock(*_ballsResetLock);

		_ballsInSight.clear();

		{
			std::lock_guard<std::mutex> lock(*_ballsPhysicsLock);
			for (unsigned bi = 0; bi < _balls->size(); bi++)
			{
				Ball * ballPtr = &_balls->at(bi);
				bool inSight = _influencer->BallInSight(ballPtr->GetPosition(), _ballRadius);
				bool inPeerSight = false;
				if (_peerInfluencer->IsOnline())
				{
					inPeerSight = _peerInfluencer->BallInSight(ballPtr->GetPosition(), _ballRadius);
				}
				bool isOwned = ballPtr->IsOwned();

				if (isOwned)
				{
					if (inSight)
					{
						// Calculate physics for this ball
						_ballsInSight.push_back(&_balls->at(bi));
						ballPtr->SetInSight(true);

						if (inPeerSight)
						{
							ballPtr->SetContendedState(true);
							ballPtr->SetSendState(true);
						}
						else
						{
							ballPtr->SetContendedState(false);
						}
					}
					else if (inPeerSight)
					{
						ballPtr->SetInSight(false);
						ballPtr->SetContendedState(false);
						ballPtr->SetSendState(true);
					}
					else
					{
						ballPtr->SetInSight(false);
						ballPtr->SetContendedState(false);
					}
				}
				else if (inSight)
				{
					ballPtr->SetInSight(true);
					if (inPeerSight)
					{
						ballPtr->SetContendedState(true);
					}
					else
					{
						ballPtr->SetContendedState(false);
					}
				}
				else
				{
					ballPtr->SetInSight(false);
					ballPtr->SetContendedState(false);
					ballPtr->SetValidState(false);
				}
			}
		}

		for (unsigned bi = 0; bi < _ballsInSight.size(); bi++)
		{
			// Get ball info
			Vector3 position = _ballsInSight.at(bi)->GetPosition();
			Vector3 velocity = _ballsInSight.at(bi)->GetVelocity();
			Vector3 rotation = _ballsInSight.at(bi)->GetRotation();
			float mass = static_cast<float>(_balls->at(bi).GetMass());

			// Calculate ball physics using Euler
			State state;
			state.p = position;
			state.v = velocity;

			Vector3 force(0.f, _gravity * mass, 0.f);
			Vector3 accel = force / mass; // slightly unnecessary...
			Integrate(state, _lastDeltaTime, deltaTime, accel);

			// Apply influencer acceleration
			float dist = Vector3::Distance(state.p, influencerPosition) - (_ballRadius + _influencerRadius);
			if (dist < 0.f)
			{
				Vector3 dir = (state.p - influencerPosition);
				dir.Normalize();

				state.v += dir * ((dist * influentialForce * deltaTime) / mass);
			}

			if (_peerInfluencer->IsOnline())
			{
				Vector3 peerInfluencerPosition = _peerInfluencer->GetPosition();
				float peerDist = Vector3::Distance(state.p, peerInfluencerPosition) - (_ballRadius + _influencerRadius);
				if (peerDist < 0.f)
				{
					Vector3 dir = (state.p - peerInfluencerPosition);
					dir.Normalize();

					state.v += dir * ((dist * _peerInfluencer->GetForce() * deltaTime) / mass);
				}
			}

			// Process collision with floor
			if (state.p.y < _floorSurfacePosition + _ballRadius)
			{
				state.p = position;

				state.v = state.v -
					(1.f + _elasticity) *
					(state.v.Dot(_floorSurfaceNormal)) *
					_floorSurfaceNormal;

				state.v *= (1 - _friction);
			}

			// Process collision with wall
			Vector2 bPos = Vector2(state.p.x, state.p.z);
			Vector2 aPos = Vector2(arenaPos.x, arenaPos.z);

			if (Vector2::Distance(bPos, aPos) > arenaRad - _ballRadius)
			{
				state.p = position;

				Vector3 normal = -position;
				normal.Normalize();

				state.v = state.v -
					(1.f + _elasticity) *
					(state.v.Dot(normal)) *
					normal;

				state.v *= (1 - _friction);
			}


			// Calculate ball rotation
			if (state.v.x < 0 && velocity.x > 0)
			{
				rotation.x = 0;
			}
			if (state.v.x > 0 && velocity.x < 0)
			{
				rotation.x = 0;
			}
			if (state.v.z < 0 && velocity.z > 0)
			{
				rotation.x = 0;
			}
			if (state.v.z > 0 && velocity.z < 0)
			{
				rotation.x = 0;
			}

			rotation.x += velocity.x * 0.1f;

			Vector2 velRoteNorm = Vector2(state.v.x, state.v.z);
			velRoteNorm.Normalize();

			float z = velRoteNorm.y;
			z = (z + 1) / 2;
			rotation.y = DirectX::XM_PI * z;

			// Set new ball info
			_ballsInSight.at(bi)->SetNewPosition(state.p);
			_ballsInSight.at(bi)->SetNewVelocity(state.v);
			_ballsInSight.at(bi)->SetRotation(rotation);
		}

		_collisionRegister.Clear();

		for (unsigned bi = 0; bi < _ballsInSight.size(); bi++)
		{
			Vector3 position = _ballsInSight.at(bi)->GetNewPosition();

			// Register collisions with other balls
			for (unsigned bj = bi + 1; bj < _ballsInSight.size(); bj++)
			{
				Vector3 otherPosition = _ballsInSight.at(bj)->GetNewPosition();

				float dist = Vector3::Distance(position, otherPosition) - (_ballRadius * 2);

				if (dist < 0.f)
				{
					CollisionPoint cp;

					cp.ballFirst = &*_ballsInSight.at(bi);
					cp.ballSecond = &*_ballsInSight.at(bj);

					cp.contactNormal = position - otherPosition;
					cp.contactNormal.Normalize();

					_collisionRegister.Add(cp);
				}
			}
		}

		for (unsigned ci = 0; ci < _collisionRegister.GetCollisionCount(); ++ci)
		{
			CollisionPoint point = _collisionRegister.GetPoint(ci);

			// Calculate new velocities
			Vector3 normal = point.contactNormal;

			Vector3 velO1 = point.ballFirst->GetNewVelocity();
			Vector3 velO2 = point.ballSecond->GetNewVelocity();
			float mass1 = static_cast<float>(point.ballFirst->GetMass());
			float mass2 = static_cast<float>(point.ballSecond->GetMass());


			float check = (velO1 - velO2).Dot(normal); // Black magic.
			if (check < 0.f)
			{
				Vector3 velN1 =
					(((mass1 - _elasticity * mass2) * (velO1.Dot(normal)) * normal) +
					((mass2 + _elasticity * mass2) * (velO2.Dot(normal)) * normal)) /
						(mass1 + mass2);

				Vector3 velN2 =
					(((mass1 + _elasticity * mass2) * (velO1.Dot(normal)) * normal) +
					((mass2 - _elasticity * mass2) * (velO2.Dot(normal)) * normal)) /
						(mass1 + mass2);

				Vector3 velR1 = velO1 - (velO1.Dot(normal) * normal) + velN1;
				Vector3 velR2 = velO2 - (velO2.Dot(normal) * normal) + velN2;

				point.ballFirst->ResetPosition();
				point.ballFirst->SetNewVelocity(velR1);

				point.ballSecond->ResetPosition();
				point.ballSecond->SetNewVelocity(velR2);
			}
		}

		for (unsigned bi = 0; bi < _ballsInSight.size(); bi++)
		{
			_ballsInSight.at(bi)->Update();
		}

	}

	_lastDeltaTime = deltaTime;
}
