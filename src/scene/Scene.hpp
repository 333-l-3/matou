#ifndef SCENE_HPP
#define SCENE_HPP

#include <SFML/Graphics.hpp>

class SceneManager;

class Scene {
public:
    virtual ~Scene() {}
    virtual void onEnter() {}
    virtual void onExit() {}
    virtual void handleEvent(const sf::Event& event) = 0;
    virtual void update(float dt) = 0;
    virtual void render(sf::RenderTarget& target) = 0;
    // 是否使用 GameWindow 的默认背景/主文本。若返回 false，场景被视为独立页面，
    // GameWindow 将不会绘制默认背景或主窗口文本，使场景成为独立功能页面。
    virtual bool usesWindowBackground() const { return true; }
};

#endif
