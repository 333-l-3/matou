#ifndef PLANTLISTSCENE_HPP
#define PLANTLISTSCENE_HPP

#include "Scene.hpp"
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include <string>

class SceneManager;

class PlantListScene : public Scene {
public:
    PlantListScene(SceneManager* mgr);
    virtual ~PlantListScene();

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderTarget& target) override;
    bool usesWindowBackground() const override;

private:
    struct PlantCard {
        std::string name;
        int sunCost = 0;
        std::string imageRelPath;
        std::string desc;
        std::shared_ptr<sf::Texture> texture;
    };

    void buildLayout(sf::Vector2u viewSize);

    SceneManager* manager;

    sf::FloatRect backRect;
    sf::FloatRect gridRect;
    std::vector<sf::FloatRect> cardRects; // base rects in "content space" (not scrolled)

    float scrollX = 0.f;
    float maxScrollX = 0.f;
    bool dragging = false;
    float dragStartMouseX = 0.f;
    float dragStartScrollX = 0.f;

    bool holdLeft = false;
    bool holdRight = false;

    bool detailOpen = false;
    int detailIndex = -1;
    sf::FloatRect detailRect;
    sf::FloatRect detailCloseRect;

    sf::Text titleText;

    std::vector<PlantCard> cards;
};

#endif
