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
        // 樱桃炸弹：短延迟后爆炸，打同列附近三行
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
                p.hp = 0;
                cd = -999.f;
            }
            continue;
        }

        const PlantStats* stats = db.getPlant(p.plantId);
        if (!stats) continue;

        // 大嘴花：近距离吞噬单体，按配置冷却
        if (p.plantId == "chomper") {
            float fireX = getPlantFireX(p);
            ZombieUnit* target = nullptr;
            for (auto& z : zombies) {
                if (z.dying || z.hp <= 0) continue;
                if (z.row != p.row) continue;
                float dx = z.x - fireX;
                if (dx >= 0.f && dx <= 70.f) {
                    if (!target || z.x < target->x) target = &z;
                }
            }

            std::string key = plantKey(p);
            float& cd = cooldowns[key];
            cd += dt;
            float needCd = std::max(0.8f, stats->attackInterval);

            if (target && cd >= needCd) {
                target->hp = 0;
                target->dying = true;
                target->deathTimer = 0.f;
                cd = 0.f;
            }
            continue;
        }

        // 发射类植物
        bool shooter =
            p.plantId == "peashooter" ||
            p.plantId == "snowpea" ||
            p.plantId == "repeater" ||
            p.plantId == "scaredyshroom" ||
            p.plantId == "fume" ||
            p.plantId == "wsdr";
        if (!shooter) continue;

        float fireX = getPlantFireX(p);
        bool hasAhead = false;
        for (const auto& z : zombies) {
            if (z.dying || z.hp <= 0) continue;
            if (z.row != p.row) continue;
            if (z.x <= fireX + 6.f) continue;
            if (p.plantId == "fume" && z.x - fireX > 320.f) continue; // 胆大菇四格喷射距离
            hasAhead = true;
            break;
        }

        std::string key = plantKey(p);
        float& cd = cooldowns[key];
        cd += dt;

        float interval = std::max(0.08f, stats->attackInterval);
        if (p.plantId == "wsdr") interval *= 0.6f; // 维什戴尔射速更高

        if (hasAhead && cd >= interval) {
            BulletUnit b1;
            b1.sourcePlantId = p.plantId;
            b1.row = p.row;
            b1.x = fireX + 20.f;
            b1.speed = (p.plantId == "snowpea") ? 240.f : 260.f;
            b1.damage = stats->attack;
            if (p.plantId == "fume") {
                b1.speed = 0.f;
                b1.damage = 0; // 视觉喷射，伤害在发射瞬间结算
                b1.life = 0.20f;
                b1.maxLife = b1.life;

                const float left = fireX - 20.f;
                const float right = fireX + 300.f; // 四格宽
                for (auto& z : zombies) {
                    if (z.dying || z.hp <= 0) continue;
                    if (z.row != p.row) continue;
                    if (z.x < left || z.x > right) continue;
                    z.hp -= stats->attack;
                    if (z.hp <= 0) {
                        z.hp = 0;
                        z.dying = true;
                        z.deathTimer = 0.f;
                    }
                }
            }
            if (p.plantId == "scaredyshroom") b1.speed = 250.f;
            if (p.plantId == "wsdr") b1.speed = 300.f;
            bulletsData.push_back(b1);

            if (p.plantId == "repeater") {
                BulletUnit b2 = b1;
                b2.x += 10.f;
                bulletsData.push_back(b2);
            } else if (p.plantId == "wsdr") {
                BulletUnit b2 = b1; b2.x += 8.f;  bulletsData.push_back(b2);
                BulletUnit b3 = b1; b3.x += 16.f; bulletsData.push_back(b3);
                BulletUnit b4 = b1; b4.x += 24.f; bulletsData.push_back(b4);
            }

            cd = 0.f;
        }
    }

    for (auto& b : bulletsData) {
        b.x += b.speed * dt;
        if (b.life > 0.f) b.life -= dt;
    }

    std::vector<BulletUnit> survivors;
    survivors.reserve(bulletsData.size());

    for (auto& b : bulletsData) {
        if (b.sourcePlantId == "fume") {
            if (b.life > 0.f) survivors.push_back(b);
            continue;
        }

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

        if (!hit && b.x < 1200.f && b.life > 0.f) {
            survivors.push_back(b);
        }
    }

    bulletsData.swap(survivors);
}

} // namespace matou::battle









