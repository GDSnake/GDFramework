
#include <map>
#include <memory>
#include "Particle.h"

class ParticleSystem
{
public:
    static ParticleSystem& GetInstance();
    void addParticle(const std::string& name, std::shared_ptr<Particle> particle);
    void removeParticle(const std::string& name);
    std::shared_ptr<Particle> getParticle(const std::string& name);
    void update(float deltaTime);

private:
    ParticleSystem();
    ~ParticleSystem() = default;

    void render();

    std::map<std::string, std::shared_ptr<Particle>> _particles;
};