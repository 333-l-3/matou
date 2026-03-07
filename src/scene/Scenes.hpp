#ifndef SCENES_HPP
#define SCENES_HPP

#include "Scene.hpp"
#include "SceneManager.hpp"
#include <SFML/Graphics.hpp>
#include <memory>
#include "EncodingTool.hpp"
#include "file.hpp"
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

#include "LevelSelectScene.hpp"
#include "Level1Scene.hpp"
#include "LoadoutScene.hpp"
#include "PlantListScene.hpp"

class MenuScene : public Scene {
public:
    MenuScene(SceneManager* mgr) : manager(mgr) {}
    virtual ~MenuScene() {}

    void onEnter() override {
        sf::Font* f = manager ? manager->getSharedFont() : nullptr;
        if (f) titleText.setFont(*f);
        else {
            static sf::Font fallback;
            static bool loaded = false;
            if (!loaded) loaded = fallback.loadFromFile("C:/Windows/Fonts/msyh.ttc");
            if (loaded) titleText.setFont(fallback);
        }

        if (f) hintText.setFont(*f);
        else {
            static sf::Font fallback2;
            static bool loaded2 = false;
            if (!loaded2) loaded2 = fallback2.loadFromFile("C:/Windows/Fonts/msyh.ttc");
            if (loaded2) hintText.setFont(fallback2);
        }

        titleText.setString(GBKToSFString("主菜单"));
        titleText.setCharacterSize(44);
        titleText.setFillColor(sf::Color(248, 244, 230));

        hintText.setString(GBKToSFString("↑↓ 选择  Enter 确认"));
        hintText.setCharacterSize(17);
        hintText.setFillColor(sf::Color(232, 236, 220));

        ResourceManager* rm = manager ? manager->getResourceManager() : nullptr;
        if (rm) {
            bgTexture = rm->getTexture(matou::tool::file::resourcePath("res\\images\\interface\\Surface.jpg"));
            if (!bgTexture) bgTexture = rm->getTexture(matou::tool::file::resourcePath("res\\one.jpg"));
            if (!bgTexture) bgTexture = rm->getTexture(matou::tool::file::resourcePath("res\\images\\interface\\background1.jpg"));
        }

        menuItems = {"开始游戏", "植物列表", "游戏模式 A", "退出"};
        menuTexts.clear();
        selectedIndex = 0;
        for (size_t i = 0; i < menuItems.size(); ++i) {
            sf::Text t;
            if (f) t.setFont(*f);
            else {
                static sf::Font fallback3;
                static bool loaded3 = false;
                if (!loaded3) loaded3 = fallback3.loadFromFile("C:/Windows/Fonts/msyh.ttc");
                if (loaded3) t.setFont(fallback3);
            }
            t.setString(GBKToSFString(menuItems[i]));
            t.setCharacterSize(32);
            t.setFillColor(i == static_cast<size_t>(selectedIndex) ? sf::Color(255, 245, 170) : sf::Color(235, 235, 235));
            menuTexts.push_back(t);
        }

        std::cout << "MenuScene: onEnter" << std::endl;
    }

    void onExit() override { std::cout << "MenuScene: onExit" << std::endl; }
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override { (void)dt; }

