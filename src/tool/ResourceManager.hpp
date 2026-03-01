#ifndef RESOURCEMANAGER_HPP
#define RESOURCEMANAGER_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>
#include <memory>
#include <iostream>

class ResourceManager {
public:
    ResourceManager() = default;
    ~ResourceManager() = default;

    std::shared_ptr<sf::Texture> getTexture(const std::string& path) {
        auto it = textures.find(path);
        if (it != textures.end()) {
            return it->second;
        }
        auto tex = std::make_shared<sf::Texture>();
        if (!tex->loadFromFile(path)) {
            std::cerr << "ResourceManager: failed to load texture '" << path << "'" << std::endl;
            return nullptr;
        }
        textures[path] = tex;
        return tex;
    }

    std::shared_ptr<sf::Font> getFont(const std::string& path) {
        auto it = fonts.find(path);
        if (it != fonts.end()) {
            return it->second;
        }
        auto f = std::make_shared<sf::Font>();
        if (!f->loadFromFile(path)) {
            std::cerr << "ResourceManager: failed to load font '" << path << "'" << std::endl;
            return nullptr;
        }
        fonts[path] = f;
        return f;
    }

private:
    std::unordered_map<std::string, std::shared_ptr<sf::Texture>> textures;
    std::unordered_map<std::string, std::shared_ptr<sf::Font>> fonts;
};

#endif
