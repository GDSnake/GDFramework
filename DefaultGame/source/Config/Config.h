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

struct ConfigData {
    std::string title = "Default Game Name";
    int width = 1280;
    int height = 720;
    bool fullscreen = false;
    int targetFPS = 60;
    bool hasError = false;
    std::unordered_map<std::string, std::string> images;
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

    bool loadConfig(const std::filesystem::path& filePath = "config.json")
    {
        try
        {
            std::ifstream file(filePath);
            if (!file)
            {
                throw std::runtime_error("Cannot open " + filePath.string());
            }

            const json data = json::parse(file);
            if (!data.is_object())
            {
                throw std::runtime_error("The root of config.json must be an object");
            }

            const json* window = &data;
            if (const auto it = data.find("window"); it != data.end())
            {
                if (!it->is_object())
                {
                    throw std::runtime_error("window must be an object");
                }
                window = &*it;
            }

            ConfigData config;
            config.title = window->value("title", data.value("title", config.title));
            config.width = window->value("width", data.value("window_width", data.value("width", config.width)));
            config.height = window->value("height", data.value("window_height", data.value("height", config.height)));
            config.fullscreen = window->value("fullscreen", data.value("fullscreen", config.fullscreen));
            config.targetFPS = window->value("target_fps", data.value("target_fps", config.targetFPS));

            if (config.width <= 0 || config.height <= 0)
            {
                throw std::runtime_error("window dimensions must be positive");
            }

            readResourceMap(data, "images", config.images);
            readResourceMap(data, "fonts", config.fonts);
            readResourceMap(data, "audio", config.audio);
            readResourceMap(data, "sounds", config.audio);
            readResourceMap(data, "sound", config.audio);

            _data = std::move(config);
            return true;
        }
        catch (const std::exception& error)
        {
            std::cerr << "Configuration load failed: " << error.what() << '\n';
            _data = ConfigData{};
            _data.hasError = true;
            return false;
        }
    }

private:
    Config() = default;

    static void readResourceMap(
        const json& data,
        const char* sectionName,
        std::unordered_map<std::string, std::string>& resources)
    {
        const auto section = data.find(sectionName);
        if (section == data.end())
        {
            return;
        }
        if (!section->is_object())
        {
            throw std::runtime_error(std::string(sectionName) + " must be an object");
        }

        for (auto it = section->begin(); it != section->end(); ++it)
        {
            resources[it.key()] = it.value().get<std::string>();
        }
    }

    ConfigData _data;
};