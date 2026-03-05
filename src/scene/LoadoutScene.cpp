#include "LoadoutScene.hpp"

#include "SceneManager.hpp"
#include "LevelSelectScene.hpp"
#include "Level1Scene.hpp"
#include "EncodingTool.hpp"
#include "file.hpp"
#include "ResourceManager.hpp"
#include "LevelContentRegistry.hpp"

#include <SFML/Window.hpp>
#include <algorithm>
#include <iostream>
#include <random>

LoadoutScene::LoadoutScene(SceneManager* mgr, int levelId) : manager(mgr), levelId(levelId) {}

LoadoutScene::~LoadoutScene() {}

void LoadoutScene::onEnter() {
    std::cout << "LoadoutScene: onEnter" << std::endl;

    allPlants.clear();
    const auto& defs = matou::scene::levelcfg::allPlantCards();
    allPlants.reserve(defs.size());
    for (const auto& def : defs) {
        allPlants.push_back({def.name, def.imageRelPath, nullptr});
    }

    ResourceManager* rm = manager ? manager->getResourceManager() : nullptr;
    if (rm) {
        for (auto& p : allPlants) {
            p.texture = rm->getTexture(matou::tool::file::resourcePath(p.imageRelPath));
        }
    }

    selectedPlants = manager ? manager->getLoadoutPlants() : std::vector<int>{};
    if (selectedPlants.empty()) {
        for (int i = 0; i < static_cast<int>(allPlants.size()) && i < 10; ++i) selectedPlants.push_back(i);
    } else if (selectedPlants.size() > 10) {
        selectedPlants.resize(10);
    }

    scrollX = 0.f;
    maxScrollX = 0.f;
    panelDragging = false;

    if (manager && manager->getSharedFont()) {
        titleText.setFont(*manager->getSharedFont());
        tipText.setFont(*manager->getSharedFont());
    }
    titleText.setString(GBKToSFString("备战选卡"));
    titleText.setCharacterSize(26);
    titleText.setFillColor(sf::Color(30, 30, 35));

    tipText.setCharacterSize(15);
    tipText.setFillColor(sf::Color(70, 70, 80));

    buildLayout({1000u, 600u});
}

void LoadoutScene::onExit() {
    std::cout << "LoadoutScene: onExit" << std::endl;
}

void LoadoutScene::update(float dt) {
    (void)dt;
}

bool LoadoutScene::usesWindowBackground() const {
    return false;
}

bool LoadoutScene::isSelected(int plantId) const {
    return std::find(selectedPlants.begin(), selectedPlants.end(), plantId) != selectedPlants.end();
}

void LoadoutScene::buildLayout(sf::Vector2u viewSize) {
    float w = static_cast<float>(viewSize.x);
    float h = static_cast<float>(viewSize.y);

    float padding = std::max(12.f, h * 0.02f);

    backRect = sf::FloatRect(padding, padding, w * 0.12f, h * 0.08f);
    startRect = sf::FloatRect(w - padding - w * 0.18f, h - padding - h * 0.10f, w * 0.18f, h * 0.10f);
    clearRect = sf::FloatRect(w * 0.62f, padding, w * 0.10f, h * 0.08f);
    randomRect = sf::FloatRect(w * 0.74f, padding, w * 0.12f, h * 0.08f);

    titleText.setPosition(padding, backRect.top + backRect.height + padding);

    float panelTop = titleText.getPosition().y + 74.f;
    float panelBottom = startRect.top - padding;
    panelRect = sf::FloatRect(padding, panelTop, w - padding * 2.f, std::max(0.f, panelBottom - panelTop));

    int rows = 2;
    float inset = std::max(8.f, h * 0.015f);
    float gapX = std::max(10.f, w * 0.015f);
    float gapY = std::max(10.f, h * 0.03f);

    float cardH = (panelRect.height - inset * 2.f - gapY * (rows - 1)) / static_cast<float>(rows);
    float cardW = std::max(140.f, cardH * 1.25f);

    plantRects.clear();
    plantRects.reserve(allPlants.size());
    for (size_t i = 0; i < allPlants.size(); ++i) {
        int row = static_cast<int>(i % static_cast<size_t>(rows));
        int col = static_cast<int>(i / static_cast<size_t>(rows));
        float x = panelRect.left + inset + col * (cardW + gapX);
        float y = panelRect.top + inset + row * (cardH + gapY);
        plantRects.emplace_back(x, y, cardW, cardH);
    }

    int cols = allPlants.empty() ? 0 : (static_cast<int>(allPlants.size()) + rows - 1) / rows;
    float contentW = cols > 0 ? (cols * cardW + (cols - 1) * gapX + inset * 2.f) : 0.f;
    maxScrollX = std::max(0.f, contentW - panelRect.width);
    if (scrollX < 0.f) scrollX = 0.f;
    if (scrollX > maxScrollX) scrollX = maxScrollX;
}

