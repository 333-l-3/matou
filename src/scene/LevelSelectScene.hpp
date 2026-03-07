#ifndef LEVELSELECTSCENE_HPP
#define LEVELSELECTSCENE_HPP

#include "Scene.hpp"
#include <SFML/Graphics.hpp>

class SceneManager;

class LevelSelectScene : public Scene {
public:
    LevelSelectScene(SceneManager* mgr);
    virtual ~LevelSelectScene();

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderTarget& target) override;
    bool usesWindowBackground() const override;

private:
    void recalcLayout(sf::Vector2u size);

    SceneManager* manager;
    sf::Text title;
    sf::Text subtitle;
    sf::Text note;

    sf::FloatRect panelRect;
    sf::FloatRect level1Rect;
    sf::Vector2f mousePos = {0.f, 0.f};
    bool level1Hovered = false;
    float animTime = 0.f;
    float hoverBlend = 0.f;
};

#endif