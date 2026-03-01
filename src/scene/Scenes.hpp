#ifndef SCENES_HPP
#define SCENES_HPP

#include "Scene.hpp"
#include "SceneManager.hpp"
#include <SFML/Graphics.hpp>
#include <memory>
#include "EncodingTool.hpp"
#include <iostream>
#include <vector>
#include <algorithm>

#include "LevelSelectScene.hpp"
#include "Level1Scene.hpp"
#include "LoadoutScene.hpp"
#include "PlantListScene.hpp"

class MenuScene : public Scene {
public:
    MenuScene(SceneManager* mgr) : manager(mgr) {}
    virtual ~MenuScene() {}
    void onEnter() override {
        // 使用共享字体（由 GameWindow 提供）
        sf::Font* f = manager ? manager->getSharedFont() : nullptr;
        if (f) titleText.setFont(*f);
        else {
            static sf::Font fallback;
            static bool loaded = false;
            if (!loaded) loaded = fallback.loadFromFile("C:/Windows/Fonts/msyh.ttc");
            if (loaded) titleText.setFont(fallback);
        }
        titleText.setString(GBKToSFString("主菜单"));

        titleText.setCharacterSize(48);
        titleText.setFillColor(sf::Color::White);
        titleText.setPosition(100.f, 80.f);
        std::cout << "MenuScene: onEnter" << std::endl;

        if (f) hintText.setFont(*f);
        else {
            static sf::Font fallback2;
            static bool loaded2 = false;
            if (!loaded2) loaded2 = fallback2.loadFromFile("C:/Windows/Fonts/msyh.ttc");
            if (loaded2) hintText.setFont(fallback2);
        }
        hintText.setString(GBKToSFString("按 Enter 开始，按 Esc 退出"));
        
        
        
        
        // 第一关场景：使用长图背景 res\\Background.jpg，可左右滑动（鼠标拖拽或箭头键）
        hintText.setPosition(100.f, 180.f);
            // 菜单项
            menuItems = {"开始游戏", "植物列表", "游戏模式 A", "退出"};
            // 清理旧的文字对象并重建
            menuTexts.clear();
            selectedIndex = 0;
            // 准备文字对象数组
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
                t.setFillColor(i == static_cast<size_t>(selectedIndex) ? sf::Color::Yellow : sf::Color::White);
                t.setPosition(120.f, 260.f + static_cast<float>(i) * 48.f);
                menuTexts.push_back(t);
            }
    }
    void onExit() override { std::cout << "MenuScene: onExit" << std::endl; }
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override { (void)dt; }
    void render(sf::RenderTarget& target) override {
        target.draw(titleText);
        target.draw(hintText);
        for (auto& t : menuTexts) target.draw(t);
    }
    // 独立页面：不使用 GameWindow 的默认背景/文本
    bool usesWindowBackground() const override { return false; }
private:
    SceneManager* manager;
    sf::Text titleText;
    sf::Text hintText;
    std::vector<std::string> menuItems;
    std::vector<sf::Text> menuTexts;
    int selectedIndex = 0;
};




// 初始页面：使用 GameWindow 的默认背景/主文本（任意键继续）
class IntroScene : public Scene {
public:
    IntroScene(SceneManager* mgr) : manager(mgr) {}
    virtual ~IntroScene() {}
    void onEnter() override {
        // 可在此启动加载任务
        std::cout << "IntroScene: onEnter" << std::endl;
    }
    void onExit() override { std::cout << "IntroScene: onExit" << std::endl; }
    void handleEvent(const sf::Event& event) override {
        if (event.type == sf::Event::KeyPressed) {
            std::cout << "IntroScene: key pressed, requesting MenuScene" << std::endl;
            manager->requestChangeScene(std::make_unique<MenuScene>(manager));
            return;
        }
    }
    void update(float dt) override { (void)dt; }
    void render(sf::RenderTarget& target) override { (void)target; }
    // 使用窗口默认背景与主文本
    bool usesWindowBackground() const override { return true; }
private:
    SceneManager* manager;
};

class PlayScene : public Scene {
public:
    PlayScene(SceneManager* mgr) : manager(mgr) {}
    virtual ~PlayScene() {}
    void onEnter() override {
        // 使用共享字体（由 GameWindow 提供）
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

// Implementations that reference both classes (keep after both are declared)
inline void MenuScene::handleEvent(const sf::Event& event) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Enter) {
                if (selectedIndex == 0) {
                manager->requestChangeScene(std::make_unique<LevelSelectScene>(manager));
                return; // 切换场景会删除当前对象，避免继续访问成员
            } else if (selectedIndex == 1) {
                manager->requestChangeScene(std::make_unique<PlantListScene>(manager));
                return;
            } else if (selectedIndex == 3) {
                // 退出：不直接关闭窗口，这里可扩展为发送退出信号
                return;
            } else {
                // 其它模式切换（暂存，在 PlayScene 可读取）
                return;
            }
        } else if (event.key.code == sf::Keyboard::Up) {
            if (!menuTexts.empty()) selectedIndex = std::max(0, selectedIndex - 1);
        } else if (event.key.code == sf::Keyboard::Down) {
            if (!menuTexts.empty()) selectedIndex = std::min<int>(static_cast<int>(menuTexts.size() - 1), selectedIndex + 1);
        } else if (event.key.code == sf::Keyboard::Escape) {
            // no-op
        }
        // 更新文字颜色和位置
        for (size_t i = 0; i < menuTexts.size(); ++i) {
            if (i < menuTexts.size())
                menuTexts[i].setFillColor(static_cast<int>(i) == selectedIndex ? sf::Color::Yellow : sf::Color::White);
        }
    }
}

inline void PlayScene::handleEvent(const sf::Event& event) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Escape) {
            manager->requestChangeScene(std::make_unique<MenuScene>(manager));
            return; // 同上，避免在已删除对象上继续执行
        }
    }
}

#endif
