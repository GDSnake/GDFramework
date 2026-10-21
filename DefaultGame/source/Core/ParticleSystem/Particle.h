#include <memory>
#include <cstdint>

class Sprite;

namespace Core::Particle
{
    enum class ParticleAnimation : std::uint8_t
    {
        Rain = 0,
        Total,
        // Add more particle types as needed
    };
}

class Particle
{
    public:
        Particle( int x, int y, std::shared_ptr<Sprite> texture, Core::Particle::ParticleAnimation type );

        void render();

        void update(float deltaTime);

        bool isDead();
        bool isVisible() const;
        void setVisible(bool isVisible);

    private:
        int _posX, _posY;

        int _frame;

        bool _isVisible;

        std::shared_ptr<Sprite> _texture;
        Core::Particle::ParticleAnimation _type;
};