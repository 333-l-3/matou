#ifndef LEVEL_CONTENT_REGISTRY_HPP
#define LEVEL_CONTENT_REGISTRY_HPP

#include <string>
#include <vector>

namespace matou::scene::levelcfg {

struct PlantCardDef {
    const char* id;
    const char* name;
    const char* imageRelPath;
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
        {"peashooter", "Õ„∂π…‰ ÷", "res\\images\\Plants\\Peashooter\\0.gif"},
        {"sunflower", "œÚ»’ø˚", "res\\images\\Plants\\SunFlower\\0.gif"},
        {"wallnut", "º·π˚«Ω", "res\\images\\Plants\\WallNut\\0.gif"},
        {"potatomine", "Õ¡∂π¿◊", "res\\images\\Plants\\PotatoMine\\0.gif"},
        {"snowpea", "∫Æ±˘…‰ ÷", "res\\images\\Plants\\SnowPea\\0.gif"},
        {"cherrybomb", "”£Ã“’®µØ", "res\\images\\Plants\\CherryBomb\\0.gif"},
        {"repeater", "À´∑¢…‰ ÷", "res\\images\\Plants\\Repeater\\0.gif"},
        {"scaredyshroom", "µ®–°πΩ", "res\\images\\Plants\\ScaredyShroom\\0.gif"},
        {"fume", "µ®¥ÛπΩ", "res\\images\\Plants\\FumeShroom\\0.gif"},
        {"chomper", "¥Û◊Ïª®", "res\\images\\Plants\\Chomper\\0.gif"},
        {"wsdr", "Œ¨ ≤¥˜∂˚", "res\\images\\Plants\\GatlingPea\\0.gif"}
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
        {"wsdr", "", 0, 0.10f, "res\\images\\Plants\\GatlingPea\\GatlingPea.gif"}
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



