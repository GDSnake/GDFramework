#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

enum class FontNames : uint8_t
{
    DefaultFont = 0,
    // Add font names as needed
    Total
};

namespace FontUtils
{
    inline const std::string to_string(FontNames name)
    {
        switch (name)
        {
            case FontNames::DefaultFont: return "defaultFont";

            // Add more font names as needed

            default: return "unknown";
        }
    }
}

struct ImageConfigData {
    std::string path;
    float width = 0.0f;
    float height = 0.0f;
};

struct ConfigData {
    std::string title = "Default Game Name";
    int width = 1280;
    int height = 720;
    bool fullscreen = false;
    int targetFPS = 60;
    bool hasError = false;
    std::unordered_map<std::string, ImageConfigData> images;
    std::unordered_map<std::string, std::string> fonts;
    std::unordered_map<std::string, std::string> audio;
};

class Config
{
public:
    static Config& getInstance()
    {
        static Config instance;
        return instance;
    }

    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    Config(Config&&) = delete;
    Config& operator=(Config&&) = delete;

    const ConfigData& getConfigData() const noexcept
    {
        return _data;
    }

inline void loadConfig()
{
    try
    {
        const std::string filePath = std::filesystem::current_path().generic_string();
        std::ifstream file(std::format("{}/config.json", filePath));
        if (!file)
        {
            throw std::runtime_error(std::format("Cannot open {}/config.json", filePath));
        }

        const json data = json::parse(file);
        if (!data.is_object())
        {
            throw std::runtime_error("The root of config.json must be an object");
        }

        ConfigData cfg;

        const json& window = data.at("window");
        cfg.title = window.at("title").get<std::string>();
        cfg.width = window.at("width").get<int>();
        cfg.height = window.at("height").get<int>();
        cfg.fullscreen = window.at("fullscreen").get<bool>();
        cfg.targetFPS = window.value("target_fps", cfg.targetFPS);

        if (const auto sound = data.find("sound"); sound != data.end())
        {
            if (!sound->is_object())
            {
                throw std::runtime_error("sound must be an object");
            }
            for (auto track = sound->begin(); track != sound->end(); ++track)
            {
                cfg.audio[track.key()] = track.value().get<std::string>();
            }
        }

        if (const auto images = data.find("images"); images != data.end())
        {
            if (!images->is_object())
            {
                throw std::runtime_error("images must be an object");
            }
            for (auto image = images->begin(); image != images->end(); ++image)
            {
                ImageConfigData imageConfig;
                if (image.value().is_string())
                {
                    imageConfig.path = image.value().get<std::string>();
                }
                else if (image.value().is_object())
                {
                    imageConfig.path = image.value().at("path").get<std::string>();
                    imageConfig.width = image.value().value("width", 0.0f);
                    imageConfig.height = image.value().value("height", 0.0f);
                }
                else
                {
                    throw std::runtime_error("each image must be a path or an object containing a path");
                }

                cfg.images[image.key()] = std::move(imageConfig);
            }
        }


        const json& fonts = data.at("fonts");
        for (int i = 0; i < static_cast<int>(FontNames::Total); ++i)
        {
            const std::string fontName = FontUtils::to_string(static_cast<FontNames>(i));
            const auto font = fonts.find(fontName);
            if (font == fonts.end())
            {
                throw std::runtime_error(std::format("Missing required font '{}'", fontName));
            }
            cfg.fonts[fontName] = font->get<std::string>();
        }

        _data = std::move(cfg);
    }
    catch (const std::exception& error)
    {
        std::cerr << "Configuration load failed: " << error.what() << '\n';
        _data = ConfigData{};
        _data.hasError = true;
    }
}
private:
    Config() = default;

    ~Config() = default;

    ConfigData _data;
};

