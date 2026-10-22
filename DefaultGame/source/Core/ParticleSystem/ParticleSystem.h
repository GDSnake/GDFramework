#pragma once

#include <memory>
#include <optional>
#include <vector>
#include "Particle.h"

class ParticleSystem
{
public:
    static ParticleSystem& GetInstance();

    ParticleSystem(const ParticleSystem&) = delete;
    ParticleSystem& operator=(const ParticleSystem&) = delete;
    ParticleSystem(ParticleSystem&&) = delete;
    ParticleSystem& operator=(ParticleSystem&&) = delete;

    ParticleHandle addParticle(const ParticleData& particle);
    bool removeParticle(ParticleHandle handle) noexcept;
    std::optional<ParticleData> getParticle(ParticleHandle handle) const;
    bool setParticle(ParticleHandle handle, const ParticleData& particle) noexcept;
    bool isValid(ParticleHandle handle) const noexcept;
    bool isDead(ParticleHandle handle) const noexcept;
    std::size_t getParticleCount() const noexcept;
    void clearParticles() noexcept;

    void update(float deltaTime);

private:
    ParticleSystem();
    ~ParticleSystem() = default;

    void render();

    struct ParticleSlot
    {
        std::size_t index = ParticleHandle::InvalidId;
        std::size_t nextFree = ParticleHandle::InvalidId;
        std::uint64_t generation = 0;
    };

    std::vector<float> _xParticles;
    std::vector<float> _yParticles;
    std::vector<int> _frames;
    std::vector<bool> _visibility;
    std::vector<std::shared_ptr<Sprite>> _textures;
    std::vector<Core::Particle::ParticleAnimation> _types;
    std::vector<float> _velocityX;
    std::vector<float> _velocityY;
    std::vector<float> _ages;
    std::vector<float> _lifetimes;
    std::vector<std::size_t> _denseSlots;
    std::vector<ParticleSlot> _slots;
    std::size_t _firstFreeSlot = ParticleHandle::InvalidId;
};