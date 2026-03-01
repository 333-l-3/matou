#include "PlantListScene.hpp"

#include "SceneManager.hpp"
#include "Scenes.hpp"
#include "EncodingTool.hpp"
#include "ResourceManager.hpp"
#include "file.hpp"

#include <SFML/Window.hpp>
#include <cmath>
#include <iostream>

PlantListScene::PlantListScene(SceneManager* mgr) : manager(mgr) {}

PlantListScene::~PlantListScene() {}

void PlantListScene::onEnter() {
    std::cout << "PlantListScene: onEnter" << std::endl;

    if (manager && manager->getSharedFont()) titleText.setFont(*manager->getSharedFont());
    titleText.setString(GBKToSFString("植物列表"));
    titleText.setCharacterSize(36);
    titleText.setFillColor(sf::Color(30, 30, 35));

    cards.clear();
    cards.push_back({"豌豆射手", 100, "res\\images\\Peashooter.png", "基础射手：持续发射豌豆"});
    cards.push_back({"向日葵", 50, "res\\images\\SunFlower.png", "生产阳光：为战斗提供资源"});
    cards.push_back({"坚果", 50, "res\\images\\WallNut.png", "防御植物：阻挡僵尸前进"});
    cards.push_back({"土豆雷", 25, "res\\images\\PotatoMine.png", "地雷：准备后爆炸，消灭附近敌人"});
    cards.push_back({"寒冰射手", 175, "res\\images\\SnowPea.png", "冰豌豆：减速敌人"});
    cards.push_back({"樱桃炸弹", 150, "res\\images\\CherryBomb.png", "爆炸：瞬间清除一片区域"});
    cards.push_back({"双发射手", 200, "res\\images\\Repeater.png", "双发：更快输出"});
    cards.push_back({"胆小菇", 0, "res\\images\\danxiaogu.png", "0消耗"});
    cards.push_back({"胆大菇", 75, "res\\images\\dandagu.png", "aoe伤害"});
    cards.push_back({"大嘴花", 150, "res\\images\\dazuihua.png", "单体吞噬"});
    cards.push_back({"维什戴尔", 800, "res\\images\\wsdr.png", "我不知道这是什么"});

    ResourceManager* rm = manager ? manager->getResourceManager() : nullptr;
    for (auto& c : cards) {
        if (!rm) break;
        c.texture = rm->getTexture(matou::tool::file::resourcePath(c.imageRelPath));
    }

    detailOpen = false;
    detailIndex = -1;
    scrollX = 0.f;
    maxScrollX = 0.f;
    dragging = false;
    holdLeft = false;
    holdRight = false;

    buildLayout({1000u, 600u});
}

void PlantListScene::onExit() {
    std::cout << "PlantListScene: onExit" << std::endl;
}

bool PlantListScene::usesWindowBackground() const {
    return false;
}

void PlantListScene::update(float dt) {
    if (detailOpen) return;

    float speed = 1200.f;
    if (holdLeft && !holdRight) scrollX -= speed * dt;
    else if (holdRight && !holdLeft) scrollX += speed * dt;

    if (scrollX < 0.f) scrollX = 0.f;
    if (scrollX > maxScrollX) scrollX = maxScrollX;
}

