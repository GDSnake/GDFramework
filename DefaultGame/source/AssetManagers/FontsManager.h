#pragma once

#include <string>
#include <unordered_map>

struct TTF_Font;

class FontsManager
{
public:
    static FontsManager& getInstance()
    {
        static FontsManager instance;
        return instance;
    }

    FontsManager(const FontsManager&) = delete;
    FontsManager& operator=(const FontsManager&) = delete;

    void load();
    TTF_Font* getFont(const std::string& name) const noexcept;
    void clear();

private:
    FontsManager() = default;
    ~FontsManager();

    std::unordered_map<std::string, TTF_Font*> _fonts;
};