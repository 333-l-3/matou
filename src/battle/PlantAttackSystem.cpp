#include "PlantAttackSystem.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_set>

namespace matou::battle {

void PlantAttackSystem::reset() {
    cooldowns.clear();
    burstStates.clear();
    mineArmedByKey.clear();
    mineArmProgressByKey.clear();
    bulletsData.clear();
    mineExplosionsData.clear();
    cherryExplosionsData.clear();
    squashAttacksData.clear();
    squashStrikesData.clear();
}

std::string PlantAttackSystem::plantKey(const PlantUnit& p) const {
    return p.plantId + "_" + std::to_string(p.row) + "_" + std::to_string(p.col);
}

std::string PlantAttackSystem::plantKey(const std::string& plantId, int row, int col) const {
    return plantId + "_" + std::to_string(row) + "_" + std::to_string(col);
}

bool PlantAttackSystem::isPotatoMineArmed(int row, int col) const {
    const std::string key = plantKey("potatomine", row, col);
    auto it = mineArmedByKey.find(key);
    return it != mineArmedByKey.end() && it->second;
}

float PlantAttackSystem::potatoMineArmProgress(int row, int col) const {
    const std::string key = plantKey("potatomine", row, col);
    auto it = mineArmProgressByKey.find(key);
    if (it == mineArmProgressByKey.end()) return 1.f;
    return std::clamp(it->second, 0.f, 1.f);
}

void PlantAttackSystem::update(float dt,
                               std::vector<PlantUnit>& plants,
                               std::vector<ZombieUnit>& zombies,
                               const StatsDatabase& db,
                               const std::function<float(const PlantUnit&)>& getPlantFireX,
                               const std::function<void(int)>& addSun) {
    std::vector<std::pair<int, float>> torchwoods;
    torchwoods.reserve(plants.size());
    for (const auto& p : plants) {
        if (p.plantId != "torchwood") continue;
        torchwoods.emplace_back(p.row, getPlantFireX(p));
    }

    std::unordered_set<std::string> livePlantKeys;
    livePlantKeys.reserve(plants.size());
    for (const auto& p : plants) {
        livePlantKeys.insert(plantKey(p));
    }

    for (auto it = cooldowns.begin(); it != cooldowns.end();) {
        if (livePlantKeys.find(it->first) == livePlantKeys.end()) it = cooldowns.erase(it);
        else ++it;
    }
    for (auto it = burstStates.begin(); it != burstStates.end();) {
        if (livePlantKeys.find(it->first) == livePlantKeys.end()) it = burstStates.erase(it);
        else ++it;
    }
    for (auto it = mineArmedByKey.begin(); it != mineArmedByKey.end();) {
        if (livePlantKeys.find(it->first) == livePlantKeys.end()) it = mineArmedByKey.erase(it);
        else ++it;
    }
    for (auto it = mineArmProgressByKey.begin(); it != mineArmProgressByKey.end();) {
        if (livePlantKeys.find(it->first) == livePlantKeys.end()) it = mineArmProgressByKey.erase(it);
        else ++it;
    }

    for (auto& p : plants) {
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
                CherryExplosionEffect fx;
                fx.row = p.row;
                fx.x = cx;
                fx.elapsed = 0.f;
                fx.duration = 0.55f;
                cherryExplosionsData.push_back(fx);
                p.hp = 0;
                cd = -999.f;
            }
            continue;
        }

        const PlantStats* stats = db.getPlant(p.plantId);
        if (!stats) continue;

        if (p.plantId == "sunflower") {
            std::string key = plantKey(p);
            float& cd = cooldowns[key];
            cd += dt;
            constexpr float kSunInterval = 12.0f;
            constexpr int kSunPerTick = 25;
            while (cd >= kSunInterval) {
                cd -= kSunInterval;
                if (addSun) addSun(kSunPerTick);
            }
            continue;
        }

