#include "TestScene.h"

#include <algorithm>
#include <cmath>
#include <format>
#include <stdexcept>

#include <SDL3_image/SDL_image.h>

#include "AssetManagers/FontsManager.h"
#include "AssetManagers/SpriteManager.h"
#include "Core/Renderer.h"
#include "Entities/Sprite.h"

void TestScene::init()
{
    _input = std::make_unique<InputManager>();
    _sheet = SpriteManager::getInstance().getSprite("fire_effects");
    if (!_sheet || _sheet->getFrameCount() < 4)
    {
        throw std::runtime_error("ParticlesDemo requires the fire effects sprite sheet and four configured frames");
    }
    SDL_SetTextureScaleMode(_sheet->getTexture().get(), SDL_SCALEMODE_NEAREST);
    _font = FontsManager::getInstance().getFont("defaultFont");

    const auto& config = Config::getInstance().getConfigData();
    _mousePosition = {static_cast<float>(config.width) * 0.5f, static_cast<float>(config.height) * 0.5f};
    _entity = {config.width / 2, config.height / 2, _sheet, true, "mouse_flame", {}};
    _entity.particles.reserve(SparkCount + SquareCount);
    auto& particles = ParticleSystem::GetInstance();
    for (std::size_t particleIndex = 0; particleIndex < SparkCount + SquareCount; ++particleIndex)
    {
        _entity.particles.push_back(particles.addParticle(makeParticle(particleIndex, true)));
    }
    Entities::GetInstance().AddEntity(_entity);
    SDL_HideCursor();
}

void TestScene::handleEvents(const SDL_Event& event)
{
    if (_input)
    {
        _input->handleEvent(event);
    }
    if (event.type == SDL_EVENT_MOUSE_MOTION)
    {
        SDL_RenderCoordinatesFromWindow(Renderer::getInstance().getRenderer(),
            event.motion.x, event.motion.y, &_mousePosition.x, &_mousePosition.y);
    }
}

void TestScene::handleInput()
{
    if (_input && _input->isKeyPressed(SDL_SCANCODE_ESCAPE))
    {
        _input->quit();
    }
}

void TestScene::update(float deltaTime)
{
    if (_input)
    {
        _input->update();
    }
    _elapsed += deltaTime;
    _entity.x = static_cast<int>(_mousePosition.x);
    _entity.y = static_cast<int>(_mousePosition.y);
    Entities::GetInstance().UpdateEntity(_entity);

    auto& particles = ParticleSystem::GetInstance();
    for (std::size_t particleIndex = 0; particleIndex < _entity.particles.size(); ++particleIndex)
    {
        const auto handle = _entity.particles[particleIndex];
        if (particles.isDead(handle))
        {
            particles.setParticle(handle, makeParticle(particleIndex));
        }
    }
    render();
}

float TestScene::random(float minimum, float maximum)
{
    return std::uniform_real_distribution<float>(minimum, maximum)(_random);
}

ParticleData TestScene::makeParticle(std::size_t particleIndex, bool prewarm)
{
    const bool spark = particleIndex < SparkCount;
    ParticleData particle;
    particle.positionX = _mousePosition.x + random(-9.0f, 9.0f);
    particle.positionY = _mousePosition.y + random(-8.0f, 10.0f);
    particle.texture = spark ? _sheet : nullptr;
    particle.type = spark ? Core::Particle::ParticleAnimation::Spark : Core::Particle::ParticleAnimation::PulseSquare;
    particle.velocityX = spark ? random(-65.0f, 65.0f) : random(-32.0f, 32.0f);
    particle.velocityY = spark ? random(-115.0f, -30.0f) : random(-65.0f, -15.0f);
    particle.lifetime = spark ? random(0.35f, 1.1f) : random(0.65f, 1.5f);
    if (prewarm)
    {
        particle.age = random(0.0f, particle.lifetime * 0.9f);
        particle.positionX += particle.velocityX * particle.age;
        particle.positionY += particle.velocityY * particle.age;
    }
    return particle;
}