    void render(sf::RenderTarget& target) override {
        const float vw = static_cast<float>(target.getSize().x);
        const float vh = static_cast<float>(target.getSize().y);

        float bgScale = 1.f;
        float bgOffsetX = 0.f;
        float bgOffsetY = 0.f;
        bool hasBgImage = false;

        if (bgTexture && bgTexture->getSize().x > 0 && bgTexture->getSize().y > 0) {
            sf::Sprite bg;
            bg.setTexture(*bgTexture);
            const auto tsz = bgTexture->getSize();
            const float sx = vw / static_cast<float>(tsz.x);
            const float sy = vh / static_cast<float>(tsz.y);
            bgScale = std::max(sx, sy); // cover: keep aspect, crop overflow
            const float w = static_cast<float>(tsz.x) * bgScale;
            const float h = static_cast<float>(tsz.y) * bgScale;
            bgOffsetX = (vw - w) * 0.5f;
            bgOffsetY = (vh - h) * 0.5f;
            bg.setScale(bgScale, bgScale);
            bg.setPosition(bgOffsetX, bgOffsetY);
            target.draw(bg);
            hasBgImage = true;
        } else {
            sf::RectangleShape bg({vw, vh});
            bg.setFillColor(sf::Color(28, 38, 28));
            target.draw(bg);
        }

        // 保留左上角信息，不遮挡背景主体信息
        titleText.setPosition(vw * 0.04f, vh * 0.06f);
        hintText.setPosition(vw * 0.04f, vh * 0.14f);
        target.draw(titleText);
        target.draw(hintText);

        // 选择区按原图 900x540 坐标映射: (500,75) - (750,375)
        float panelX = vw * 0.66f;
        float panelY = vh * 0.27f;
        float panelW = vw * 0.30f;
        float panelH = vh * 0.50f;
        if (hasBgImage) {
            panelX = bgOffsetX + 500.f * bgScale;
            panelY = bgOffsetY + 75.f * bgScale;
            panelW = 250.f * bgScale;
            panelH = 300.f * bgScale;
        }

        sf::RectangleShape panel({panelW, panelH});
        panel.setPosition(panelX, panelY);
        panel.setFillColor(sf::Color(25, 32, 22, 88));
        panel.setOutlineThickness(1.5f);
        panel.setOutlineColor(sf::Color(220, 228, 190, 150));
        target.draw(panel);

        const float itemH = panelH / static_cast<float>(std::max<size_t>(1, menuTexts.size()));
        for (size_t i = 0; i < menuTexts.size(); ++i) {
            const float iy = panelY + i * itemH;

            sf::RectangleShape row({panelW, itemH});
            row.setPosition(panelX, iy);
            if (static_cast<int>(i) == selectedIndex) {
                row.setFillColor(sf::Color(195, 208, 150, 95));
                row.setOutlineThickness(1.f);
                row.setOutlineColor(sf::Color(236, 240, 208, 175));
            } else {
                row.setFillColor(sf::Color(0, 0, 0, 0));
                row.setOutlineThickness(0.f);
            }
            target.draw(row);

            auto& t = menuTexts[i];
            t.setFillColor(static_cast<int>(i) == selectedIndex ? sf::Color(38, 45, 16) : sf::Color(242, 242, 232));
            auto b = t.getLocalBounds();
            t.setPosition(panelX + panelW * 0.5f - b.width * 0.5f - b.left,
                          iy + itemH * 0.5f - b.height * 0.5f - b.top);
            target.draw(t);
        }
    }

    bool usesWindowBackground() const override { return false; }

private:
    SceneManager* manager;
    sf::Text titleText;
    sf::Text hintText;
    std::shared_ptr<sf::Texture> bgTexture;
    std::vector<std::string> menuItems;
    std::vector<sf::Text> menuTexts;
    int selectedIndex = 0;
};

class IntroScene : public Scene {
public:
    IntroScene(SceneManager* mgr) : manager(mgr) {}
    virtual ~IntroScene() {}

    void onEnter() override {
        ResourceManager* rm = manager ? manager->getResourceManager() : nullptr;
        if (rm) {
            introBgTexture = rm->getTexture(matou::tool::file::resourcePath("res\\one.jpg"));
            if (!introBgTexture) introBgTexture = rm->getTexture(matou::tool::file::resourcePath("res\\images\\interface\\background1.jpg"));
            loadBarTexture = rm->getTexture(matou::tool::file::resourcePath("res\\images\\interface\\LoadBar.png"));
        }

        blinkTimer = 0.f;
        std::cout << "IntroScene: onEnter" << std::endl;
    }

    void onExit() override { std::cout << "IntroScene: onExit" << std::endl; }

    void handleEvent(const sf::Event& event) override {
        if (event.type == sf::Event::KeyPressed || event.type == sf::Event::MouseButtonPressed) {
            std::cout << "IntroScene: key pressed, requesting MenuScene" << std::endl;
            manager->requestChangeScene(std::make_unique<MenuScene>(manager));
            return;
        }
    }

    void update(float dt) override { blinkTimer += dt; }

