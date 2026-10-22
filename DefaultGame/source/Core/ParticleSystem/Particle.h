#pragma once

#include <cstddef>
#include <memory>
#include <cstdint>
#include <limits>

class Sprite;

namespace Core::Particle
{
    enum class ParticleAnimation : std::uint8_t
    {
        Rain = 0,
        Spark,
        PulseSquare,
        Total,
        // Add more particle types as needed
    };
}

struct ParticleHandle
{
    static constexpr std::size_t InvalidId = std::numeric_limits<std::size_t>::max();

    std::size_t id = InvalidId;
    std::uint64_t generation = 0;

    bool operator==(const ParticleHandle&) const = default;
};

struct ParticleData
{
    float positionX = 0.0f;
    float positionY = 0.0f;
    std::shared_ptr<Sprite> texture;
    Core::Particle::ParticleAnimation type = Core::Particle::ParticleAnimation::Rain;
    int frame = 0;
    bool visible = true;
    float velocityX = 0.0f;
    float velocityY = 0.0f;
    float age = 0.0f;
    float lifetime = 1.0f;
};