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
    float prevX = 0.f;
    float speed = 0.f;
    int damage = 0;
    float life = 4.0f;
    float maxLife = 4.0f;
    bool torchEnhanced = false;
};

struct MineExplosionEffect {
    int row = 0;
    float x = 0.f;
    float elapsed = 0.f;
    float duration = 0.45f;
};

struct CherryExplosionEffect {
    int row = 0;
    float x = 0.f;
    float elapsed = 0.f;
    float duration = 0.55f;
};

struct SquashAttackEffect {
    int row = 0;
    float startX = 0.f;
    float x = 0.f;
    float elapsed = 0.f;
    float duration = 0.45f;
};

struct SquashStrikePending {
    int row = 0;
    float centerX = 0.f;
    float halfWidth = 40.f;
    int damage = 0;
    float elapsed = 0.f;
    float triggerTime = 0.75f;
    bool triggered = false;
};

class PlantAttackSystem {
public:
    void reset();

    void update(float dt,
                std::vector<PlantUnit>& plants,
                std::vector<ZombieUnit>& zombies,
                const StatsDatabase& db,
                const std::function<float(const PlantUnit&)>& getPlantFireX,
                const std::function<void(int)>& addSun);

    const std::vector<BulletUnit>& bullets() const { return bulletsData; }
    const std::vector<MineExplosionEffect>& mineExplosions() const { return mineExplosionsData; }
    const std::vector<CherryExplosionEffect>& cherryExplosions() const { return cherryExplosionsData; }
    const std::vector<SquashAttackEffect>& squashAttacks() const { return squashAttacksData; }
    bool isPotatoMineArmed(int row, int col) const;
    float potatoMineArmProgress(int row, int col) const;

private:
    struct BurstState {
        int pendingShots = 0;
        float chainTimer = 0.f;
        float chainInterval = 0.f;
    };

    std::string plantKey(const PlantUnit& p) const;
    std::string plantKey(const std::string& plantId, int row, int col) const;

    std::unordered_map<std::string, float> cooldowns;
    std::unordered_map<std::string, BurstState> burstStates;
    std::unordered_map<std::string, bool> mineArmedByKey;
    std::unordered_map<std::string, float> mineArmProgressByKey;
    std::vector<BulletUnit> bulletsData;
    std::vector<MineExplosionEffect> mineExplosionsData;
    std::vector<CherryExplosionEffect> cherryExplosionsData;
    std::vector<SquashAttackEffect> squashAttacksData;
    std::vector<SquashStrikePending> squashStrikesData;
};

} // namespace matou::battle

#endif

