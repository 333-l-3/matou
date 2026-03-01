#include "LevelSelectScene.hpp"

#include "Scenes.hpp"
#include "LoadoutScene.hpp"
#include "SceneManager.hpp"
#include "EncodingTool.hpp"

#include <SFML/Window.hpp>
#include <iostream>
#include <memory>

LevelSelectScene::LevelSelectScene(SceneManager* mgr) : manager(mgr) {}

LevelSelectScene::~LevelSelectScene() {}

void LevelSelectScene::onEnter() {
    std::cout << "LevelSelectScene: onEnter" << std::endl;
    // 使用共享字体
    sf::Font* f = manager ? manager->getSharedFont() : nullptr;
    if (f) title.setFont(*f);
    title.setString(GBKToSFString("选择关卡"));
    title.setCharacterSize(20);
    title.setFillColor(sf::Color::White);
    // 准备一个可点击的区域表示第一关（根据窗口大小在 render 中布局）
    level1Rect = sf::FloatRect(100.f, 220.f, 240.f, 120.f);
}

void LevelSelectScene::onExit() {
    std::cout << "LevelSelectScene: onExit" << std::endl;
}

void LevelSelectScene::handleEvent(const sf::Event& event) {
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f p(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));
        if (level1Rect.contains(p)) {
            std::cout << "LevelSelectScene: Level1 clicked" << std::endl;
            manager->requestChangeScene(std::make_unique<LoadoutScene>(manager, 1));
            return;
        }
    } else if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Escape) {
            manager->requestChangeScene(std::make_unique<MenuScene>(manager));
            return;
        }
    }
}

void LevelSelectScene::update(float dt) {
    (void)dt;
}

void LevelSelectScene::render(sf::RenderTarget& target) {
    // 简单布局：标题 + 一个代表第一关的矩形（可加缩放/图片）
    target.draw(title);
    sf::RectangleShape r;
    r.setPosition(level1Rect.left, level1Rect.top);
    r.setSize({level1Rect.width, level1Rect.height});
    r.setFillColor(sf::Color(80, 80, 140));
    r.setOutlineColor(sf::Color::White);
    r.setOutlineThickness(2.f);
    target.draw(r);
    // 关卡文字
    sf::Text t;
    if (manager && manager->getSharedFont()) t.setFont(*manager->getSharedFont());
    t.setString(GBKToSFString("关卡选择"));
    t.setCharacterSize(28);
    t.setFillColor(sf::Color::Yellow);
    t.setPosition(level1Rect.left + 24.f, level1Rect.top + 36.f);
    target.draw(t);
    // 注释：小字放在右下角
    sf::Text note;
    if (manager && manager->getSharedFont()) note.setFont(*manager->getSharedFont());
    note.setString(GBKToSFString("点击进入第一关  Esc 返回"));
    note.setCharacterSize(14);
    note.setFillColor(sf::Color(200, 200, 200));
    auto b = note.getLocalBounds();
    float padding = 12.f;
    float vx = static_cast<float>(target.getSize().x);
    float vy = static_cast<float>(target.getSize().y);
    note.setPosition(vx - b.width - padding - b.left, vy - b.height - padding - b.top);
    target.draw(note);
}

bool LevelSelectScene::usesWindowBackground() const {
    return false;
}
