#include "ParticleSystem.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace
{
    bool validParticleData(const ParticleData& particle)
    {
        return std::isfinite(particle.positionX) && std::isfinite(particle.positionY)
            && std::isfinite(particle.velocityX) && std::isfinite(particle.velocityY)
            && std::isfinite(particle.age) && particle.age >= 0.0f
            && std::isfinite(particle.lifetime) && particle.lifetime > 0.0f;
    }
}

ParticleSystem& ParticleSystem::GetInstance()
{
    static ParticleSystem instance;
    return instance;
}

ParticleSystem::ParticleSystem()
{
}

ParticleHandle ParticleSystem::addParticle(const ParticleData& particle)
{
    if (!validParticleData(particle))
    {
        throw std::invalid_argument("Particle values must be finite with a positive lifetime and nonnegative age");
    }

    const auto previousSize = _xParticles.size();
    const bool newSlot = _firstFreeSlot == ParticleHandle::InvalidId;
    const auto slotId = newSlot ? _slots.size() : _firstFreeSlot;
    if (newSlot)
    {
        _slots.emplace_back();
    }

    try
    {
        _xParticles.emplace_back(particle.positionX);
        _yParticles.emplace_back(particle.positionY);
        _frames.emplace_back(particle.frame);
        _visibility.emplace_back(particle.visible);
        _textures.emplace_back(particle.texture);
        _types.emplace_back(particle.type);
        _velocityX.emplace_back(particle.velocityX);
        _velocityY.emplace_back(particle.velocityY);
        _ages.emplace_back(particle.age);
        _lifetimes.emplace_back(particle.lifetime);
        _denseSlots.emplace_back(slotId);
    }
    catch (...)
    {
        _xParticles.resize(previousSize);
        _yParticles.resize(previousSize);
        _frames.resize(previousSize);
        _visibility.resize(previousSize);
        _textures.resize(previousSize);
        _types.resize(previousSize);
        _velocityX.resize(previousSize);
        _velocityY.resize(previousSize);
        _ages.resize(previousSize);
        _lifetimes.resize(previousSize);
        _denseSlots.resize(previousSize);
        if (newSlot)
        {
            _slots.pop_back();
        }
        throw;
    }

    auto& slot = _slots[slotId];
    if (!newSlot)
    {
        _firstFreeSlot = slot.nextFree;
    }
    slot.index = previousSize;
    slot.nextFree = ParticleHandle::InvalidId;
    return {slotId, slot.generation};
}

bool ParticleSystem::removeParticle(ParticleHandle handle) noexcept
{
    if (!isValid(handle))
    {
        return false;
    }

    auto& slot = _slots[handle.id];
    const auto index = slot.index;
    const auto lastIndex = _xParticles.size() - 1;
    if (index != lastIndex)
    {
        _xParticles[index] = _xParticles.back();
        _yParticles[index] = _yParticles.back();
        _frames[index] = _frames.back();
        _visibility[index] = _visibility.back();
        _textures[index] = std::move(_textures.back());
        _types[index] = _types.back();
        _velocityX[index] = _velocityX.back();
        _velocityY[index] = _velocityY.back();
        _ages[index] = _ages.back();
        _lifetimes[index] = _lifetimes.back();
        _denseSlots[index] = _denseSlots.back();
        _slots[_denseSlots[index]].index = index;
    }
    _xParticles.pop_back();
    _yParticles.pop_back();
    _frames.pop_back();
    _visibility.pop_back();
    _textures.pop_back();
    _types.pop_back();
    _velocityX.pop_back();
    _velocityY.pop_back();
    _ages.pop_back();
    _lifetimes.pop_back();
    _denseSlots.pop_back();

    slot.index = ParticleHandle::InvalidId;
    if (slot.generation != std::numeric_limits<std::uint64_t>::max())
    {
        ++slot.generation;
        slot.nextFree = _firstFreeSlot;
        _firstFreeSlot = handle.id;
    }
    return true;
}

std::optional<ParticleData> ParticleSystem::getParticle(ParticleHandle handle) const
{
    if (!isValid(handle))
    {
        return std::nullopt;
    }

    const auto index = _slots[handle.id].index;
    return ParticleData{
        _xParticles[index], _yParticles[index], _textures[index],
        _types[index], _frames[index], _visibility[index],
        _velocityX[index], _velocityY[index], _ages[index], _lifetimes[index]
    };
}

bool ParticleSystem::setParticle(ParticleHandle handle, const ParticleData& particle) noexcept
{
    if (!isValid(handle) || !validParticleData(particle))
    {
        return false;
    }

    const auto index = _slots[handle.id].index;
    _xParticles[index] = particle.positionX;
    _yParticles[index] = particle.positionY;
    _frames[index] = particle.frame;
    _visibility[index] = particle.visible;
    _textures[index] = particle.texture;
    _types[index] = particle.type;
    _velocityX[index] = particle.velocityX;
    _velocityY[index] = particle.velocityY;
    _ages[index] = particle.age;
    _lifetimes[index] = particle.lifetime;
    return true;
}

bool ParticleSystem::isValid(ParticleHandle handle) const noexcept
{
    return handle.id < _slots.size()
        && _slots[handle.id].generation == handle.generation
        && _slots[handle.id].index != ParticleHandle::InvalidId;
}

bool ParticleSystem::isDead(ParticleHandle handle) const noexcept
{
    return isValid(handle) && _ages[_slots[handle.id].index] >= _lifetimes[_slots[handle.id].index];
}

std::size_t ParticleSystem::getParticleCount() const noexcept
{
    return _xParticles.size();
}

void ParticleSystem::clearParticles() noexcept
{
    while (!_denseSlots.empty())
    {
        const auto slotId = _denseSlots.back();
        removeParticle(ParticleHandle{slotId, _slots[slotId].generation});
    }
}

void ParticleSystem::update(float deltaTime)
{
    if (!std::isfinite(deltaTime) || deltaTime <= 0.0f)
    {
        return;
    }

    for (std::size_t index = 0; index < _ages.size(); ++index)
    {
        const float activeTime = std::min(deltaTime, std::max(0.0f, _lifetimes[index] - _ages[index]));
        _xParticles[index] += _velocityX[index] * activeTime;
        _yParticles[index] += _velocityY[index] * activeTime;
        _ages[index] = std::min(_ages[index] + deltaTime, _lifetimes[index]);
        _frames[index] = static_cast<int>(60.0f * (_ages[index] / _lifetimes[index]));
    }
}

void ParticleSystem::render()
{
}