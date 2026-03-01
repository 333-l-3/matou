#ifndef WAVE_SPAWN_CONTROLLER_HPP
#define WAVE_SPAWN_CONTROLLER_HPP

#include "BattleSimulator.hpp"
#include <algorithm>
#include <fstream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace matou::battle {

struct WaveSpawnEntry {
    float timeSec = 0.f;
    std::string zombieId;
    int row = 0;
    float startX = 980.f;
    int count = 1;
    float intervalSec = 0.f;
};

class WaveSpawnController {
public:
    bool loadFromCsv(const std::string& csvPath) {
        entries.clear();
        runtime.clear();
        elapsedSec = 0.f;

        std::ifstream in(csvPath);
        if (!in.is_open()) return false;

        std::string line;
        while (std::getline(in, line)) {
            trimLine(line);
            if (line.empty() || line[0] == '#') continue;

            auto cols = splitCsv(line);
            if (cols.size() < 6) continue;
            if (cols[0] == "time_sec") continue;

            WaveSpawnEntry e;
            e.timeSec = toFloat(cols[0], 0.f);
            e.zombieId = cols[1];
            e.row = toInt(cols[2], 0);
            e.startX = toFloat(cols[3], 980.f);
            e.count = std::max(1, toInt(cols[4], 1));
            e.intervalSec = std::max(0.f, toFloat(cols[5], 0.f));
            if (!e.zombieId.empty()) entries.push_back(e);
        }

        std::sort(entries.begin(), entries.end(), [](const WaveSpawnEntry& a, const WaveSpawnEntry& b) {
            return a.timeSec < b.timeSec;
        });

        runtime.resize(entries.size());
        return !entries.empty();
    }

    void reset() {
        elapsedSec = 0.f;
        runtime.assign(entries.size(), RuntimeState{});
    }

    void setRandomSeed(unsigned int seed) {
        rng.seed(seed);
    }

    void update(float dt, BattleSimulator& sim, int lawnRows) {
        elapsedSec += dt;

        for (size_t i = 0; i < entries.size(); ++i) {
            const auto& e = entries[i];
            auto& st = runtime[i];

            if (st.spawnedCount >= e.count) continue;
            if (elapsedSec < e.timeSec) continue;

            if (st.spawnedCount == 0) {
                spawnOne(sim, e, lawnRows);
                st.spawnedCount = 1;
                st.cooldown = e.intervalSec;
                continue;
            }

            if (e.intervalSec <= 0.f) {
                while (st.spawnedCount < e.count) {
                    spawnOne(sim, e, lawnRows);
                    st.spawnedCount++;
                }
                continue;
            }

            st.cooldown -= dt;
            while (st.spawnedCount < e.count && st.cooldown <= 0.f) {
                spawnOne(sim, e, lawnRows);
                st.spawnedCount++;
                st.cooldown += e.intervalSec;
            }
        }
    }

    bool empty() const { return entries.empty(); }

private:
    struct RuntimeState {
        int spawnedCount = 0;
        float cooldown = 0.f;
    };

    static void trimLine(std::string& s) {
        while (!s.empty() && (s.back() == '\r' || s.back() == '\n' || s.back() == ' ' || s.back() == '\t')) s.pop_back();
        size_t b = 0;
        while (b < s.size() && (s[b] == ' ' || s[b] == '\t')) ++b;
        if (b > 0) s = s.substr(b);
    }

    static std::vector<std::string> splitCsv(const std::string& line) {
        std::vector<std::string> cols;
        std::stringstream ss(line);
        std::string cell;
        while (std::getline(ss, cell, ',')) {
            trimLine(cell);
            cols.push_back(cell);
        }
        return cols;
    }

    static int toInt(const std::string& s, int fallback) {
        try { return std::stoi(s); } catch (...) { return fallback; }
    }

    static float toFloat(const std::string& s, float fallback) {
        try { return std::stof(s); } catch (...) { return fallback; }
    }

    void spawnOne(BattleSimulator& sim, const WaveSpawnEntry& e, int lawnRows) {
        int row = e.row;
        if (row < 0) {
            int maxRow = std::max(1, lawnRows);
            std::uniform_int_distribution<int> dist(0, maxRow - 1);
            row = dist(rng);
        }
        row = std::clamp(row, 0, std::max(0, lawnRows - 1));
        sim.spawnZombie(e.zombieId, row, e.startX);
    }

private:
    std::vector<WaveSpawnEntry> entries;
    std::vector<RuntimeState> runtime;
    float elapsedSec = 0.f;
    std::mt19937 rng{1337u};
};

} // namespace matou::battle

#endif
