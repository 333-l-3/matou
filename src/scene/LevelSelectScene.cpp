#include "LevelSelectScene.hpp"

#include "Scenes.hpp"
#include "LoadoutScene.hpp"
#include "SceneManager.hpp"
#include "EncodingTool.hpp"

#include <SFML/Window.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>

LevelSelectScene::LevelSelectScene(SceneManager* mgr) : manager(mgr) {}

LevelSelectScene::~LevelSelectScene() {}

void LevelSelectScene::onEnter() {
    std::cout << "LevelSelectScene: onEnter" << std::endl;

    sf::Font* f = manager ? manager->getSharedFont() : nullptr;
    if (f) {
        title.setFont(*f);
        subtitle.setFont(*f);
        note.setFont(*f);
    }

    title.setString(GBKToSFString("关卡选择"));
    title.setCharacterSize(40);
    title.setFillColor(sf::Color(236, 241, 247));

    subtitle.setString(GBKToSFString("选择要开始的关卡"));
    subtitle.setCharacterSize(18);
    subtitle.setFillColor(sf::Color(166, 176, 191));

    note.setString(GBKToSFString("点击关卡进入备战  Esc 返回主菜单"));
    note.setCharacterSize(15);
    note.setFillColor(sf::Color(150, 160, 174));

    panelRect = sf::FloatRect(160.f, 110.f, 680.f, 380.f);
    level1Rect = sf::FloatRect(320.f, 260.f, 360.f, 62.f);
    level1Hovered = false;
    animTime = 0.f;
    hoverBlend = 0.f;
}

void LevelSelectScene::onExit() {
    std::cout << "LevelSelectScene: onExit" << std::endl;
}

void LevelSelectScene::recalcLayout(sf::Vector2u size) {
    const float vw = static_cast<float>(size.x);
    const float vh = static_cast<float>(size.y);

    const float panelW = std::min(700.f, vw * 0.72f);
    const float panelH = std::min(390.f, vh * 0.74f);
    const float panelX = (vw - panelW) * 0.5f;
    const float panelY = (vh - panelH) * 0.5f;
    panelRect = sf::FloatRect(panelX, panelY, panelW, panelH);

    const float btnW = std::max(240.f, panelW * 0.50f);
    const float btnH = 62.f;
    const float btnX = panelX + (panelW - btnW) * 0.5f;
    const float btnY = panelY + panelH * 0.56f - btnH * 0.5f;
    level1Rect = sf::FloatRect(btnX, btnY, btnW, btnH);

    level1Hovered = level1Rect.contains(mousePos);
}

void LevelSelectScene::handleEvent(const sf::Event& event) {
    if (event.type == sf::Event::Resized) {
        recalcLayout({event.size.width, event.size.height});
        return;
    }

    if (event.type == sf::Event::MouseMoved) {
        mousePos = {static_cast<float>(event.mouseMove.x), static_cast<float>(event.mouseMove.y)};
        level1Hovered = level1Rect.contains(mousePos);
        return;
    }

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
    animTime += dt;
    const float target = level1Hovered ? 1.f : 0.f;
    const float blendSpeed = std::min(1.f, dt * 8.f);
    hoverBlend += (target - hoverBlend) * blendSpeed;
}

