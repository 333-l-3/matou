#include "Peashooter.hpp"

namespace matou::entry {

Peashooter::Peashooter() {
    hp = 300;
}

void Peashooter::setShootInterval(float seconds) {
    shootInterval = seconds;
}

float Peashooter::getShootInterval() const {
    return shootInterval;
}

void Peashooter::update(float dt) {
    shootTimer += dt;
    if (shootTimer >= shootInterval) {
        shootTimer = 0.f;
    }
}

void Peashooter::render(sf::RenderTarget& target) const {
    sf::CircleShape body;
    body.setRadius(radius);
    body.setOrigin(radius, radius);
    body.setPosition(position);
    body.setFillColor(sf::Color(70, 190, 90));
    body.setOutlineColor(sf::Color(30, 120, 50));
    body.setOutlineThickness(3.f);
    target.draw(body);

    sf::CircleShape muzzle;
    float mr = radius * 0.35f;
    muzzle.setRadius(mr);
    muzzle.setOrigin(mr, mr);
    muzzle.setPosition(position + sf::Vector2f(radius * 0.8f, -radius * 0.2f));
    muzzle.setFillColor(sf::Color(60, 160, 80));
    target.draw(muzzle);
}

}
