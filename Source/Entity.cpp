#include "Entity.h"



Entity::Entity()
{
}


Entity::~Entity()
{
}

void Entity::SetPosition(DirectX::SimpleMath::Vector3 position)
{
	_position = position;
}

DirectX::SimpleMath::Vector3 Entity::GetPosition() const
{
	return _position;
}