void LevelSelectScene::render(sf::RenderTarget& target) {
    recalcLayout(target.getSize());

    const float vw = static_cast<float>(target.getSize().x);
    const float vh = static_cast<float>(target.getSize().y);

    const int bands = 14;
    for (int i = 0; i < bands; ++i) {
        float t0 = static_cast<float>(i) / static_cast<float>(bands);
        float t1 = static_cast<float>(i + 1) / static_cast<float>(bands);
        sf::RectangleShape strip({vw, vh * (t1 - t0) + 1.f});
        strip.setPosition(0.f, vh * t0);
        sf::Uint8 r = static_cast<sf::Uint8>(18 + t0 * 18.f);
        sf::Uint8 g = static_cast<sf::Uint8>(23 + t0 * 20.f);
        sf::Uint8 b = static_cast<sf::Uint8>(30 + t0 * 24.f);
        strip.setFillColor(sf::Color(r, g, b));
        target.draw(strip);
    }

    sf::CircleShape glowA(220.f);
    glowA.setFillColor(sf::Color(70, 120, 150, 26));
    glowA.setPosition(panelRect.left - 210.f + std::sin(animTime * 0.45f) * 20.f, panelRect.top - 180.f);
    target.draw(glowA);

    sf::CircleShape glowB(180.f);
    glowB.setFillColor(sf::Color(90, 130, 108, 22));
    glowB.setPosition(panelRect.left + panelRect.width - 24.f + std::sin(animTime * 0.55f + 1.1f) * 14.f, panelRect.top + panelRect.height - 46.f);
    target.draw(glowB);

    sf::RectangleShape panel({panelRect.width, panelRect.height});
    panel.setPosition(panelRect.left, panelRect.top);
    panel.setFillColor(sf::Color(28, 34, 44, 226));
    panel.setOutlineColor(sf::Color(88, 104, 124, 210));
    panel.setOutlineThickness(2.f);
    target.draw(panel);

    sf::RectangleShape panelInner({panelRect.width - 20.f, panelRect.height - 20.f});
    panelInner.setPosition(panelRect.left + 10.f, panelRect.top + 10.f);
    panelInner.setFillColor(sf::Color(34, 41, 54, 175));
    panelInner.setOutlineThickness(0.f);
    target.draw(panelInner);

    auto tb = title.getLocalBounds();
    const float titleFloatY = std::sin(animTime * 1.2f) * 1.6f;
    title.setPosition(panelRect.left + (panelRect.width - tb.width) * 0.5f - tb.left, panelRect.top + 34.f + titleFloatY - tb.top);
    target.draw(title);

    auto sb = subtitle.getLocalBounds();
    subtitle.setPosition(panelRect.left + (panelRect.width - sb.width) * 0.5f - sb.left, panelRect.top + 92.f - sb.top);
    target.draw(subtitle);

    const float pulse = 1.f + (0.012f + 0.016f * hoverBlend) * std::sin(animTime * 2.9f);
    const float btnW = level1Rect.width * pulse;
    const float btnH = level1Rect.height * pulse;
    const float btnX = level1Rect.left + (level1Rect.width - btnW) * 0.5f;
    const float btnY = level1Rect.top + (level1Rect.height - btnH) * 0.5f;

    sf::RectangleShape btn({btnW, btnH});
    btn.setPosition(btnX, btnY);
    btn.setFillColor(level1Hovered ? sf::Color(72, 96, 122, 242) : sf::Color(56, 76, 98, 232));
    btn.setOutlineColor(level1Hovered ? sf::Color(178, 202, 228) : sf::Color(120, 145, 170));
    btn.setOutlineThickness(2.f + hoverBlend);
    target.draw(btn);

    sf::RectangleShape btnInner({btnW - 10.f, btnH - 10.f});
    btnInner.setPosition(btnX + 5.f, btnY + 5.f);
    btnInner.setFillColor(level1Hovered ? sf::Color(84, 112, 142, 126) : sf::Color(76, 100, 126, 108));
    btnInner.setOutlineThickness(0.f);
    target.draw(btnInner);

    sf::Text levelTitle;
    if (manager && manager->getSharedFont()) levelTitle.setFont(*manager->getSharedFont());
    levelTitle.setString(GBKToSFString("关卡 1"));
    levelTitle.setCharacterSize(26);
    levelTitle.setFillColor(sf::Color(244, 248, 255));
    auto lb = levelTitle.getLocalBounds();
    levelTitle.setPosition(btnX + 18.f - lb.left, btnY + (btnH - lb.height) * 0.5f - lb.top - 1.f);
    target.draw(levelTitle);

    sf::Text levelSub;
    if (manager && manager->getSharedFont()) levelSub.setFont(*manager->getSharedFont());
    levelSub.setString(GBKToSFString("白天草地"));
    levelSub.setCharacterSize(16);
    levelSub.setFillColor(sf::Color(204, 220, 236));
    auto lsb = levelSub.getLocalBounds();
    levelSub.setPosition(btnX + btnW - lsb.width - 18.f - lsb.left, btnY + (btnH - lsb.height) * 0.5f - lsb.top);
    target.draw(levelSub);

    auto nb = note.getLocalBounds();
    note.setPosition(panelRect.left + (panelRect.width - nb.width) * 0.5f - nb.left, panelRect.top + panelRect.height - 40.f - nb.top);
    target.draw(note);
}

bool LevelSelectScene::usesWindowBackground() const {
    return false;
}