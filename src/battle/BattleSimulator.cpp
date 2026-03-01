#include "BattleSimulator.hpp"
#include <algorithm>

namespace matou::battle {

bool BattleSimulator::initialize(const std::string& plantStatsCsv,
                                 const std::string& zombieStatsCsv,
                                 const std::string& zombieAnimCsv) {
    currentSun = 50;
    plantUnits.clear();
    zombieUnits.clear();
    return db.loadFromFiles(plantStatsCsv, zombieStatsCsv, zombieAnimCsv);
}

bool BattleSimulator::placePlant(const std::string& plantId, int row, int col) {
    const PlantStats* stats = db.getPlant(plantId);
    if (!stats) return false;
    if (currentSun < stats->sunCost) return false;

    for (const auto& p : plantUnits) {
        if (p.row == row && p.col == col) return false;
    }

    currentSun -= stats->sunCost;
    PlantUnit p;
    p.plantId = plantId;
    p.row = row;
    p.col = col;
    p.hp = stats->hp;
    plantUnits.push_back(p);
    return true;
}

bool BattleSimulator::forcePlacePlant(const std::string& plantId, int row, int col) {
    for (const auto& p : plantUnits) {
        if (p.row == row && p.col == col) return false;
    }

    PlantUnit p;
    p.plantId = plantId;
    p.row = row;
    p.col = col;

    const PlantStats* stats = db.getPlant(plantId);
    p.hp = stats ? stats->hp : 300;

    plantUnits.push_back(p);
    return true;
}

bool BattleSimulator::spawnZombie(const std::string& zombieId, int row, float startX) {
    const ZombieStats* stats = db.getZombie(zombieId);
    if (!stats) return false;

    ZombieUnit z;
    z.zombieId = zombieId;
    z.row = row;
    z.x = startX;
    z.hp = stats->hp;
    z.slowTimer = 0.f;
    z.dying = false;
    z.deathTimer = 0.f;
    zombieUnits.push_back(z);
    return true;
}

void BattleSimulator::update(float dt) {
    for (auto& z : zombieUnits) {
        if (z.dying) {
            z.deathTimer += dt;
            continue;
        }

        const ZombieStats* stats = db.getZombie(z.zombieId);
        if (!stats) continue;

        if (z.slowTimer > 0.f) {
            z.slowTimer -= dt;
            if (z.slowTimer < 0.f) z.slowTimer = 0.f;
        }

        float speedScale = (z.slowTimer > 0.f) ? 0.55f : 1.f;
        z.x -= stats->speed * speedScale * dt;
    }

    zombieUnits.erase(
        std::remove_if(zombieUnits.begin(), zombieUnits.end(),
            [](const ZombieUnit& z) { return z.dying && z.deathTimer >= 0.7f; }),
        zombieUnits.end()
    );

    plantUnits.erase(
        std::remove_if(plantUnits.begin(), plantUnits.end(),
            [](const PlantUnit& p) { return p.hp <= 0; }),
        plantUnits.end()
    );
}

} // namespace matou::battle
