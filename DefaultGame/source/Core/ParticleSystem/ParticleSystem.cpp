#include "ParticleSystem.h"

ParticleSystem& ParticleSystem::GetInstance()
{
    static ParticleSystem instance;
    return instance;
}

ParticleSystem::ParticleSystem()
{
}

void ParticleSystem::addParticle(const std::string& name, std::shared_ptr<Particle> particle)
{
    _particles[name] = particle;
}

void ParticleSystem::removeParticle(const std::string& name)
{
    _particles.erase(name);
}

std::shared_ptr<Particle> ParticleSystem::getParticle(const std::string& name)
{
    auto it = _particles.find(name);
    if (it != _particles.end())
    {
        return it->second;
    }
    return nullptr;
}

void ParticleSystem::update(float deltaTime)
{
    for (auto& [name, particle] : _particles)
    {
        particle->update(deltaTime);
    }
}

void ParticleSystem::render()
{
    for (auto& [name, particle] : _particles)
    {
        particle->render();
    }
}