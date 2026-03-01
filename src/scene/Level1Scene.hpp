#ifndef LEVEL1SCENE_HPP
#define LEVEL1SCENE_HPP

#include "Scene.hpp"
#include "..\\battle\\BattleSimulator.hpp"
#include "..\\battle\\PlantAttackSystem.hpp"
#include "..\\battle\\WaveSpawnController.hpp"
#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

class SceneManager;

class Level1Scene : public Scene {
public:
    Level1Scene(SceneManager* mgr);
    virtual ~Level1Scene();

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderTarget& target) override;
    bool usesWindowBackground() const override;

private:
    struct PlantOption {
        std::string id;
        std::string name;
        std::string imageRelPath;
        std::shared_ptr<sf::Texture> texture;
    };

    struct PlacedPlant {
        int optionIndex = -1;
        int row = 0;
        int col = 0;
    };

    void rebuildLayout(sf::Vector2u viewSize);
    bool findCellByPoint(const sf::Vector2f& p, int& outRow, int& outCol) const;
    int hitPlantCard(const sf::Vector2f& p) const;
    void clampOffset(float viewW, float imgW);
    PlacedPlant* findPlacedPlant(int row, int col);

    SceneManager* manager;
    std::shared_ptr<sf::Texture> bgTexture;
    sf::Sprite bgSprite;
    bool rightDragging = false;
    float lastMouseX = 0.f;
    float viewOffsetX = 0.f;

    std::vector<PlantOption> plantOptions;
    std::vector<sf::FloatRect> plantCardRects;
    sf::FloatRect plantBarRect;

    bool draggingPlant = false;
    int draggingPlantIndex = -1;
    sf::Vector2f dragMousePos = {0.f, 0.f};

    std::vector<PlacedPlant> placedPlants;

    matou::battle::BattleSimulator battleSim;
    matou::battle::PlantAttackSystem attackSystem;
    matou::battle::WaveSpawnController waveSpawner;
    bool waveSpawnerEnabled = false;
    std::unordered_map<std::string, std::vector<std::shared_ptr<sf::Texture>>> zombieWalkFramesById;
    std::unordered_map<std::string, std::vector<std::shared_ptr<sf::Texture>>> zombieDieFramesById;
    std::unordered_map<std::string, std::vector<std::shared_ptr<sf::Texture>>> plantIdleFramesById;
    std::shared_ptr<sf::Texture> bulletTexture;
    std::shared_ptr<sf::Texture> bulletSnowTexture;
    std::unordered_map<std::string, float> zombieAnimTimeById;
    std::unordered_map<std::string, int> zombieAnimFrameById;
    std::unordered_map<std::string, float> plantAnimTimeById;
    std::unordered_map<std::string, int> plantAnimFrameById;

    sf::Vector2u cachedViewSize = {1000u, 600u};
    float cachedScale = 1.f;
    float cachedImageWidth = 0.f;
    sf::FloatRect lawnRect;
    float lawnCellW = 0.f;
    float lawnCellH = 0.f;
    std::vector<sf::Vector2f> gridPoints;

    static constexpr int lawnRows = 5;
    static constexpr int lawnCols = 9;
    static constexpr int plantableStartCol = 0;
    static constexpr int plantableEndCol = 8;
};

#endif





