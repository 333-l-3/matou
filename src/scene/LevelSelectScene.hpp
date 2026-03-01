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
    SceneManager* manager;
    sf::Text title;
    sf::FloatRect level1Rect;
};

#endif
