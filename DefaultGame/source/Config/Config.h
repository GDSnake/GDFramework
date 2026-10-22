#pragma once

#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <unordered_map>
#include <utility>
#include <vector>

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

struct ImageSheetConfigData {
    std::string path;
    float frameWidth = 0.0f;
    float frameHeight = 0.0f;
    int columns = 0;
    int rows = 0;

    struct Frame {
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;
        float height = 0.0f;
    };

    std::vector<Frame> frames;
};

struct ConfigData {
    std::string title = "Default Game Name";
    int width = 1280;
    int height = 720;
    bool fullscreen = false;
    int targetFPS = 60;
    int startEntitiesNumber = 10;
    bool hasError = false;
    std::unordered_map<std::string, ImageConfigData> images;
    std::unordered_map<std::string, ImageSheetConfigData> imageSheets;
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

    inline void setWindowSettings(int width, int height, bool fullscreen) noexcept
    {
        if (width > 0 && height > 0)
        {
            _data.width = width;
            _data.height = height;
        }
        _data.fullscreen = fullscreen;
    }

    inline bool saveConfig() const
    {
        std::filesystem::path configPath;
        std::filesystem::path temporaryDirectory;
        std::filesystem::path temporaryPath;
        bool temporaryDirectoryCreated = false;
        bool saved = false;

        try
        {
            configPath = std::filesystem::current_path() / "config.json";
            std::ifstream input(configPath);
            if (!input)
            {
                throw std::runtime_error("Cannot open configuration for reading");
            }

            json data = json::parse(input);
            input.close();

            auto window = data.find("window");
            if (window == data.end() || !window->is_object())
            {
                throw std::runtime_error("Missing or invalid 'window' object");
            }

            (*window)["width"] = _data.width;
            (*window)["height"] = _data.height;
            (*window)["fullscreen"] = _data.fullscreen;

            const std::string serialized = data.dump(4) + '\n';
            temporaryDirectory = configPath;
            temporaryDirectory += std::format(".tmp.{}", std::chrono::steady_clock::now().time_since_epoch().count());
            temporaryPath = temporaryDirectory / configPath.filename();
            if (!std::filesystem::create_directory(temporaryDirectory))
            {
                throw std::runtime_error(std::format("Cannot reserve temporary configuration directory {}", temporaryDirectory.string()));
            }
            temporaryDirectoryCreated = true;

            std::ofstream output(temporaryPath);
            if (!output)
            {
                throw std::runtime_error(std::format("Cannot create temporary configuration {}", temporaryPath.string()));
            }

            output << serialized;
            output.close();
            if (!output)
            {
                throw std::runtime_error(std::format("Failed to write or close temporary configuration {}", temporaryPath.string()));
            }

            std::filesystem::rename(temporaryPath, configPath);
            saved = true;
        }
        catch (const std::exception& error)
        {
            std::cerr << "Configuration save failed for " << configPath << ": " << error.what() << '\n';
        }

        if (temporaryDirectoryCreated)
        {
            std::error_code cleanupError;
            std::filesystem::remove_all(temporaryDirectory, cleanupError);
            if (cleanupError)
            {
                std::cerr << "Temporary configuration cleanup failed for " << temporaryDirectory
                          << ": " << cleanupError.message() << '\n';
            }
        }
        return saved;
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

            cfg.startEntitiesNumber = data.value("start_entities_number", cfg.startEntitiesNumber);
            if (cfg.startEntitiesNumber < 0)
            {
                throw std::runtime_error("start_entities_number must not be negative");
            }

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

            if (const auto imageSheets = data.find("imageSheet");
                imageSheets != data.end())
            {
                if (!imageSheets->is_object())
                {
                    throw std::runtime_error("imageSheet must be an object");
                }
                for (auto sheet = imageSheets->begin(); sheet != imageSheets->end(); ++sheet)
                {
                    if (!sheet.value().is_object())
                    {
                        throw std::runtime_error("each imageSheet entry must be an object");
                    }

                    ImageSheetConfigData sheetConfig;
                    sheetConfig.path = sheet.value().at("path").get<std::string>();
                    sheetConfig.frameWidth = sheet.value().value("frame_width", 0.0f);
                    sheetConfig.frameHeight = sheet.value().value("frame_height", 0.0f);
                    sheetConfig.columns = sheet.value().value("columns", 0);
                    sheetConfig.rows = sheet.value().value("rows", 0);
                    if (const auto frames = sheet.value().find("frames");
                        frames != sheet.value().end())
                    {
                        if (!frames->is_array())
                        {
                            throw std::runtime_error("imageSheet frames must be an array");
                        }
                        for (const auto& frame : *frames)
                        {
                            if (!frame.is_object())
                            {
                                throw std::runtime_error("each imageSheet frame must be an object");
                            }

                            ImageSheetConfigData::Frame frameConfig;
                            frameConfig.x = frame.at("x").get<float>();
                            frameConfig.y = frame.at("y").get<float>();
                            frameConfig.width = frame.at("width").get<float>();
                            frameConfig.height = frame.at("height").get<float>();
                            sheetConfig.frames.push_back(frameConfig);
                        }
                    }
                    cfg.imageSheets[sheet.key()] = std::move(sheetConfig);
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