void TestScene::renderParticles()
{
    auto& renderer = Renderer::getInstance();
    SDL_Renderer* nativeRenderer = renderer.getRenderer();
    SDL_SetRenderDrawBlendMode(nativeRenderer, SDL_BLENDMODE_ADD);
    SDL_Texture* texture = _sheet->getTexture().get();
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_ADD);
    auto& particles = ParticleSystem::GetInstance();

    for (const auto handle : _entity.particles)
    {
        const auto particle = particles.getParticle(handle);
        if (!particle || !particle->visible)
        {
            continue;
        }
        const float remaining = std::clamp(1.0f - particle->age / particle->lifetime, 0.0f, 1.0f);
        if (particle->type == Core::Particle::ParticleAnimation::Spark)
        {
            const float size = 4.0f + 7.0f * remaining;
            const SDL_FRect destination{
                particle->positionX - size * 0.5f, particle->positionY - size * 0.5f, size, size
            };
            SDL_SetTextureColorMod(texture, 255, 205, 115);
            SDL_SetTextureAlphaMod(texture, static_cast<Uint8>(230.0f * remaining));
            const auto& source = _sheet->getSourceRect(1 + handle.id % 3);
            const double angle = static_cast<double>(particle->age * 130.0f + static_cast<float>(handle.id % 36) * 10.0f);
            renderer.drawTextureRotated(texture, destination, &source, angle);
        }
        else
        {
            const float pulse = 0.5f + 0.5f * std::sin(_elapsed * 9.0f + static_cast<float>(handle.id) * 1.618f);
            const float size = (3.0f + 6.0f * pulse) * (0.4f + remaining * 0.6f);
            const SDL_FRect square{
                particle->positionX - size * 0.5f, particle->positionY - size * 0.5f, size, size
            };
            renderer.setDrawColor({255, static_cast<Uint8>(115.0f + pulse * 120.0f), 24,
                static_cast<Uint8>(200.0f * remaining)});
            SDL_RenderFillRect(nativeRenderer, &square);
        }
    }
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureColorMod(texture, 255, 255, 255);
    SDL_SetTextureAlphaMod(texture, 255);
    SDL_SetRenderDrawBlendMode(nativeRenderer, SDL_BLENDMODE_BLEND);
}

void TestScene::render()
{
    auto& renderer = Renderer::getInstance();
    renderer.setDrawColor({16, 18, 22, 255});
    renderer.clear();
    SDL_Renderer* nativeRenderer = renderer.getRenderer();
    int width = 0;
    int height = 0;
    SDL_GetRenderOutputSize(nativeRenderer, &width, &height);
    renderer.setDrawColor({25, 28, 33, 255});
    for (int column = 0; column < width; column += 32)
    {
        SDL_RenderLine(nativeRenderer, static_cast<float>(column), 0.0f, static_cast<float>(column), static_cast<float>(height));
    }
    for (int row = 0; row < height; row += 32)
    {
        SDL_RenderLine(nativeRenderer, 0.0f, static_cast<float>(row), static_cast<float>(width), static_cast<float>(row));
    }

    renderParticles();
    if (_entity.visible && _entity.sprite)
    {
        constexpr float FlameSize = 48.0f;
        const SDL_FRect destination{
            static_cast<float>(_entity.x) - FlameSize * 0.5f,
            static_cast<float>(_entity.y) - FlameSize * 0.5f,
            FlameSize, FlameSize
        };
        renderer.drawTexture(_entity.sprite->getTexture().get(), destination, &_entity.sprite->getSourceRect(0));
    }
    if (_font)
    {
        renderer.drawText(_font, "Particles Demo", {24.0f, 18.0f}, {218, 221, 226, 255});
        renderer.drawText(_font, std::format("{} particles", _entity.particles.size()),
            {24.0f, 50.0f}, {119, 128, 139, 255});
    }

    if (!_capturePath.empty())
    {
        SDL_Surface* surface = SDL_RenderReadPixels(nativeRenderer, nullptr);
        _captureSucceeded = surface && IMG_SavePNG(surface, _capturePath.c_str());
        SDL_DestroySurface(surface);
        _capturePath.clear();
    }
    renderer.present();
}

bool TestScene::captureFrame(const std::string& path)
{
    _capturePath = path;
    _captureSucceeded = false;
    render();
    return _captureSucceeded;
}

void TestScene::cleanup()
{
    Entities::GetInstance().RemoveEntity(_entity);
    auto& particles = ParticleSystem::GetInstance();
    for (const auto handle : _entity.particles)
    {
        particles.removeParticle(handle);
    }
    _entity.particles.clear();
    _entity.sprite.reset();
    _sheet.reset();
    _font = nullptr;
    _input.reset();
    SDL_ShowCursor();
}

std::string TestScene::getName() const
{
    return "ParticlesDemoScene";
}

bool TestScene::shouldQuit() const
{
    return _input && _input->shouldQuit();
}

std::unique_ptr<Scene> TestScene::getNextScene()
{
    return {};
}