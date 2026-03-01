#ifndef SCENEMANAGER_HPP
#define SCENEMANAGER_HPP

#include "Scene.hpp"
#include <memory>
#include <SFML/Graphics.hpp>
#include "ResourceManager.hpp"
#include <iostream>
#include <vector>

class SceneManager {
private:
    std::unique_ptr<Scene> current;
    std::unique_ptr<Scene> pending;
    sf::Font* sharedFont = nullptr;
public:
    SceneManager() = default;
    ~SceneManager() = default;

    // 请求切换场景（不会立即销毁当前场景），在主循环安全点调用 applyPending()
    void requestChangeScene(std::unique_ptr<Scene> s) {
        std::cout << "SceneManager: requestChangeScene()" << std::endl;
        pending = std::move(s);
    }

    // 在主循环的安全点应用挂起的场景切换
    void applyPending() {
        if (pending) {
            std::cout << "SceneManager: applying pending scene change" << std::endl;
            if (current) {
                std::cout << "SceneManager: current scene onExit()" << std::endl;
                current->onExit();
            }
            current = std::move(pending);
            if (current) {
                std::cout << "SceneManager: new scene onEnter()" << std::endl;
                current->onEnter();
            }
        }
    }

    Scene* getCurrent() { return current.get(); }

    void handleEvent(const sf::Event& e) {
        if (current) current->handleEvent(e);
    }

    void update(float dt) {
        if (current) current->update(dt);
    }

    void render(sf::RenderTarget& target) {
        if (current) current->render(target);
    }

    void setSharedFont(sf::Font* f) { sharedFont = f; }
    sf::Font* getSharedFont() const { return sharedFont; }
    void setResourceManager(ResourceManager* r) { resourceManager = r; }
    ResourceManager* getResourceManager() const { return resourceManager; }

    void setLoadoutPlants(const std::vector<int>& ids) { loadoutPlantIds = ids; }
    const std::vector<int>& getLoadoutPlants() const { return loadoutPlantIds; }

private:
    ResourceManager* resourceManager = nullptr;
    std::vector<int> loadoutPlantIds;
};

#endif