void LoadoutScene::handleEvent(const sf::Event& event) {
    if (event.type == sf::Event::Resized) {
        buildLayout({event.size.width, event.size.height});
        return;
    }

    if (event.type == sf::Event::MouseWheelScrolled) {
        if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
            sf::Vector2f p(static_cast<float>(event.mouseWheelScroll.x), static_cast<float>(event.mouseWheelScroll.y));
            if (panelRect.contains(p)) {
                scrollX -= event.mouseWheelScroll.delta * 60.f;
                if (scrollX < 0.f) scrollX = 0.f;
                if (scrollX > maxScrollX) scrollX = maxScrollX;
                return;
            }
        }
    }

    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Escape) {
            manager->requestChangeScene(std::make_unique<LevelSelectScene>(manager));
            return;
        }
        if (event.key.code == sf::Keyboard::Enter) {
            manager->setLoadoutPlants(selectedPlants);
                manager->requestChangeScene(std::make_unique<Level1Scene>(manager));
            return;
        }
    }

    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f p(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));

        if (backRect.contains(p)) {
            manager->requestChangeScene(std::make_unique<LevelSelectScene>(manager));
            return;
        }

        if (startRect.contains(p)) {
            manager->setLoadoutPlants(selectedPlants);
                manager->requestChangeScene(std::make_unique<Level1Scene>(manager));
            return;
        }

        if (clearRect.contains(p)) {
            selectedPlants.clear();
            return;
        }

        if (randomRect.contains(p)) {
            std::vector<int> ids;
            ids.reserve(allPlants.size());
            for (int i = 0; i < static_cast<int>(allPlants.size()); ++i) ids.push_back(i);
            std::mt19937 rng(static_cast<unsigned int>(std::random_device{}()));
            std::shuffle(ids.begin(), ids.end(), rng);
            selectedPlants.assign(ids.begin(), ids.begin() + std::min<size_t>(10, ids.size()));
            return;
        }

        bool hitCard = false;
        if (panelRect.contains(p)) {
            for (int i = 0; i < static_cast<int>(plantRects.size()); ++i) {
                sf::FloatRect r = plantRects[static_cast<size_t>(i)];
                r.left -= scrollX;
                if (!r.contains(p)) continue;

                hitCard = true;
                auto it = std::find(selectedPlants.begin(), selectedPlants.end(), i);
                if (it != selectedPlants.end()) {
                    selectedPlants.erase(it);
                } else {
                    if (selectedPlants.size() < 10) selectedPlants.push_back(i);
                }
                break;
            }

            if (!hitCard) {
                panelDragging = true;
                panelDragStartMouseX = p.x;
                panelDragStartScrollX = scrollX;
            }
            return;
        }
    }

    if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
        panelDragging = false;
        return;
    }

    if (event.type == sf::Event::MouseMoved) {
        if (panelDragging) {
            float mx = static_cast<float>(event.mouseMove.x);
            float dx = mx - panelDragStartMouseX;
            scrollX = panelDragStartScrollX - dx;
            if (scrollX < 0.f) scrollX = 0.f;
            if (scrollX > maxScrollX) scrollX = maxScrollX;
        }
        return;
    }
}

