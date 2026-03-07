#ifndef PLANT_CARD_CATALOG_HPP
#define PLANT_CARD_CATALOG_HPP

#include "file.hpp"

#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace matou::scene::catalog {

struct PlantCardDef {
    std::string id;
    std::string name;
    std::string imageRelPath;
    int sunCost = 0;
    std::string desc;
};

namespace detail {

struct PlantCardExtra {
    const char* imageRelPath;
    const char* desc;
    const char* fallbackName;
};

inline const std::unordered_map<std::string, PlantCardExtra>& extraById() {
    static const std::unordered_map<std::string, PlantCardExtra> kData = {
        {"peashooter", {"res\\images\\Plants\\Peashooter\\0.gif", "基础射手：持续发射豌豆", "豌豆射手"}},
        {"sunflower", {"res\\images\\Plants\\SunFlower\\0.gif", "生产阳光：稳定补充资源", "向日葵"}},
        {"wallnut", {"res\\images\\Plants\\WallNut\\0.gif", "防御植物：高生命值阻挡前进", "坚果墙"}},
        {"potatomine", {"res\\images\\Plants\\PotatoMine\\0.gif", "埋地后进入武装，接触触发爆炸", "土豆雷"}},
        {"snowpea", {"res\\images\\Plants\\SnowPea\\0.gif", "冰豌豆：命中后减速僵尸", "寒冰射手"}},
        {"cherrybomb", {"res\\images\\Plants\\CherryBomb\\0.gif", "范围爆炸：短延迟后清场", "樱桃炸弹"}},
        {"repeater", {"res\\images\\Plants\\Repeater\\0.gif", "双弹连发：中高持续输出", "双发射手"}},
        {"scaredyshroom", {"res\\images\\Plants\\ScaredyShroom\\0.gif", "低消耗远程：发射小菇孢子", "胆小菇"}},
        {"fume", {"res\\images\\Plants\\FumeShroom\\0.gif", "短程喷射：同排范围伤害", "胆大菇"}},
        {"chomper", {"res\\images\\Plants\\Chomper\\0.gif", "近战吞噬：一口秒杀前方目标", "大嘴花"}},
        {"wsdr", {"res\\images\\Plants\\GatlingPea\\0.gif", "高速四连发：高费高输出", "维什戴尔"}},
        {"threepeater", {"res\\images\\Plants\\Threepeater\\0.gif", "三向射击：同时覆盖上中下三行", "三线射手"}},
        {"tallnut", {"res\\images\\Plants\\TallNut\\0.gif", "重型防线：更高生命值的墙体", "高坚果"}},
        {"squash", {"res\\images\\Plants\\Squash\\Squash.gif", "近距压杀：触发后重击并自毁", "倭瓜"}},
        {"torchwood", {"res\\images\\Plants\\Torchwood\\0.gif", "火炬树桩：穿过后子弹强化并增伤", "火炬树桩"}},
    };
    return kData;
}

inline std::string trim(const std::string& s) {
    size_t b = 0;
    while (b < s.size() && (s[b] == ' ' || s[b] == '\t' || s[b] == '\r' || s[b] == '\n')) ++b;
    size_t e = s.size();
    while (e > b && (s[e - 1] == ' ' || s[e - 1] == '\t' || s[e - 1] == '\r' || s[e - 1] == '\n')) --e;
    return s.substr(b, e - b);
}

inline std::vector<std::string> splitCsvLine(const std::string& line) {
    std::vector<std::string> cols;
    std::stringstream ss(line);
    std::string cell;
    while (std::getline(ss, cell, ',')) cols.push_back(trim(cell));
    return cols;
}

inline bool shouldSkipLine(const std::string& line) {
    const std::string t = trim(line);
    return t.empty() || t[0] == '#';
}

inline std::vector<PlantCardDef> fallbackCards() {
    static const char* kOrder[] = {
        "peashooter", "sunflower", "wallnut", "potatomine", "snowpea", "cherrybomb",
        "repeater", "scaredyshroom", "fume", "chomper", "wsdr", "threepeater", "tallnut",
        "squash", "torchwood"
    };
    std::vector<PlantCardDef> out;
    out.reserve(sizeof(kOrder) / sizeof(kOrder[0]));
    for (const char* id : kOrder) {
        auto it = extraById().find(id);
        if (it == extraById().end()) continue;
        PlantCardDef d;
        d.id = id;
        d.name = it->second.fallbackName;
        d.imageRelPath = it->second.imageRelPath;
        d.sunCost = 0;
        d.desc = it->second.desc;
        out.push_back(d);
    }
    return out;
}

inline std::vector<PlantCardDef> loadBaseCards() {
    const std::string csvPath = matou::tool::file::resourcePath("res\\config\\plant_stats.csv");
    std::ifstream in(csvPath);
    if (!in.is_open()) return fallbackCards();

    std::vector<PlantCardDef> out;
    std::string line;
    while (std::getline(in, line)) {
        if (shouldSkipLine(line)) continue;

        auto c = splitCsvLine(line);
        if (c.size() < 6) continue;
        if (c[0] == "id") continue;

        PlantCardDef d;
        d.id = c[0];
        d.name = c[1];
        d.sunCost = std::stoi(c[2]);

        auto it = extraById().find(d.id);
        if (it != extraById().end()) {
            d.imageRelPath = it->second.imageRelPath;
            d.desc = it->second.desc;
            if (d.name.empty()) d.name = it->second.fallbackName;
        }
        out.push_back(d);
    }

    if (out.empty()) return fallbackCards();
    return out;
}

} // namespace detail

inline const std::vector<PlantCardDef>& basePlantCards() {
    static const std::vector<PlantCardDef> defs = detail::loadBaseCards();
    return defs;
}

} // namespace matou::scene::catalog

#endif