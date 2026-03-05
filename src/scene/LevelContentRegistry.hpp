#ifndef LEVEL_CONTENT_REGISTRY_HPP
#define LEVEL_CONTENT_REGISTRY_HPP

#include <string>
#include <vector>

namespace matou::scene::levelcfg {

struct PlantCardDef {
    const char* id;
    const char* name;
    const char* imageRelPath;
    int sunCost;
    const char* desc;
};

struct ZombieSpawnDef {
    const char* id;
    int row;
    float startX;
    int hpFallback;
};

struct ZombieAnimDef {
    const char* id;
    const char* walkFramePattern;
    int walkMaxFrames;
    float frameStep;
    const char* walkFallbackA;
    const char* walkFallbackB;
    const char* dieFramePattern;
    int dieMaxFrames;
    const char* dieFallback;
};

struct PlantAnimDef {
    const char* id;
    const char* framePattern;
    int maxFrames;
    float frameStep;
    const char* fallbackStatic;
};

inline const std::vector<PlantCardDef>& allPlantCards() {
    static const std::vector<PlantCardDef> defs = {
        {"peashooter", "豌豆射手", "res\\images\\Plants\\Peashooter\\0.gif", 100, "基础射手：持续发射豌豆"},
        {"sunflower", "向日葵", "res\\images\\Plants\\SunFlower\\0.gif", 50, "生产阳光：稳定补充资源"},
        {"wallnut", "坚果墙", "res\\images\\Plants\\WallNut\\0.gif", 50, "防御植物：高生命值阻挡前进"},
        {"potatomine", "土豆雷", "res\\images\\Plants\\PotatoMine\\0.gif", 25, "埋地后进入武装，接触触发爆炸"},
        {"snowpea", "寒冰射手", "res\\images\\Plants\\SnowPea\\0.gif", 175, "冰豌豆：命中后减速僵尸"},
        {"cherrybomb", "樱桃炸弹", "res\\images\\Plants\\CherryBomb\\0.gif", 150, "范围爆炸：短延迟后清场"},
        {"repeater", "双发射手", "res\\images\\Plants\\Repeater\\0.gif", 200, "双弹连发：中高持续输出"},
        {"scaredyshroom", "胆小菇", "res\\images\\Plants\\ScaredyShroom\\0.gif", 25, "低消耗远程：发射小菇孢子"},
        {"fume", "胆大菇", "res\\images\\Plants\\FumeShroom\\0.gif", 75, "短程喷射：同排范围伤害"},
        {"chomper", "大嘴花", "res\\images\\Plants\\Chomper\\0.gif", 150, "近战吞噬：一口秒杀前方目标"},
        {"wsdr", "维什戴尔", "res\\images\\Plants\\GatlingPea\\0.gif", 800, "高速四连发：高费高输出"},
        {"threepeater", "三线射手", "res\\images\\Plants\\Threepeater\\0.gif", 325, "三向射击：同时覆盖上中下三行"},
        {"tallnut", "高坚果", "res\\images\\Plants\\TallNut\\0.gif", 125, "重型防线：更高生命值的墙体"}
    };
    return defs;
}

inline int maxLoadoutCards() { return 10; }
inline int initialSun() { return 5000; }
inline const char* level1WaveCsvPath() {
    return "res\\config\\level1_waves.csv";
}

inline const std::vector<ZombieSpawnDef>& initialZombieSpawns() {
    static const std::vector<ZombieSpawnDef> spawns = {
        {"normal", 2, 980.f, 270}
    };
    return spawns;
}

inline const std::vector<ZombieAnimDef>& zombieAnimDefs() {
    static const std::vector<ZombieAnimDef> defs = {
        {"normal", "", 0, 0.09f, "res\\images\\Zombies\\Zombie\\Zombie.gif", "res\\images\\Zombies\\Zombie\\ZombieAttack.gif", "", 0, "res\\images\\Zombies\\Zombie\\ZombieDie.gif"},
        {"cone", "", 0, 0.09f, "res\\images\\Zombies\\ConeheadZombie\\ConeheadZombie.gif", "res\\images\\Zombies\\ConeheadZombie\\ConeheadZombieAttack.gif", "", 0, "res\\images\\Zombies\\Zombie\\ZombieDie.gif"},
        {"bucket", "", 0, 0.09f, "res\\images\\Zombies\\BucketheadZombie\\BucketheadZombie.gif", "res\\images\\Zombies\\BucketheadZombie\\BucketheadZombieAttack.gif", "", 0, "res\\images\\Zombies\\Zombie\\ZombieDie.gif"},
        {"football", "", 0, 0.09f, "res\\images\\Zombies\\FootballZombie\\FootballZombie.gif", "res\\images\\Zombies\\FootballZombie\\FootballZombieAttack.gif", "", 0, "res\\images\\Zombies\\FootballZombie\\Die.gif"},
        {"screen", "", 0, 0.09f, "res\\images\\Zombies\\ScreenDoorZombie\\ScreenDoorZombie.gif", "res\\images\\Zombies\\ScreenDoorZombie\\ScreenDoorZombieAttack.gif", "", 0, "res\\images\\Zombies\\Zombie\\ZombieDie.gif"}
    };
    return defs;
}

inline const std::vector<PlantAnimDef>& plantAnimDefs() {
    static const std::vector<PlantAnimDef> defs = {
        {"peashooter", "", 0, 0.11f, "res\\images\\Plants\\Peashooter\\Peashooter.gif"},
        {"sunflower", "", 0, 0.11f, "res\\images\\Plants\\SunFlower\\SunFlower.gif"},
        {"wallnut", "", 0, 0.11f, "res\\images\\Plants\\WallNut\\WallNut.gif"},
        {"potatomine", "", 0, 0.11f, "res\\images\\Plants\\PotatoMine\\PotatoMine.gif"},
        {"snowpea", "", 0, 0.11f, "res\\images\\Plants\\SnowPea\\SnowPea.gif"},
        {"cherrybomb", "", 0, 0.11f, "res\\images\\Plants\\CherryBomb\\CherryBomb.gif"},
        {"repeater", "", 0, 0.11f, "res\\images\\Plants\\Repeater\\Repeater.gif"},
        {"scaredyshroom", "", 0, 0.11f, "res\\images\\Plants\\ScaredyShroom\\ScaredyShroom.gif"},
        {"fume", "", 0, 0.11f, "res\\images\\Plants\\FumeShroom\\FumeShroom.gif"},
        {"chomper", "", 0, 0.11f, "res\\images\\Plants\\Chomper\\ChomperAttack.gif"},
        {"wsdr", "", 0, 0.10f, "res\\images\\Plants\\GatlingPea\\GatlingPea.gif"},
        {"threepeater", "", 0, 0.11f, "res\\images\\Plants\\Threepeater\\Threepeater.gif"},
        {"tallnut", "", 0, 0.11f, "res\\images\\Plants\\TallNut\\TallNut.gif"}
    };
    return defs;
}

inline const ZombieAnimDef* findZombieAnimDef(const std::string& id) {
    const auto& defs = zombieAnimDefs();
    for (const auto& d : defs) if (id == d.id) return &d;
    return nullptr;
}

inline const PlantAnimDef* findPlantAnimDef(const std::string& id) {
    const auto& defs = plantAnimDefs();
    for (const auto& d : defs) if (id == d.id) return &d;
    return nullptr;
}

} // namespace matou::scene::levelcfg

#endif