void LoadoutScene::render(sf::RenderTarget& target) {
    float viewW = static_cast<float>(target.getSize().x);
    float viewH = static_cast<float>(target.getSize().y);

    {
        sf::RectangleShape bg;
        bg.setPosition(0.f, 0.f);
        bg.setSize({viewW, viewH});
        bg.setFillColor(sf::Color(245, 245, 248));
        target.draw(bg);
    }

    auto drawButton = [&](const sf::FloatRect& r, const std::string& label, sf::Color fill, sf::Color outline, sf::Color textColor) {
        sf::RectangleShape s;
        s.setPosition(r.left, r.top);
        s.setSize({r.width, r.height});
        s.setFillColor(fill);
        s.setOutlineColor(outline);
        s.setOutlineThickness(2.f);
        target.draw(s);

        if (manager && manager->getSharedFont()) {
            sf::Text t;
            t.setFont(*manager->getSharedFont());
            t.setString(GBKToSFString(label));
            t.setCharacterSize(static_cast<unsigned int>(std::max(14.f, r.height * 0.38f)));
            t.setFillColor(textColor);
            auto b = t.getLocalBounds();
            t.setPosition(r.left + (r.width - b.width) * 0.5f - b.left, r.top + (r.height - b.height) * 0.5f - b.top);
            target.draw(t);
        }
    };

    drawButton(backRect, "返回", sf::Color(245, 245, 248), sf::Color(160, 160, 170), sf::Color(40, 40, 50));
    drawButton(clearRect, "清空", sf::Color(236, 236, 240), sf::Color(160, 160, 170), sf::Color(50, 50, 60));
    drawButton(randomRect, "随机10", sf::Color(230, 245, 255), sf::Color(140, 180, 220), sf::Color(40, 80, 120));
    drawButton(startRect, "开始", sf::Color(80, 200, 120), sf::Color(60, 160, 100), sf::Color::White);

    target.draw(titleText);

    std::string tip = "已选 " + std::to_string(selectedPlants.size()) + " 张（可直接开始）";
    tipText.setString(GBKToSFString(tip));
    tipText.setPosition(titleText.getPosition().x, titleText.getPosition().y + 30.f);
    target.draw(tipText);

    {
        sf::RectangleShape panel;
        panel.setPosition(panelRect.left, panelRect.top);
        panel.setSize({panelRect.width, panelRect.height});
        panel.setFillColor(sf::Color(250, 250, 252));
        panel.setOutlineColor(sf::Color(170, 170, 180));
        panel.setOutlineThickness(2.f);
        target.draw(panel);
    }

    for (int i = 0; i < static_cast<int>(plantRects.size()); ++i) {
        sf::FloatRect r = plantRects[static_cast<size_t>(i)];
        r.left -= scrollX;

        if (r.left + r.width < panelRect.left + 4.f || r.left > panelRect.left + panelRect.width - 4.f) continue;

        bool selected = isSelected(i);

        sf::RectangleShape card;
        card.setPosition(r.left, r.top);
        card.setSize({r.width, r.height});
        card.setFillColor(sf::Color::White);
        card.setOutlineColor(selected ? sf::Color(70, 170, 90) : sf::Color(170, 170, 180));
        card.setOutlineThickness(selected ? 3.f : 2.f);
        target.draw(card);

        if (i < static_cast<int>(allPlants.size()) && allPlants[static_cast<size_t>(i)].texture) {
            sf::Sprite sp;
            sp.setTexture(*allPlants[static_cast<size_t>(i)].texture);
            auto tsz = allPlants[static_cast<size_t>(i)].texture->getSize();
            if (tsz.x > 0 && tsz.y > 0) {
                float maxW = r.width * 0.45f;
                float maxH = r.height * 0.78f;
                float sx = maxW / static_cast<float>(tsz.x);
                float sy = maxH / static_cast<float>(tsz.y);
                float s = std::min(sx, sy);
                sp.setScale(s, s);
                float h = static_cast<float>(tsz.y) * s;
                sp.setPosition(r.left + 8.f, r.top + (r.height - h) * 0.5f);
                target.draw(sp);
            }
        }

        if (manager && manager->getSharedFont() && i < static_cast<int>(allPlants.size())) {
            sf::Text name;
            name.setFont(*manager->getSharedFont());
            name.setString(GBKToSFString(allPlants[static_cast<size_t>(i)].name));
            name.setCharacterSize(static_cast<unsigned int>(std::max(12.f, r.height * 0.15f)));
            name.setFillColor(sf::Color(50, 50, 60));
            auto b = name.getLocalBounds();
            name.setPosition(r.left + r.width * 0.52f - b.left, r.top + (r.height - b.height) * 0.5f - b.top);
            target.draw(name);

            if (selected) {
                sf::Text tag;
                tag.setFont(*manager->getSharedFont());
                tag.setString(GBKToSFString("已带入"));
                tag.setCharacterSize(static_cast<unsigned int>(std::max(11.f, r.height * 0.13f)));
                tag.setFillColor(sf::Color(70, 170, 90));
                auto tb = tag.getLocalBounds();
                tag.setPosition(r.left + r.width - tb.width - 8.f - tb.left, r.top + 6.f - tb.top);
                target.draw(tag);
            }
        }
    }
}






