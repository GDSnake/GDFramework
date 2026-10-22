#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <vector>
#include <map>

#include "Core/ParticleSystem/ParticleSystem.h"

class Sprite;

struct EntityData
{
    int x;
    int y;
    std::shared_ptr<Sprite> sprite;
    bool visible;
    std::string name;
    std::vector<ParticleHandle> particles;
};

struct Entities
{
  static Entities& GetInstance();

  void AddEntity(const EntityData& entity);

  void RemoveEntity(const EntityData& entity);

  void UpdateEntity(const EntityData& entity);

  void ClearEntities();

  EntityData GetEntity(std::size_t index) const;
  EntityData GetEntity(const std::string& name) const;
  
private:
    Entities();
    ~Entities() = default;

    std::vector<int> _xEntities;  
    std::vector<int> _yEntities;
    std::vector<std::shared_ptr<Sprite>> _spriteEntities;
    std::vector<bool> _visibilityEntities;
    std::vector<std::vector<ParticleHandle>> _particlesEntitiesList;
    std::map<std::string, std::size_t> _nameEntities;

    ParticleSystem& _particleSystem;
};