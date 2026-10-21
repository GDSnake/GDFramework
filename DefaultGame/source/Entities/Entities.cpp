#include "Entities.h"

#include <algorithm>
#include <stdexcept>

Entities& Entities::GetInstance()
{
	static Entities instance;
	return instance;
}

void Entities::AddEntity(const EntityData& entity)
{
	if (_nameEntities.find(entity.name) != _nameEntities.end())
	{
		throw std::invalid_argument("An entity with this name already exists");
	}

	const auto previousSize = _nameEntities.size();
	try
	{
		_xEntities.emplace_back(entity.x);
		_yEntities.emplace_back(entity.y);
		_spriteEntities.emplace_back(entity.sprite);
		_visibilityEntities.emplace_back(entity.visible);
		_nameEntities.try_emplace(entity.name, previousSize);
	}
	catch (...)
	{
		_xEntities.resize(previousSize);
		_yEntities.resize(previousSize);
		_spriteEntities.resize(previousSize);
		_visibilityEntities.resize(previousSize);
		throw;
	}
}

void Entities::RemoveEntity(const EntityData& entity)
{
	const auto found = _nameEntities.find(entity.name);
	if (found == _nameEntities.end())
	{
		return;
	}

	const auto index = found->second;
	_xEntities.erase(_xEntities.begin() + index);
	_yEntities.erase(_yEntities.begin() + index);
	_spriteEntities.erase(_spriteEntities.begin() + index);
	_visibilityEntities.erase(_visibilityEntities.begin() + index);
	_nameEntities.erase(found);
	for (auto& nameIndex : _nameEntities)
	{
		if (nameIndex.second > index)
		{
			--nameIndex.second;
		}
	}
}

void Entities::UpdateEntity(const EntityData& entity)
{
	const auto found = _nameEntities.find(entity.name);
	if (found == _nameEntities.end())
	{
		throw std::out_of_range("Entity name was not found");
	}

	const auto index = found->second;
	_xEntities[index] = entity.x;
	_yEntities[index] = entity.y;
	_spriteEntities[index] = entity.sprite;
	_visibilityEntities[index] = entity.visible;
}

void Entities::ClearEntities()
{
	_xEntities.clear();
	_yEntities.clear();
	_spriteEntities.clear();
	_visibilityEntities.clear();
	_nameEntities.clear();
}

EntityData Entities::GetEntity(const std::size_t index) const
{
	if (index >= _nameEntities.size())
	{
		throw std::out_of_range("Entity index is out of range");
	}

	const auto found = std::find_if(_nameEntities.begin(), _nameEntities.end(),
		[index](const auto& nameIndex) { return nameIndex.second == index; });

	return {
		_xEntities[index],
		_yEntities[index],
		_spriteEntities[index],
		_visibilityEntities[index],
		found->first
	};
}

EntityData Entities::GetEntity(const std::string& name) const
{
	const auto found = _nameEntities.find(name);
	if (found == _nameEntities.end())
	{
		throw std::out_of_range("Entity name was not found");
	}

	const auto index = found->second;
	return {
		_xEntities[index],
		_yEntities[index],
		_spriteEntities[index],
		_visibilityEntities[index],
		found->first
	};
}
