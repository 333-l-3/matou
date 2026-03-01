#ifndef BATTLE_SIMULATOR_HPP
#define BATTLE_SIMULATOR_HPP

#include "BattleTypes.hpp"
#include "StatsDatabase.hpp"
#include <vector>
#include <string>

namespace matou::battle {

struct PlantUnit {
    std::string plantId;
    int row = 0;
    int col = 0;
    int hp = 0;
};

struct ZombieUnit {
    std::string zombieId;
    int row = 0;
    float x = 0.f;
    int hp = 0;
    float slowTimer = 0.f;
    bool dying = false;
    float deathTimer = 0.f;
};

class BattleSimulator {
public:
    bool initialize(const std::string& plantStatsCsv,
                    const std::string& zombieStatsCsv,
                    const std::string& zombieAnimCsv);

    bool placePlant(const std::string& plantId, int row, int col);
    bool forcePlacePlant(const std::string& plantId, int row, int col);
    bool spawnZombie(const std::string& zombieId, int row, float startX);

    void update(float dt);

    int sun() const { return currentSun; }
    void addSun(int v) { currentSun += v; }

    const std::vector<PlantUnit>& plants() const { return plantUnits; }
    std::vector<PlantUnit>& plantsMutable() { return plantUnits; }
    const std::vector<ZombieUnit>& zombies() const { return zombieUnits; }
    std::vector<ZombieUnit>& zombiesMutable() { return zombieUnits; }
    const StatsDatabase& stats() const { return db; }

private:
    StatsDatabase db;
    int currentSun = 50;
    std::vector<PlantUnit> plantUnits;
    std::vector<ZombieUnit> zombieUnits;
};

} // namespace matou::battle

#endif
