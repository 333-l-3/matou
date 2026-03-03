#ifndef PLANT_ATTACK_SYSTEM_HPP
#define PLANT_ATTACK_SYSTEM_HPP

#include "BattleSimulator.hpp"
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace matou::battle {

struct BulletUnit {
    std::string sourcePlantId;
    int row = 0;
    float x = 0.f;
    float speed = 0.f;
    int damage = 0;
    float life = 4.0f;
    float maxLife = 4.0f;
};

class PlantAttackSystem {
public:
    void reset();

    void update(float dt,
                std::vector<PlantUnit>& plants,
                std::vector<ZombieUnit>& zombies,
                const StatsDatabase& db,
                const std::function<float(const PlantUnit&)>& getPlantFireX);

    const std::vector<BulletUnit>& bullets() const { return bulletsData; }

private:
    std::string plantKey(const PlantUnit& p) const;

    std::unordered_map<std::string, float> cooldowns;
    std::vector<BulletUnit> bulletsData;
};

} // namespace matou::battle

#endif

