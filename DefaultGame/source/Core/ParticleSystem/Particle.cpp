#include "Particle.h"

Particle::Particle(int x, int y, std::shared_ptr<Sprite> texture, Core::Particle::ParticleAnimation type)
    : _posX(x), _posY(y), _frame(0), _isVisible(true), _texture(texture), _type(type)
{
}

void Particle::render()
{

}
void Particle::update(float deltaTime)
{
    // Update the particle's state based on deltaTime
    _frame++;
}
  
bool Particle::isDead()
{
    // Check if the particle is dead based on _frame or other criteria
    return _frame >= 60; // Example condition, particle is dead after 60 frames
}
bool Particle::isVisible() const
{
    return _isVisible;
}