    void render(sf::RenderTarget& target) override {
        const float vw = static_cast<float>(target.getSize().x);
        const float vh = static_cast<float>(target.getSize().y);

        if (introBgTexture && introBgTexture->getSize().x > 0 && introBgTexture->getSize().y > 0) {
            sf::Sprite bg;
            bg.setTexture(*introBgTexture);
            const auto tsz = introBgTexture->getSize();
            const float sx = vw / static_cast<float>(tsz.x);
            const float sy = vh / static_cast<float>(tsz.y);
            const float s = std::max(sx, sy); // cover: keep aspect, crop overflow
            const float w = static_cast<float>(tsz.x) * s;
            const float h = static_cast<float>(tsz.y) * s;
            bg.setScale(s, s);
            bg.setPosition((vw - w) * 0.5f, (vh - h) * 0.5f);
            target.draw(bg);
        } else {
            sf::RectangleShape bg({vw, vh});
            bg.setFillColor(sf::Color(28, 32, 28));
            target.draw(bg);
        }

        // 删除提示词区域，改为 LoadBar 提示图
        if (loadBarTexture && loadBarTexture->getSize().x > 0 && loadBarTexture->getSize().y > 0) {
            sf::Sprite bar;
            bar.setTexture(*loadBarTexture);
            const auto tsz = loadBarTexture->getSize();
            const float targetW = vw * 0.36f;
            const float s = targetW / static_cast<float>(tsz.x);
            bar.setScale(s, s);
            const float w = static_cast<float>(tsz.x) * s;
            const float h = static_cast<float>(tsz.y) * s;
            bar.setPosition((vw - w) * 0.5f, vh * 0.84f - h * 0.5f);

            const sf::Uint8 alpha = static_cast<sf::Uint8>(160 + 95 * (0.5f + 0.5f * std::sin(blinkTimer * 3.0f)));
            bar.setColor(sf::Color(255, 255, 255, alpha));
            target.draw(bar);
        }
    }

    bool usesWindowBackground() const override { return false; }

private:
    SceneManager* manager;
    std::shared_ptr<sf::Texture> introBgTexture;
    std::shared_ptr<sf::Texture> loadBarTexture;
    float blinkTimer = 0.f;
};

class PlayScene : public Scene {
public:
    PlayScene(SceneManager* mgr) : manager(mgr) {}
    virtual ~PlayScene() {}
    void onEnter() override {
        sf::Font* f = manager ? manager->getSharedFont() : nullptr;
        if (f) info.setFont(*f);
        else {
            static sf::Font fallback;
            static bool loaded = false;
            if (!loaded) loaded = fallback.loadFromFile("C:/Windows/Fonts/msyh.ttc");
            if (loaded) info.setFont(fallback);
        }
        info.setString(GBKToSFString("游戏场景：按 Esc 返回菜单"));
        info.setCharacterSize(30);
        info.setFillColor(sf::Color::Yellow);
        info.setPosition(80.f, 120.f);
        std::cout << "PlayScene: onEnter" << std::endl;
    }
    void onExit() override { std::cout << "PlayScene: onExit" << std::endl; }
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override { (void)dt; }
    void render(sf::RenderTarget& target) override { target.draw(info); }
    bool usesWindowBackground() const override { return false; }
private:
    SceneManager* manager;
    sf::Text info;
};

inline void MenuScene::handleEvent(const sf::Event& event) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Enter) {
            if (selectedIndex == 0) {
                manager->requestChangeScene(std::make_unique<LevelSelectScene>(manager));
                return;
            } else if (selectedIndex == 1) {
                manager->requestChangeScene(std::make_unique<PlantListScene>(manager));
                return;
            } else if (selectedIndex == 3) {
                return;
            } else {
                return;
            }
        } else if (event.key.code == sf::Keyboard::Up) {
            if (!menuTexts.empty()) selectedIndex = std::max(0, selectedIndex - 1);
        } else if (event.key.code == sf::Keyboard::Down) {
            if (!menuTexts.empty()) selectedIndex = std::min<int>(static_cast<int>(menuTexts.size() - 1), selectedIndex + 1);
        }

        for (size_t i = 0; i < menuTexts.size(); ++i) {
            menuTexts[i].setFillColor(static_cast<int>(i) == selectedIndex ? sf::Color(255, 245, 170)
                                                                           : sf::Color(235, 235, 235));
        }
    }
}

inline void PlayScene::handleEvent(const sf::Event& event) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Escape) {
            manager->requestChangeScene(std::make_unique<MenuScene>(manager));
            return;
        }
    }
}

#endif