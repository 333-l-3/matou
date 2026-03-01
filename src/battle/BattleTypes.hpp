#ifndef BATTLE_TYPES_HPP
#define BATTLE_TYPES_HPP

#include <string>

namespace matou::battle {

struct PlantStats {
    std::string id;
    std::string name;
    int sunCost = 0;
    int hp = 0;
    int attack = 0;
    float attackInterval = 0.f;
};

struct ZombieStats {
    std::string id;
    std::string name;
    int hp = 0;
    float speed = 0.f;
    int attack = 0;
    float attackInterval = 0.f;
};

struct ZombieAnimationSet {
    std::string id;
    std::string walkGif;
    std::string attackGif;
    std::string dieGif;
};

} // namespace matou::battle

#endif
