#ifndef STATS_DATABASE_HPP
#define STATS_DATABASE_HPP

#include "BattleTypes.hpp"
#include <string>
#include <unordered_map>

namespace matou::battle {

class StatsDatabase {
public:
    bool loadFromFiles(const std::string& plantStatsCsv,
                       const std::string& zombieStatsCsv,
                       const std::string& zombieAnimCsv);

    const PlantStats* getPlant(const std::string& id) const;
    const ZombieStats* getZombie(const std::string& id) const;
    const ZombieAnimationSet* getZombieAnim(const std::string& id) const;

    const std::unordered_map<std::string, PlantStats>& plants() const { return plantsById; }
    const std::unordered_map<std::string, ZombieStats>& zombies() const { return zombiesById; }

private:
    bool loadPlantStats(const std::string& csvPath);
    bool loadZombieStats(const std::string& csvPath);
    bool loadZombieAnims(const std::string& csvPath);

    std::unordered_map<std::string, PlantStats> plantsById;
    std::unordered_map<std::string, ZombieStats> zombiesById;
    std::unordered_map<std::string, ZombieAnimationSet> zombieAnimsById;
};

} // namespace matou::battle

#endif
