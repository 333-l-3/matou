#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include "encodingtool.hpp"
#include "file.hpp"
#include "SceneManager.hpp"
#include "ResourceManager.hpp"

class GameWindow {
private:
    sf::RenderWindow window;
        sf::Font font;
        sf::Sprite backgroundSprite;
        std::shared_ptr<sf::Texture> backgroundTexturePtr;
        ResourceManager resourceManager;
    sf::Text text;
    SceneManager sceneManager;
    // 基本属性（默认值）
    unsigned int width = 1000;
    unsigned int height = 600;
    std::string title = "植物大战僵尸";
    std::string fontPath = "C:/Windows/Fonts/msyh.ttc";
    std::string textContent = "任意键继续";
    unsigned int textSize = 40;
    sf::Color textColor = sf::Color::Green;
    float textX = 100.0f;
    float textY = 150.0f;
    sf::Color bgColor = sf::Color(30, 30, 30);
    unsigned int fps = 60;

public:
    // 构造函数
    GameWindow() {
        init();
    }

    // 设置属性（链式调用）
    GameWindow& setSize(unsigned int w, unsigned int h) {
        width = w; height = h;
        window.setSize({w, h});
        return *this;
    }
    
    GameWindow& setTitle(const std::string& t) {
        title = t;
        window.setTitle(GBKToSFString(t));
        return *this;
    }

    GameWindow& setText(const std::string& t) {
        textContent = t;
        text.setString(GBKToSFString(t));
        return *this;
    }

    GameWindow& setTextColor(const sf::Color& c) {
        textColor = c;
        text.setFillColor(c);
        return *this;
    }

    GameWindow& setTextSize(unsigned int s) {
        textSize = s;
        text.setCharacterSize(s);
        return *this;
    }

    GameWindow& setPosition(float x, float y) {
        textX = x; textY = y;
        text.setPosition(x, y);
        return *this;
    }

    GameWindow& setBackground(const sf::Color& c) {
        bgColor = c;
        return *this;
    }

    GameWindow& setFPS(unsigned int f) {
        fps = f;
        window.setFramerateLimit(f);
        return *this;
    }

    GameWindow& setBackgroundImage(const std::string& imagePath) {
        auto texPtr = resourceManager.getTexture(imagePath);
        if (!texPtr) {
            std::cerr << "背景图片加载失败：" << imagePath << std::endl;
            return *this;
        }
        backgroundTexturePtr = texPtr;
        backgroundSprite.setTexture(*backgroundTexturePtr);
        auto tsz = backgroundTexturePtr->getSize();
        if (tsz.x > 0 && tsz.y > 0) {
            backgroundSprite.setScale(
                static_cast<float>(width) / tsz.x,
                static_cast<float>(height) / tsz.y
            );
        }
        return *this;
    }
    // 访问场景管理器以便在外部创建/切换场景
    SceneManager& scenes() { return sceneManager; }
    // 运行窗口
    void run() {
        sf::Clock clock;
        // 在进入主循环前清空可能存在的遗留事件（例如构建/运行过程中产生的按键）
        {
            sf::Event e;
            while (window.pollEvent(e)) {}
        }

        // 短暂忽略输入以避免立刻响应历史事件（秒）
        const float inputIgnoreDuration = 0.1f;
        float inputIgnoreTimer = inputIgnoreDuration;

        while (window.isOpen()) {
            float dt = clock.restart().asSeconds();
            if (inputIgnoreTimer > 0.f) inputIgnoreTimer -= dt;

            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    window.close();
                } else {
                    if (inputIgnoreTimer <= 0.f) {
                        sceneManager.handleEvent(event);
                    }
                }
            }

            // 在安全点应用挂起的场景切换（避免场景在事件处理中被立即删除造成复用问题）
            sceneManager.applyPending();

            sceneManager.update(dt);

            window.clear(bgColor);
            // 根据当前场景是否需要窗口默认背景来决定绘制内容
            Scene* cur = sceneManager.getCurrent();
            bool drawWindowDefault = (cur == nullptr) || cur->usesWindowBackground();
            if (drawWindowDefault) {
                // 绘制背景图片和主窗口文本
                window.draw(backgroundSprite);
                window.draw(text);
            }
            // 绘制当前场景（场景可以选择覆盖或叠加）
            sceneManager.render(window);
            window.display();
        }
    }

    
private:
    void init() {
        SetConsoleGBK();
        // 获取屏幕分辨率
        sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
        // 计算居中位置
        int x = (desktopMode.width - width) / 2;
        int y = (desktopMode.height - height) / 2;
        window.create({width, height}, GBKToSFString(title), sf::Style::Titlebar | sf::Style::Close);
        window.setPosition({x, y});  // 设置窗口居中
        window.setFramerateLimit(fps);
        // 注册资源管理器到场景管理器
        sceneManager.setResourceManager(&resourceManager);
        // 使用 ResourceManager 加载并设置共享字体
        auto fontPtr = resourceManager.getFont(fontPath);
        if (!fontPtr) {
            std::cerr << "字体加载失败！" << std::endl;
            system("pause");
            exit(1);
        }
        font = *fontPtr; // keep a local copy for window UI
        sceneManager.setSharedFont(&font);
        // 设置默认背景图片（通过 ResourceManager 加载）
        setBackgroundImage(matou::tool::file::resourcePath("res\\one.jpg")); // 设置默认背景图片

        text.setFont(font);
        text.setString(GBKToSFString(textContent));
        text.setCharacterSize(textSize);
        text.setFillColor(textColor);
        text.setPosition(textX, textY);        
    }

};

#endif