void PlantListScene::buildLayout(sf::Vector2u viewSize) {
    float w = static_cast<float>(viewSize.x);
    float h = static_cast<float>(viewSize.y);

    float padding = std::max(10.f, h * 0.02f);

    backRect = sf::FloatRect(padding, padding, w * 0.12f, h * 0.08f);

    titleText.setPosition(padding, padding + backRect.height + padding * 0.4f);

    // 卡片区域：2 行，多列横向滚动
    float gridTop = titleText.getPosition().y + h * 0.10f;
    float gridBottom = h - padding;
    float gridLeft = padding;
    float gridRight = w - padding;
    gridRect = sf::FloatRect(gridLeft, gridTop, std::max(0.f, gridRight - gridLeft), std::max(0.f, gridBottom - gridTop));

    int rows = 2;
    float gapX = w * 0.02f;
    float gapY = h * 0.04f;

    float cardW = std::max(160.f, gridRect.width * 0.23f);
    float cardH = (gridRect.height - gapY * (rows - 1)) / rows;

    cardRects.clear();
    cardRects.reserve(cards.size());
    for (size_t i = 0; i < cards.size(); ++i) {
        int row = static_cast<int>(i % static_cast<size_t>(rows));
        int col = static_cast<int>(i / static_cast<size_t>(rows));
        float x = gridRect.left + col * (cardW + gapX);
        float y = gridRect.top + row * (cardH + gapY);
        cardRects.emplace_back(x, y, cardW, cardH);
    }

    // 计算最大可滚动距离
    int cols = (cards.empty() ? 0 : static_cast<int>((cards.size() - 1) / static_cast<size_t>(rows)) + 1);
    float contentW = cols > 0 ? (cols * cardW + (cols - 1) * gapX) : 0.f;
    maxScrollX = std::max(0.f, contentW - gridRect.width);
    if (scrollX > maxScrollX) scrollX = maxScrollX;

    // 详情弹窗布局
    float dw = w * 0.62f;
    float dh = h * 0.55f;
    detailRect = sf::FloatRect((w - dw) * 0.5f, (h - dh) * 0.5f, dw, dh);
    detailCloseRect = sf::FloatRect(detailRect.left + detailRect.width - padding * 3.0f, detailRect.top + padding, padding * 2.0f, padding * 2.0f);
}

void PlantListScene::handleEvent(const sf::Event& event) {
    if (event.type == sf::Event::Resized) {
        buildLayout({event.size.width, event.size.height});
        return;
    }

    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Escape) {
            if (detailOpen) {
                detailOpen = false;
                detailIndex = -1;
                return;
            }
            manager->requestChangeScene(std::make_unique<MenuScene>(manager));
            return;
        }

        if (!detailOpen && event.key.code == sf::Keyboard::Left) {
            if (!holdLeft) {
                scrollX = std::max(0.f, scrollX - gridRect.width * 0.35f);
            }
            holdLeft = true;
            return;
        }
        if (!detailOpen && event.key.code == sf::Keyboard::Right) {
            if (!holdRight) {
                scrollX = std::min(maxScrollX, scrollX + gridRect.width * 0.35f);
            }
            holdRight = true;
            return;
        }
    }

    if (event.type == sf::Event::KeyReleased) {
        if (event.key.code == sf::Keyboard::Left) holdLeft = false;
        if (event.key.code == sf::Keyboard::Right) holdRight = false;
    }

    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f p(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));
        if (detailOpen) {
            if (!detailRect.contains(p) || detailCloseRect.contains(p)) {
                detailOpen = false;
                detailIndex = -1;
            }
            return;
        }
        if (backRect.contains(p)) {
            manager->requestChangeScene(std::make_unique<MenuScene>(manager));
            return;
        }

        // 点击卡片：打开详情（如果你想拖拽时不误触，这里会在 MouseButtonReleased 再判断）
        if (gridRect.contains(p)) {
            dragging = true;
            dragStartMouseX = p.x;
            dragStartScrollX = scrollX;
            return;
        }
    }

    if (event.type == sf::Event::MouseMoved) {
        if (dragging && !detailOpen) {
            float mx = static_cast<float>(event.mouseMove.x);
            float dx = mx - dragStartMouseX;
            scrollX = dragStartScrollX - dx;
            if (scrollX < 0.f) scrollX = 0.f;
            if (scrollX > maxScrollX) scrollX = maxScrollX;
        }
    }

    if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
        if (!dragging) return;
        dragging = false;

        if (detailOpen) return;

        // 如果释放时位移很小，则视为点击卡片
        float mx = static_cast<float>(event.mouseButton.x);
        float moved = std::abs(mx - dragStartMouseX);
        if (moved > 8.f) return;

        sf::Vector2f p(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));
        for (size_t i = 0; i < cardRects.size(); ++i) {
            sf::FloatRect rr = cardRects[i];
            rr.left -= scrollX;
            if (rr.contains(p)) {
                detailOpen = true;
                detailIndex = static_cast<int>(i);
                return;
            }
        }
    }
}

