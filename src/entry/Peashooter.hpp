#ifndef PEASHOOTER_HPP
#define PEASHOOTER_HPP

#include "Plant.hpp"

namespace matou::entry {

class Peashooter final : public Plant {
public:
    Peashooter();

    void update(float dt) override;
    void render(sf::RenderTarget& target) const override;

    void setShootInterval(float seconds);
    float getShootInterval() const;

private:
    float shootInterval = 1.5f;
    float shootTimer = 0.f;

    float radius = 22.f;
};

}

#endif
