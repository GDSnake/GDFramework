#pragma once

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "Config/Config.h"

struct TestConfigData : ConfigData
{
    std::filesystem::path blocksFolder;
    std::vector<ImageConfigData> blockImages;
};

class TestConfig
{
public:
    static TestConfig& getInstance()
    {
        static TestConfig instance;
        return instance;
    }

    void load()
    {
        _data = TestConfigData{};
        static_cast<ConfigData&>(_data) = Config::getInstance().getConfigData();

        std::ifstream file(std::filesystem::current_path() / "config.json");
        if (!file)
        {
            throw std::runtime_error("Cannot open config.json for sample configuration");
        }

        const json data = json::parse(file);
        const json& images = data.at("images");
        _data.blocksFolder = images.at("blocks_folder").get<std::string>();

        for (const auto& entry : std::filesystem::directory_iterator(_data.blocksFolder))
        {
            if (!entry.is_regular_file() || entry.path().extension() != ".png")
            {
                continue;
            }

            ImageConfigData image;
            image.path = entry.path().generic_string();
            _data.blockImages.push_back(std::move(image));
        }
    }

    const TestConfigData& getConfigData() const noexcept
    {
        return _data;
    }

private:
    TestConfig() = default;
    TestConfigData _data;
};
