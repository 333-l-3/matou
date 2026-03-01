#ifndef PLANT_HPP
#define PLANT_HPP

#include <SFML/Graphics.hpp>

namespace matou::entry {

class Plant {
public:
    virtual ~Plant() = default;

    virtual void update(float dt) = 0;
    virtual void render(sf::RenderTarget& target) const = 0;

    void setPosition(sf::Vector2f p) { position = p; }
    sf::Vector2f getPosition() const { return position; }

    void setHp(int v) { hp = v; }
    int getHp() const { return hp; }

protected:
    sf::Vector2f position{0.f, 0.f};
    int hp = 100;
};

}

#endif