        if (p.plantId == "potatomine") {
            std::string key = plantKey(p);
            float& cd = cooldowns[key];
            cd += dt;

            constexpr float kArmTime = 3.0f;
            const float armProgress = std::clamp(cd / kArmTime, 0.f, 1.f);
            const bool armed = armProgress >= 1.f;
            mineArmedByKey[key] = armed;
            mineArmProgressByKey[key] = armProgress;
            if (!armed) continue;

            float fireX = getPlantFireX(p);
            bool exploded = false;
            for (auto& z : zombies) {
                if (z.dying || z.hp <= 0) continue;
                if (z.row != p.row) continue;
                const float dx = z.x - fireX;
                if (dx < -18.f || dx > 44.f) continue;
                exploded = true;
                break;
            }

            if (exploded) {
                constexpr int kMaxCol = 8;
                const int dmg = std::max(1, stats->attack);

                auto closestColForZombie = [&](const ZombieUnit& z) {
                    PlantUnit probe = p;
                    probe.row = z.row;
                    int bestCol = 0;
                    float bestDist = std::numeric_limits<float>::max();
                    for (int col = 0; col <= kMaxCol; ++col) {
                        probe.col = col;
                        float centerX = getPlantFireX(probe);
                        float dist = std::fabs(z.x - centerX);
                        if (dist < bestDist) {
                            bestDist = dist;
                            bestCol = col;
                        }
                    }
                    return bestCol;
                };

                for (auto& z : zombies) {
                    if (z.dying || z.hp <= 0) continue;
                    if (std::abs(z.row - p.row) > 1) continue;

                    const int zCol = closestColForZombie(z);
                    const bool inCross = (z.row == p.row && std::abs(zCol - p.col) <= 1) ||
                                         (zCol == p.col && std::abs(z.row - p.row) <= 1);
                    if (!inCross) continue;

                    z.hp -= dmg;
                    if (z.hp <= 0) {
                        z.hp = 0;
                        z.dying = true;
                        z.deathTimer = 0.f;
                    }
                }

                MineExplosionEffect fx;
                fx.row = p.row;
                fx.x = fireX;
                fx.elapsed = 0.f;
                fx.duration = 0.45f;
                mineExplosionsData.push_back(fx);

                p.hp = 0;
                cd = -999.f;
                mineArmedByKey[key] = false;
                mineArmProgressByKey[key] = 1.f;
            }
            continue;
        }

        if (p.plantId == "squash") {
            std::string key = plantKey(p);
            float& cd = cooldowns[key];
            cd += dt;

            float selfX = getPlantFireX(p);
            PlantUnit probe = p;
            const int targetCol = std::clamp(p.col + 1, 0, 8);
            probe.col = targetCol;
            const float targetX = getPlantFireX(probe);
            probe.col = std::clamp(targetCol - 1, 0, 8);
            const float prevX = getPlantFireX(probe);
            probe.col = std::clamp(targetCol + 1, 0, 8);
            const float nextX = getPlantFireX(probe);
            const float leftBound = (prevX + targetX) * 0.5f;
            const float rightBound = (targetX + nextX) * 0.5f;

            bool hasTargetInFrontCell = false;
            for (auto& z : zombies) {
                if (z.dying || z.hp <= 0) continue;
                if (z.row != p.row) continue;
                if (z.x < leftBound || z.x > rightBound) continue;
                hasTargetInFrontCell = true;
                break;
            }

            constexpr float kWindup = 0.62f;
            if (hasTargetInFrontCell && cd >= kWindup) {
                SquashAttackEffect fx;
                fx.row = p.row;
                fx.startX = selfX;
                fx.x = targetX;
                fx.elapsed = 0.f;
                fx.duration = 1.05f;
                squashAttacksData.push_back(fx);

                SquashStrikePending strike;
                strike.row = p.row;
                strike.centerX = targetX;
                strike.halfWidth = std::max(20.f, (rightBound - leftBound) * 0.5f);
                strike.damage = std::max(1, stats->attack);
                strike.elapsed = 0.f;
                strike.triggerTime = fx.duration * 0.92f;
                strike.triggered = false;
                squashStrikesData.push_back(strike);

                p.hp = 0;
                cd = -999.f;
            }
            continue;
        }

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

        bool shooter =
            p.plantId == "peashooter" ||
            p.plantId == "snowpea" ||
            p.plantId == "repeater" ||
            p.plantId == "scaredyshroom" ||
            p.plantId == "fume" ||
            p.plantId == "wsdr" ||
            p.plantId == "threepeater";
        if (!shooter) continue;

        float fireX = getPlantFireX(p);
        bool hasAhead = false;
        for (const auto& z : zombies) {
            if (z.dying || z.hp <= 0) continue;
            if (p.plantId == "threepeater") {
                if (std::abs(z.row - p.row) > 1) continue;
            } else {
                if (z.row != p.row) continue;
            }
            if (z.x <= fireX + 6.f) continue;
            if (p.plantId == "fume" && z.x - fireX > 320.f) continue;
            hasAhead = true;
            break;
        }

        std::string key = plantKey(p);
        float& cd = cooldowns[key];
        cd += dt;

        float interval = std::max(0.08f, stats->attackInterval);
        if (p.plantId == "wsdr") interval *= 0.6f;

