#ifndef LOADOUTSCENE_HPP
#define LOADOUTSCENE_HPP

#include "Scene.hpp"
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include <string>

class SceneManager;

class LoadoutScene : public Scene {
public:
    LoadoutScene(SceneManager* mgr, int levelId);
    virtual ~LoadoutScene();

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderTarget& target) override;
    bool usesWindowBackground() const override;

private:
    struct PlantDef {
        std::string name;
        std::string imageRelPath;
        std::shared_ptr<sf::Texture> texture;
    };

    void buildLayout(sf::Vector2u viewSize);
    bool isSelected(int plantId) const;

    SceneManager* manager;
    int levelId = 1;

    std::vector<PlantDef> allPlants;
    std::vector<int> selectedPlants;
    std::vector<sf::FloatRect> plantRects;

    sf::FloatRect backRect;
    sf::FloatRect startRect;
    sf::FloatRect clearRect;
    sf::FloatRect randomRect;
    sf::FloatRect panelRect;

    float scrollX = 0.f;
    float maxScrollX = 0.f;
    bool panelDragging = false;
    float panelDragStartMouseX = 0.f;
    float panelDragStartScrollX = 0.f;

    sf::Text titleText;
    sf::Text tipText;
};

#endif
