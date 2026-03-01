#include "PlantAttackSystem.hpp"

#include <algorithm>
#include <cmath>

namespace matou::battle {

void PlantAttackSystem::reset() {
    cooldowns.clear();
    bulletsData.clear();
}

std::string PlantAttackSystem::plantKey(const PlantUnit& p) const {
    return p.plantId + "_" + std::to_string(p.row) + "_" + std::to_string(p.col);
}

void PlantAttackSystem::update(float dt,
                               std::vector<PlantUnit>& plants,
                               std::vector<ZombieUnit>& zombies,
                               const StatsDatabase& db,
                               const std::function<float(const PlantUnit&)>& getPlantFireX) {
    for (auto& p : plants) {
        // Ó£ÌÒÕ¨µ¯£º¶ÌÑÓ³Ùºó±¬Õ¨£¬´òÍ¬ÁÐ¸½½üÈýÐÐ
        if (p.plantId == "cherrybomb") {
            std::string key = plantKey(p);
            float& cd = cooldowns[key];
            cd += dt;

            if (cd >= 0.35f) {
                float cx = getPlantFireX(p);

                for (auto& z : zombies) {
                    if (z.dying || z.hp <= 0) continue;
                    if (std::abs(z.row - p.row) > 1) continue;
                    if (std::fabs(z.x - cx) > 130.f) continue;

                    const PlantStats* cstats = db.getPlant("cherrybomb");
                    int dmg = cstats ? cstats->attack : 1800;
                    z.hp -= dmg;
                    if (z.hp <= 0) {
                        z.hp = 0;
                        z.dying = true;
                        z.deathTimer = 0.f;
                    }

                }
                p.hp = 0; // µ½Ê±±¬Õ¨ºóÒÆ³ý£¨¼´Ê¹·¶Î§ÄÚÎÞ½©Ê¬£©
                cd = -999.f;
            }
            continue;
        }

        // Ô¶³ÌÉä»÷Ö²Îï
        if (p.plantId != "peashooter" && p.plantId != "snowpea" && p.plantId != "repeater") continue;

        const PlantStats* stats = db.getPlant(p.plantId);
        if (!stats) continue;

        float fireX = getPlantFireX(p);
        bool hasAhead = false;
        for (const auto& z : zombies) {
            if (z.dying || z.hp <= 0) continue;
            if (z.row == p.row && z.x > fireX + 6.f) {
                hasAhead = true;
                break;
            }
        }

        std::string key = plantKey(p);
        float& cd = cooldowns[key];
        cd += dt;

        if (hasAhead && cd >= stats->attackInterval) {
            BulletUnit b1;
            b1.sourcePlantId = p.plantId;
            b1.row = p.row;
            b1.x = fireX + 20.f;
            b1.speed = (p.plantId == "snowpea") ? 240.f : 260.f;
            b1.damage = stats->attack;
            bulletsData.push_back(b1);

            if (p.plantId == "repeater") {
                BulletUnit b2 = b1;
                b2.x += 10.f;
                bulletsData.push_back(b2);
            }

            cd = 0.f;
        }
    }

    for (auto& b : bulletsData) {
        b.x += b.speed * dt;
    }

    std::vector<BulletUnit> survivors;
    survivors.reserve(bulletsData.size());

    for (auto& b : bulletsData) {
        bool hit = false;
        for (auto& z : zombies) {
            if (z.dying || z.hp <= 0) continue;
            if (z.row != b.row) continue;
            if (std::fabs(z.x - b.x) <= 18.f) {
                z.hp -= b.damage;
                if (b.sourcePlantId == "snowpea") {
                    z.slowTimer = std::max(z.slowTimer, 2.5f);
                }
                if (z.hp <= 0) {
                    z.hp = 0;
                    z.dying = true;
                    z.deathTimer = 0.f;
                }
                hit = true;
                break;
            }
        }

        if (!hit && b.x < 1200.f) {
            survivors.push_back(b);
        }
    }

    bulletsData.swap(survivors);
}

} // namespace matou::battle