void PlantListScene::render(sf::RenderTarget& target) {
    float viewW = static_cast<float>(target.getSize().x);
    float viewH = static_cast<float>(target.getSize().y);

    {
        sf::RectangleShape bg;
        bg.setPosition(0.f, 0.f);
        bg.setSize({viewW, viewH});
        bg.setFillColor(sf::Color(245, 245, 248));
        target.draw(bg);
    }

    // 返回按钮
    {
        sf::RectangleShape btn;
        btn.setPosition(backRect.left, backRect.top);
        btn.setSize({backRect.width, backRect.height});
        btn.setFillColor(sf::Color(240, 240, 244));
        btn.setOutlineColor(sf::Color(160, 160, 170));
        btn.setOutlineThickness(2.f);
        target.draw(btn);

        if (manager && manager->getSharedFont()) {
            sf::Text t;
            t.setFont(*manager->getSharedFont());
            t.setString(GBKToSFString("退出"));
            t.setCharacterSize(static_cast<unsigned int>(std::max(14.f, backRect.height * 0.45f)));
            t.setFillColor(sf::Color(40, 40, 50));
            auto b = t.getLocalBounds();
            t.setPosition(
                backRect.left + (backRect.width - b.width) * 0.5f - b.left,
                backRect.top + (backRect.height - b.height) * 0.5f - b.top
            );
            target.draw(t);
        }
    }

    target.draw(titleText);

    // 卡片网格（图片 + 名字 + 阳光消耗）
    for (size_t i = 0; i < cardRects.size(); ++i) {
        sf::FloatRect r = cardRects[i];
        r.left -= scrollX;

        // 视口裁剪：不在可视范围就不画
        if (r.left + r.width < gridRect.left || r.left > gridRect.left + gridRect.width) continue;

        sf::RectangleShape card;
        card.setPosition(r.left, r.top);
        card.setSize({r.width, r.height});
        card.setFillColor(sf::Color::White);
        card.setOutlineColor(sf::Color(170, 170, 180));
        card.setOutlineThickness(2.f);
        target.draw(card);

        float footerH = std::max(28.f, r.height * 0.24f);
        sf::RectangleShape footer;
        float footerTop = r.top + r.height - footerH;
        footer.setPosition(r.left, footerTop);
        footer.setSize({r.width, footerH});
        footer.setFillColor(sf::Color(245, 245, 248));
        target.draw(footer);

        const PlantCard* cardInfo = (i < cards.size()) ? &cards[i] : nullptr;

        if (cardInfo && cardInfo->texture) {
            sf::Sprite sp;
            sp.setTexture(*cardInfo->texture);
            auto tsz = cardInfo->texture->getSize();
            if (tsz.x > 0 && tsz.y > 0) {
                float maxW = r.width * 0.85f;
                float maxH = (r.height - footerH) * 0.80f;
                float sx = maxW / static_cast<float>(tsz.x);
                float sy = maxH / static_cast<float>(tsz.y);
                float s = std::min(sx, sy);
                sp.setScale(s, s);
                float imgW = tsz.x * s;
                float imgH = tsz.y * s;
                sp.setPosition(r.left + (r.width - imgW) * 0.5f, r.top + (r.height - footerH - imgH) * 0.18f);
                target.draw(sp);
            }
        }

        if (manager && manager->getSharedFont()) {
            sf::Text name;
            name.setFont(*manager->getSharedFont());
            if (cardInfo) name.setString(GBKToSFString(cardInfo->name));
            else name.setString(GBKToSFString("(空)"));
            name.setCharacterSize(static_cast<unsigned int>(std::max(12.f, r.height * 0.12f)));
            name.setFillColor(sf::Color(60, 60, 70));
            auto nb = name.getLocalBounds();
            name.setPosition(r.left + 10.f, footerTop + 3.f - nb.top);
            target.draw(name);

            sf::Text cost;
            cost.setFont(*manager->getSharedFont());
            if (cardInfo) cost.setString(GBKToSFString(std::string("消耗阳光 ") + std::to_string(cardInfo->sunCost)));
            else cost.setString(GBKToSFString(""));
            cost.setCharacterSize(static_cast<unsigned int>(std::max(11.f, footerH * 0.45f)));
            cost.setFillColor(sf::Color(90, 90, 100));
            auto cb = cost.getLocalBounds();
            cost.setPosition(r.left + 10.f, footerTop + footerH - cb.height - 4.f - cb.top);
            target.draw(cost);
        }
    }

    // 详情弹窗覆盖层
    if (detailOpen) {
        sf::RectangleShape mask;
        mask.setPosition(0.f, 0.f);
        mask.setSize({viewW, viewH});
        mask.setFillColor(sf::Color(0, 0, 0, 140));
        target.draw(mask);

        sf::RectangleShape panel;
        panel.setPosition(detailRect.left, detailRect.top);
        panel.setSize({detailRect.width, detailRect.height});
        panel.setFillColor(sf::Color(250, 250, 252));
        panel.setOutlineColor(sf::Color(160, 160, 170));
        panel.setOutlineThickness(2.f);
        target.draw(panel);

        sf::RectangleShape closeBtn;
        closeBtn.setPosition(detailCloseRect.left, detailCloseRect.top);
        closeBtn.setSize({detailCloseRect.width, detailCloseRect.height});
        closeBtn.setFillColor(sf::Color(240, 240, 244));
        closeBtn.setOutlineColor(sf::Color(160, 160, 170));
        closeBtn.setOutlineThickness(2.f);
        target.draw(closeBtn);

        if (manager && manager->getSharedFont()) {
            sf::Text x;
            x.setFont(*manager->getSharedFont());
            x.setString(GBKToSFString("X"));
            x.setCharacterSize(static_cast<unsigned int>(std::max(14.f, detailCloseRect.height * 0.70f)));
            x.setFillColor(sf::Color(60, 60, 70));
            auto xb = x.getLocalBounds();
            x.setPosition(
                detailCloseRect.left + (detailCloseRect.width - xb.width) * 0.5f - xb.left,
                detailCloseRect.top + (detailCloseRect.height - xb.height) * 0.5f - xb.top
            );
            target.draw(x);

            if (detailIndex >= 0 && detailIndex < static_cast<int>(cards.size())) {
                const auto& c = cards[detailIndex];

                sf::Text h;
                h.setFont(*manager->getSharedFont());
                h.setString(GBKToSFString(c.name));
                h.setCharacterSize(30);
                h.setFillColor(sf::Color(30, 30, 35));
                h.setPosition(detailRect.left + 24.f, detailRect.top + 18.f);
                target.draw(h);

                sf::Text cost;
                cost.setFont(*manager->getSharedFont());
                cost.setString(GBKToSFString(std::string("消耗阳光：") + std::to_string(c.sunCost)));
                cost.setCharacterSize(20);
                cost.setFillColor(sf::Color(70, 70, 80));
                cost.setPosition(detailRect.left + 24.f, detailRect.top + 62.f);
                target.draw(cost);

                sf::Text desc;
                desc.setFont(*manager->getSharedFont());
                desc.setString(GBKToSFString(c.desc));
                desc.setCharacterSize(20);
                desc.setFillColor(sf::Color(70, 70, 80));
                desc.setPosition(detailRect.left + 24.f, detailRect.top + 110.f);
                target.draw(desc);
            }
        }
    }
}
