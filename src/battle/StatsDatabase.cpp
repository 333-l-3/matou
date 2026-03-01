#include "StatsDatabase.hpp"

#include <fstream>
#include <sstream>
#include <vector>

namespace matou::battle {
namespace {

std::string trim(const std::string& s) {
    size_t b = 0;
    while (b < s.size() && (s[b] == ' ' || s[b] == '\t' || s[b] == '\r' || s[b] == '\n')) ++b;
    size_t e = s.size();
    while (e > b && (s[e - 1] == ' ' || s[e - 1] == '\t' || s[e - 1] == '\r' || s[e - 1] == '\n')) --e;
    return s.substr(b, e - b);
}

std::vector<std::string> splitCsvLine(const std::string& line) {
    std::vector<std::string> cols;
    std::stringstream ss(line);
    std::string cell;
    while (std::getline(ss, cell, ',')) cols.push_back(trim(cell));
    return cols;
}

bool shouldSkipLine(const std::string& line) {
    std::string t = trim(line);
    return t.empty() || t[0] == '#';
}

} // namespace

bool StatsDatabase::loadFromFiles(const std::string& plantStatsCsv,
                                  const std::string& zombieStatsCsv,
                                  const std::string& zombieAnimCsv) {
    plantsById.clear();
    zombiesById.clear();
    zombieAnimsById.clear();

    return loadPlantStats(plantStatsCsv)
        && loadZombieStats(zombieStatsCsv)
        && loadZombieAnims(zombieAnimCsv);
}

const PlantStats* StatsDatabase::getPlant(const std::string& id) const {
    auto it = plantsById.find(id);
    return (it == plantsById.end()) ? nullptr : &it->second;
}

const ZombieStats* StatsDatabase::getZombie(const std::string& id) const {
    auto it = zombiesById.find(id);
    return (it == zombiesById.end()) ? nullptr : &it->second;
}

const ZombieAnimationSet* StatsDatabase::getZombieAnim(const std::string& id) const {
    auto it = zombieAnimsById.find(id);
    return (it == zombieAnimsById.end()) ? nullptr : &it->second;
}

bool StatsDatabase::loadPlantStats(const std::string& csvPath) {
    std::ifstream in(csvPath);
    if (!in.is_open()) return false;

    std::string line;
    while (std::getline(in, line)) {
        if (shouldSkipLine(line)) continue;

        auto c = splitCsvLine(line);
        if (c.size() < 6) continue;
        if (c[0] == "id") continue;

        PlantStats s;
        s.id = c[0];
        s.name = c[1];
        s.sunCost = std::stoi(c[2]);
        s.hp = std::stoi(c[3]);
        s.attack = std::stoi(c[4]);
        s.attackInterval = std::stof(c[5]);

        plantsById[s.id] = s;
    }
    return !plantsById.empty();
}

bool StatsDatabase::loadZombieStats(const std::string& csvPath) {
    std::ifstream in(csvPath);
    if (!in.is_open()) return false;

    std::string line;
    while (std::getline(in, line)) {
        if (shouldSkipLine(line)) continue;

        auto c = splitCsvLine(line);
        if (c.size() < 6) continue;
        if (c[0] == "id") continue;

        ZombieStats s;
        s.id = c[0];
        s.name = c[1];
        s.hp = std::stoi(c[2]);
        s.speed = std::stof(c[3]);
        s.attack = std::stoi(c[4]);
        s.attackInterval = std::stof(c[5]);

        zombiesById[s.id] = s;
    }
    return !zombiesById.empty();
}

bool StatsDatabase::loadZombieAnims(const std::string& csvPath) {
    std::ifstream in(csvPath);
    if (!in.is_open()) return false;

    std::string line;
    while (std::getline(in, line)) {
        if (shouldSkipLine(line)) continue;

        auto c = splitCsvLine(line);
        if (c.size() < 4) continue;
        if (c[0] == "id") continue;

        ZombieAnimationSet s;
        s.id = c[0];
        s.walkGif = c[1];
        s.attackGif = c[2];
        s.dieGif = c[3];

        zombieAnimsById[s.id] = s;
    }
    return !zombieAnimsById.empty();
}

} // namespace matou::battle