        auto emitShot = [&](int row) {
            BulletUnit b1;
            b1.sourcePlantId = p.plantId;
            b1.row = row;
            b1.x = fireX + 20.f;
            b1.prevX = b1.x;
            b1.speed = (p.plantId == "snowpea") ? 240.f : 260.f;
            b1.damage = stats->attack;
            if (p.plantId == "fume") {
                b1.speed = 0.f;
                b1.damage = 0;
                b1.life = 0.32f;
                b1.maxLife = b1.life;

                const float left = fireX - 20.f;
                const float right = fireX + 300.f;
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
            if (p.plantId == "threepeater") {
                for (int rr = std::max(0, p.row - 1); rr <= std::min(4, p.row + 1); ++rr) {
                    BulletUnit b = b1;
                    b.row = rr;
                    bulletsData.push_back(b);
                }
            } else {
                bulletsData.push_back(b1);
            }
        };

        BurstState& burst = burstStates[key];
        if (burst.pendingShots > 0) {
            burst.chainTimer += dt;
            while (burst.pendingShots > 0 && burst.chainTimer >= burst.chainInterval) {
                burst.chainTimer -= burst.chainInterval;
                emitShot(p.row);
                --burst.pendingShots;
            }
        }

        int burstCount = 1;
        float chainInterval = 0.f;
        if (p.plantId == "repeater") {
            burstCount = 2;
            chainInterval = 0.18f;
        } else if (p.plantId == "wsdr") {
            burstCount = 4;
            chainInterval = 0.13f;
        }

        if (hasAhead && burst.pendingShots <= 0 && cd >= interval) {
            emitShot(p.row);
            if (burstCount > 1) {
                burst.pendingShots = burstCount - 1;
                burst.chainInterval = chainInterval;
                burst.chainTimer = 0.f;
            }
            cd = 0.f;
        }
    }

    for (auto& b : bulletsData) {
        b.prevX = b.x;
        b.x += b.speed * dt;
        if (b.life > 0.f) b.life -= dt;

        const bool canTorch =
            b.sourcePlantId == "peashooter" ||
            b.sourcePlantId == "snowpea" ||
            b.sourcePlantId == "repeater" ||
            b.sourcePlantId == "wsdr" ||
            b.sourcePlantId == "threepeater";
        if (!canTorch || b.torchEnhanced) continue;

        for (const auto& tw : torchwoods) {
            if (tw.first != b.row) continue;
            const float lo = std::min(b.prevX, b.x) - 6.f;
            const float hi = std::max(b.prevX, b.x) + 6.f;
            if (tw.second < lo || tw.second > hi) continue;
            b.torchEnhanced = true;
            b.damage = std::max(b.damage * 2, b.damage + 20);
            break;
        }
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
                if (b.sourcePlantId == "snowpea" && !b.torchEnhanced) {
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

    for (auto& s : squashStrikesData) {
        s.elapsed += dt;
        if (s.triggered || s.elapsed < s.triggerTime) continue;
        const float left = s.centerX - s.halfWidth;
        const float right = s.centerX + s.halfWidth;
        for (auto& z : zombies) {
            if (z.dying || z.hp <= 0) continue;
            if (z.row != s.row) continue;
            if (z.x < left || z.x > right) continue;
            z.hp -= s.damage;
            if (z.hp <= 0) {
                z.hp = 0;
                z.dying = true;
                z.deathTimer = 0.f;
            }
        }
        s.triggered = true;
    }
    squashStrikesData.erase(
        std::remove_if(squashStrikesData.begin(), squashStrikesData.end(),
            [](const SquashStrikePending& s) { return s.elapsed >= s.triggerTime + 0.25f; }),
        squashStrikesData.end()
    );

    for (auto& fx : mineExplosionsData) {
        fx.elapsed += dt;
    }
    mineExplosionsData.erase(
        std::remove_if(mineExplosionsData.begin(), mineExplosionsData.end(),
            [](const MineExplosionEffect& fx) { return fx.elapsed >= fx.duration; }),
        mineExplosionsData.end()
    );

    for (auto& fx : cherryExplosionsData) {
        fx.elapsed += dt;
    }
    cherryExplosionsData.erase(
        std::remove_if(cherryExplosionsData.begin(), cherryExplosionsData.end(),
            [](const CherryExplosionEffect& fx) { return fx.elapsed >= fx.duration; }),
        cherryExplosionsData.end()
    );

    for (auto& fx : squashAttacksData) {
        fx.elapsed += dt;
    }
    squashAttacksData.erase(
        std::remove_if(squashAttacksData.begin(), squashAttacksData.end(),
            [](const SquashAttackEffect& fx) { return fx.elapsed >= fx.duration; }),
        squashAttacksData.end()
    );
}

} // namespace matou::battle
