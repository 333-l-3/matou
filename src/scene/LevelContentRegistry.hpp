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
        {"peashooter", "Õ„∂π…‰ ÷", "res\\images\\Peashooter.png"},
        {"sunflower", "œÚ»’ø˚", "res\\images\\SunFlower.png"},
        {"wallnut", "º·π˚«Ω", "res\\images\\WallNut.png"},
        {"potatomine", "Õ¡∂π¿◊", "res\\images\\PotatoMine.png"},
        {"snowpea", "∫Æ±˘…‰ ÷", "res\\images\\SnowPea.png"},
        {"cherrybomb", "”£Ã“’®µØ", "res\\images\\CherryBomb.png"},
        {"repeater", "À´∑¢…‰ ÷", "res\\images\\Repeater.png"},
        {"scaredyshroom", "µ®–°πΩ", "res\\images\\danxiaogu.png"},
        {"fume", "µ®¥ÛπΩ", "res\\images\\dandagu.png"},
        {"chomper", "¥Û◊Ïª®", "res\\images\\dazuihua.png"},
        {"wsdr", "Œ¨ ≤¥˜∂˚", "res\\images\\wsdr.png"}
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
        {"normal", "res\\images\\zombie_frames\\normal_walk_%02d.png", 64, 0.08f, "res\\images\\ZombieWalk1.gif", "res\\images\\ZombieWalk2.gif", "res\\images\\zombie_frames\\normal_die_%02d.png", 64, "res\\images\\ZombieDie.gif"},
        {"cone", "res\\images\\zombie_frames\\cone_walk_%02d.png", 64, 0.08f, "res\\images\\ConeZombieWalk.gif", "res\\images\\ConeZombieAttack.gif", "res\\images\\zombie_frames\\normal_die_%02d.png", 64, "res\\images\\ZombieDie.gif"},
        {"bucket", "res\\images\\zombie_frames\\bucket_walk_%02d.png", 64, 0.08f, "res\\images\\BucketZombieWalk.gif", "res\\images\\BucketZombieAttack.gif", "res\\images\\zombie_frames\\normal_die_%02d.png", 64, "res\\images\\ZombieDie.gif"},
        {"football", "res\\images\\zombie_frames\\football_walk_%02d.png", 64, 0.08f, "res\\images\\FootballZombieWalk.gif", "res\\images\\FootballZombieAttack.gif", "res\\images\\zombie_frames\\football_die_%02d.png", 64, "res\\images\\FootballZombieDie.gif"},
        {"screen", "res\\images\\zombie_frames\\screen_walk_%02d.png", 64, 0.08f, "res\\images\\ScreenZombieWalk.gif", "res\\images\\ScreenZombieAttack.gif", "res\\images\\zombie_frames\\normal_die_%02d.png", 64, "res\\images\\ZombieDie.gif"}
    };
    return defs;
}

inline const std::vector<PlantAnimDef>& plantAnimDefs() {
    static const std::vector<PlantAnimDef> defs = {
        {"peashooter", "res\\images\\plant_frames\\peashooter_idle_%02d.png", 64, 0.09f, "res\\images\\Peashooter.png"},
        {"sunflower", "res\\images\\plant_frames\\sunflower_idle_%02d.png", 64, 0.08f, "res\\images\\SunFlower.png"},
        {"wallnut", "res\\images\\plant_frames\\wallnut_idle_%02d.png", 64, 0.10f, "res\\images\\WallNut.png"},
        {"potatomine", "res\\images\\plant_frames\\potatomine_idle_%02d.png", 64, 0.10f, "res\\images\\PotatoMine.png"},
        {"snowpea", "res\\images\\plant_frames\\snowpea_idle_%02d.png", 64, 0.09f, "res\\images\\SnowPea.png"},
        {"cherrybomb", "res\\images\\plant_frames\\cherrybomb_idle_%02d.png", 64, 0.08f, "res\\images\\CherryBomb.png"},
        {"repeater", "res\\images\\plant_frames\\repeater_idle_%02d.png", 64, 0.09f, "res\\images\\Repeater.png"}
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

