#include <array>
#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

#include <SDL3_image/SDL_image.h>

#include "Config/Config.h"
#include "Core/ParticleSystem/ParticleSystem.h"
#include "Entities/Entities.h"
#include "Game/Game.h"
#include "TestScene.h"

namespace
{
    void smokeTest(TestScene& scene, const std::filesystem::path& outputPrefix)
    {
        if (!outputPrefix.parent_path().empty())
        {
            std::filesystem::create_directories(outputPrefix.parent_path());
        }
        const std::array<SDL_FPoint, 3> positions{{{160.0f, 320.0f}, {480.0f, 160.0f}, {800.0f, 440.0f}}};
        auto& particles = ParticleSystem::GetInstance();
        for (std::size_t stage = 0; stage < positions.size(); ++stage)
        {
            SDL_Event motion{};
            motion.type = SDL_EVENT_MOUSE_MOTION;
            motion.motion.x = positions[stage].x;
            motion.motion.y = positions[stage].y;
            scene.handleEvents(motion);
            for (int frameIndex = 0; frameIndex < 90; ++frameIndex)
            {
                SDL_PumpEvents();
                scene.handleInput();
                scene.update(1.0f / 60.0f);
                particles.update(1.0f / 60.0f);
            }

            const auto entity = Entities::GetInstance().GetEntity("mouse_flame");
            if (entity.x != static_cast<int>(positions[stage].x) || entity.y != static_cast<int>(positions[stage].y)
                || entity.particles.size() != 128 || particles.getParticleCount() != 128)
            {
                throw std::runtime_error("Mouse-following entity or bounded particle pool check failed");
            }
            std::size_t sparks = 0;
            std::size_t squares = 0;
            std::size_t moving = 0;
            for (const auto handle : entity.particles)
            {
                const auto particle = particles.getParticle(handle);
                if (!particle)
                {
                    throw std::runtime_error("The demo retained a stale particle handle");
                }
                sparks += particle->type == Core::Particle::ParticleAnimation::Spark;
                squares += particle->type == Core::Particle::ParticleAnimation::PulseSquare;
                moving += particle->positionY < static_cast<float>(entity.y) - 5.0f;
            }
            if (sparks != 96 || squares != 32 || moving < 16)
            {
                throw std::runtime_error("Particle type or central simulation check failed");
            }

            const auto path = outputPrefix.string() + "-" + std::to_string(stage) + ".png";
            if (!scene.captureFrame(path))
            {
                throw std::runtime_error(std::string("Unable to capture particle demo: ") + SDL_GetError());
            }
            const std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)> surface(IMG_Load(path.c_str()), SDL_DestroySurface);
            if (!surface)
            {
                throw std::runtime_error("Unable to read the captured particle demo");
            }
            std::size_t warmPixels = 0;
            for (int pixelY = 0; pixelY < surface->h; ++pixelY)
            {
                for (int pixelX = 0; pixelX < surface->w; ++pixelX)
                {
                    Uint8 red = 0;
                    Uint8 green = 0;
                    Uint8 blue = 0;
                    Uint8 alpha = 0;
                    SDL_ReadSurfacePixel(surface.get(), pixelX, pixelY, &red, &green, &blue, &alpha);
                    warmPixels += red > 140 && red > green + 8 && green > 40 && blue < 120;
                }
            }
            if (warmPixels < 80)
            {
                throw std::runtime_error("The particle demo screenshot is blank or missing the fire effects");
            }
            std::cout << "PASS stage " << stage << ": mouse=(" << entity.x << ',' << entity.y
                      << "), sparks=" << sparks << ", squares=" << squares << ", warm_pixels=" << warmPixels << '\n';
        }
        scene.cleanup();
        if (particles.getParticleCount() != 0)
        {
            throw std::runtime_error("Particle demo cleanup left live particles");
        }
        std::cout << "PASS: demo cleanup\n";
    }
}

int main(int argumentCount, char** arguments)
{
    try
    {
        Config::getInstance().loadConfig();
        if (Config::getInstance().getConfigData().hasError)
        {
            std::cerr << "ParticlesDemo configuration could not be loaded\n";
            return 1;
        }

        auto scene = std::make_unique<TestScene>();
        TestScene* activeScene = scene.get();
        Game game(std::move(scene));
        if (!game.init())
        {
            std::cerr << "ParticlesDemo initialization failed: " << SDL_GetError() << '\n';
            return 1;
        }
        if (argumentCount > 1 && std::string_view(arguments[1]) == "--smoke-test")
        {
            smokeTest(*activeScene, argumentCount > 2 ? std::filesystem::path(arguments[2]) : "particles-demo");
        }
        else
        {
            game.run();
        }
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "ParticlesDemo failed: " << error.what() << '\n';
        return 1;
    }
}