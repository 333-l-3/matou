#include "Level1Scene.hpp"

#include "LevelSelectScene.hpp"
#include "SceneManager.hpp"
#include "file.hpp"
#include "EncodingTool.hpp"
#include "ResourceManager.hpp"
#include "LevelContentRegistry.hpp"

#include <SFML/Window.hpp>
#include <algorithm>
#include <array>
#include <cstdio>
#include <cmath>
#include <iostream>
#include <fstream>

namespace {

constexpr float kBaseW = 1000.f;
constexpr float kBaseH = 600.f;
constexpr int kPointRows = 6;
constexpr int kPointCols = 10;

using PointGrid = std::array<std::array<sf::Vector2f, kPointCols>, kPointRows>;

const PointGrid kBaseGrid = {{
    {{
        {255.f, 82.f}, {337.f, 76.f}, {406.f, 72.f}, {497.f, 67.f}, {569.f, 67.f},
        {659.f, 68.f}, {739.f, 69.f}, {813.f, 75.f}, {892.f, 83.f}, {974.f, 88.f}
    }},
    {{
        {252.f, 173.f}, {335.f, 175.f}, {410.f, 176.f}, {495.f, 171.f}, {573.f, 178.f},
        {656.f, 178.f}, {738.f, 177.f}, {810.f, 179.f}, {890.f, 180.f}, {983.f, 177.f}
    }},
    {{
        {253.f, 276.f}, {329.f, 277.f}, {409.f, 274.f}, {496.f, 276.f}, {574.f, 275.f},
        {654.f, 281.f}, {738.f, 276.f}, {812.f, 277.f}, {897.f, 278.f}, {989.f, 277.f}
    }},
    {{
        {251.f, 375.f}, {327.f, 373.f}, {409.f, 378.f}, {494.f, 382.f}, {573.f, 384.f},
        {653.f, 384.f}, {737.f, 382.f}, {810.f, 383.f}, {901.f, 386.f}, {990.f, 384.f}
    }},
    {{
        {246.f, 470.f}, {332.f, 473.f}, {411.f, 472.f}, {497.f, 473.f}, {572.f, 474.f},
        {659.f, 469.f}, {734.f, 470.f}, {817.f, 467.f}, {897.f, 468.f}, {991.f, 467.f}
    }},
    {{
        {237.f, 567.f}, {333.f, 572.f}, {409.f, 573.f}, {499.f, 572.f}, {584.f, 571.f},
        {666.f, 568.f}, {738.f, 568.f}, {817.f, 569.f}, {897.f, 570.f}, {991.f, 573.f}
    }}
}};

float cross2d(const sf::Vector2f& a, const sf::Vector2f& b, const sf::Vector2f& p) {
    sf::Vector2f ab = b - a;
    sf::Vector2f ap = p - a;
    return ab.x * ap.y - ab.y * ap.x;
}

bool pointInTriangle(const sf::Vector2f& p, const sf::Vector2f& a, const sf::Vector2f& b, const sf::Vector2f& c) {
    float c1 = cross2d(a, b, p);
    float c2 = cross2d(b, c, p);
    float c3 = cross2d(c, a, p);
    bool hasNeg = (c1 < 0.f) || (c2 < 0.f) || (c3 < 0.f);
    bool hasPos = (c1 > 0.f) || (c2 > 0.f) || (c3 > 0.f);
    return !(hasNeg && hasPos);
}

bool pointInQuad(const sf::Vector2f& p, const sf::Vector2f& tl, const sf::Vector2f& tr, const sf::Vector2f& br, const sf::Vector2f& bl) {
    return pointInTriangle(p, tl, tr, br) || pointInTriangle(p, tl, br, bl);
}

float lengthOf(const sf::Vector2f& v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}

} // namespace

Level1Scene::Level1Scene(SceneManager* mgr) : manager(mgr) {}

Level1Scene::~Level1Scene() {}

void Level1Scene::onEnter() {
    std::cout << "Level1Scene: onEnter" << std::endl;

    ResourceManager* rm = manager ? manager->getResourceManager() : nullptr;
    if (rm) {
        auto tex = rm->getTexture(matou::tool::file::resourcePath("res\\Background.jpg"));
        if (tex) {
            bgTexture = tex;
            bgSprite.setTexture(*bgTexture);
        }
    }

    rightDragging = false;
    draggingPlant = false;
    draggingPlantIndex = -1;
    viewOffsetX = 0.f;
    placedPlants.clear();
    attackSystem.reset();
    waveSpawnerEnabled = false;

    zombieAnimTimeById.clear();
    zombieAnimFrameById.clear();
    zombieWalkFramesById.clear();
    zombieDieFramesById.clear();

    plantAnimTimeById.clear();
    plantAnimFrameById.clear();
    plantIdleFramesById.clear();

    const auto& cardDefs = matou::scene::levelcfg::allPlantCards();
    std::vector<PlantOption> all;
    all.reserve(cardDefs.size());
    for (const auto& def : cardDefs) {
        all.push_back({def.id, def.name, def.imageRelPath, nullptr});
    }

    if (rm) {
        for (auto& p : all) {
            p.texture = rm->getTexture(matou::tool::file::resourcePath(p.imageRelPath));
            if (p.texture) p.texture->setSmooth(false);
        }
    }

    plantOptions.clear();
    const std::vector<int>& loadout = manager ? manager->getLoadoutPlants() : std::vector<int>{};
    if (!loadout.empty()) {
        for (int id : loadout) {
            if (id >= 0 && id < static_cast<int>(all.size())) plantOptions.push_back(all[static_cast<size_t>(id)]);
            if (plantOptions.size() >= static_cast<size_t>(matou::scene::levelcfg::maxLoadoutCards())) break;
        }
    }
    if (plantOptions.empty()) {
        for (size_t i = 0; i < all.size() && i < static_cast<size_t>(matou::scene::levelcfg::maxLoadoutCards()); ++i) {
            plantOptions.push_back(all[i]);
        }
    }

    bool battleReady = battleSim.initialize(
        matou::tool::file::resourcePath("res\\config\\plant_stats.csv"),
        matou::tool::file::resourcePath("res\\config\\zombie_stats.csv"),
        matou::tool::file::resourcePath("res\\config\\zombie_animations.csv"));

    battleSim.addSun(matou::scene::levelcfg::initialSun());

    waveSpawnerEnabled = waveSpawner.loadFromCsv(matou::tool::file::resourcePath(matou::scene::levelcfg::level1WaveCsvPath()));
    waveSpawner.reset();

    if (!waveSpawnerEnabled) {
        for (const auto& spawn : matou::scene::levelcfg::initialZombieSpawns()) {
            bool spawned = battleReady && battleSim.spawnZombie(spawn.id, spawn.row, spawn.startX);
            if (!spawned) {
                battleSim.zombiesMutable().push_back({spawn.id, spawn.row, spawn.startX, spawn.hpFallback, 0.f, false, 0.f});
            }
        }
    }

    if (rm) {
        for (const auto& zdef : matou::scene::levelcfg::zombieAnimDefs()) {
            auto& walkFrames = zombieWalkFramesById[zdef.id];
            for (int i = 0; i < zdef.walkMaxFrames; ++i) {
                char rel[160] = {0};
                std::snprintf(rel, sizeof(rel), zdef.walkFramePattern, i);
                std::string absPath = matou::tool::file::resourcePath(rel);
                std::ifstream fin(absPath, std::ios::binary);
                if (!fin.good()) break;
                auto tex = rm->getTexture(absPath);
                if (!tex) break;
                tex->setSmooth(false);
                walkFrames.push_back(tex);
            }
            if (walkFrames.empty() && zdef.walkFallbackA && zdef.walkFallbackA[0] != '\0') {
                const std::string gifAbs = matou::tool::file::resourcePath(zdef.walkFallbackA);
                auto anim = rm->getGifAnimation(gifAbs);
                if (anim && !anim->frames.empty()) {
                    for (auto& f : anim->frames) {
                        if (!f) continue;
                        f->setSmooth(false);
                        walkFrames.push_back(f);
                    }
                }
                if (walkFrames.empty()) {
                    auto tex = rm->getTexture(gifAbs);
                    if (tex) { tex->setSmooth(false); walkFrames.push_back(tex); }
                }
            }
            if (walkFrames.empty() && zdef.walkFallbackB && zdef.walkFallbackB[0] != '\0') {
                const std::string gifAbs = matou::tool::file::resourcePath(zdef.walkFallbackB);
                auto anim = rm->getGifAnimation(gifAbs);
                if (anim && !anim->frames.empty()) {
                    for (auto& f : anim->frames) {
                        if (!f) continue;
                        f->setSmooth(false);
                        walkFrames.push_back(f);
                    }
                }
                if (walkFrames.empty()) {
                    auto tex = rm->getTexture(gifAbs);
                    if (tex) { tex->setSmooth(false); walkFrames.push_back(tex); }
                }
            }

            auto& dieFrames = zombieDieFramesById[zdef.id];
            for (int i = 0; i < zdef.dieMaxFrames; ++i) {
                char rel[160] = {0};
                std::snprintf(rel, sizeof(rel), zdef.dieFramePattern, i);
                std::string absPath = matou::tool::file::resourcePath(rel);
                std::ifstream fin(absPath, std::ios::binary);
                if (!fin.good()) break;
                auto tex = rm->getTexture(absPath);
                if (!tex) break;
                tex->setSmooth(false);
                dieFrames.push_back(tex);
            }
            if (dieFrames.empty() && zdef.dieFallback && zdef.dieFallback[0] != '\0') {
                const std::string gifAbs = matou::tool::file::resourcePath(zdef.dieFallback);
                auto anim = rm->getGifAnimation(gifAbs);
                if (anim && !anim->frames.empty()) {
                    for (auto& f : anim->frames) {
                        if (!f) continue;
                        f->setSmooth(false);
                        dieFrames.push_back(f);
                    }
                }
                if (dieFrames.empty()) {
                    auto tex = rm->getTexture(gifAbs);
                    if (tex) { tex->setSmooth(false); dieFrames.push_back(tex); }
                }
            }
        }

        for (const auto& pdef : matou::scene::levelcfg::plantAnimDefs()) {
            auto& frames = plantIdleFramesById[pdef.id];
            for (int i = 0; i < pdef.maxFrames; ++i) {
                char rel[160] = {0};
                std::snprintf(rel, sizeof(rel), pdef.framePattern, i);
                std::string absPath = matou::tool::file::resourcePath(rel);
                std::ifstream fin(absPath, std::ios::binary);
                if (!fin.good()) break;
                auto tex = rm->getTexture(absPath);
                if (!tex) break;
                tex->setSmooth(false);
                frames.push_back(tex);
            }
            if (frames.empty() && pdef.fallbackStatic && pdef.fallbackStatic[0] != '\0') {
                const std::string gifAbs = matou::tool::file::resourcePath(pdef.fallbackStatic);
                auto anim = rm->getGifAnimation(gifAbs);
                if (anim && !anim->frames.empty()) {
                    for (auto& f : anim->frames) {
                        if (!f) continue;
                        f->setSmooth(false);
                        frames.push_back(f);
                    }
                }
                if (frames.empty()) {
                    auto tex = rm->getTexture(gifAbs);
                    if (tex) { tex->setSmooth(false); frames.push_back(tex); }
                }
            }
        }

        bulletTexture = rm->getTexture(matou::tool::file::resourcePath("res\\images\\Plants\\PB01.gif"));
        if (bulletTexture) bulletTexture->setSmooth(false);
        bulletSnowTexture = rm->getTexture(matou::tool::file::resourcePath("res\\images\\Plants\\SnowPea\\SnowPea.gif"));
        if (bulletSnowTexture) bulletSnowTexture->setSmooth(false);
        bulletShroomTexture = rm->getTexture(matou::tool::file::resourcePath("res\\images\\Plants\\ScaredyShroom\\ShroomBullet.gif"));
        if (!bulletShroomTexture) bulletShroomTexture = rm->getTexture(matou::tool::file::resourcePath("res\\images\\Plants\\ShroomBullet.gif"));
        if (bulletShroomTexture) bulletShroomTexture->setSmooth(false);
        bulletFumeTexture = rm->getTexture(matou::tool::file::resourcePath("res\\images\\Plants\\FumeShroom\\FumeShroomBullet.gif"));
        if (bulletFumeTexture) bulletFumeTexture->setSmooth(false);
    }

    rebuildLayout(cachedViewSize);
}

void Level1Scene::onExit() {
    std::cout << "Level1Scene: onExit" << std::endl;
}

void Level1Scene::update(float dt) {
    if (waveSpawnerEnabled) {
        waveSpawner.update(dt, battleSim, lawnRows);
    }

    battleSim.update(dt);

    for (const auto& pair : zombieWalkFramesById) {
        const std::string& zid = pair.first;
        const auto& frames = pair.second;
        if (frames.empty()) continue;

        bool aliveInScene = false;
        for (const auto& z : battleSim.zombies()) {
            if (z.zombieId == zid) { aliveInScene = true; break; }
        }
        if (!aliveInScene) continue;

        float frameStep = 0.08f;
        if (const auto* def = matou::scene::levelcfg::findZombieAnimDef(zid)) frameStep = std::max(0.02f, def->frameStep);

        float& animTime = zombieAnimTimeById[zid];
        int& animFrame = zombieAnimFrameById[zid];
        animTime += dt;
        while (animTime >= frameStep) {
            animTime -= frameStep;
            animFrame = (animFrame + 1) % static_cast<int>(frames.size());
        }
    }

    for (const auto& pair : plantIdleFramesById) {
        const std::string& pid = pair.first;
        const auto& frames = pair.second;
        if (frames.empty()) continue;

        bool placedAlive = false;
        for (const auto& pp : placedPlants) {
            if (pp.optionIndex < 0 || pp.optionIndex >= static_cast<int>(plantOptions.size())) continue;
            if (plantOptions[static_cast<size_t>(pp.optionIndex)].id == pid) { placedAlive = true; break; }
        }
        if (!placedAlive) continue;

        float frameStep = 0.10f;
        if (const auto* def = matou::scene::levelcfg::findPlantAnimDef(pid)) frameStep = std::max(0.02f, def->frameStep);

        float& animTime = plantAnimTimeById[pid];
        int& animFrame = plantAnimFrameById[pid];
        animTime += dt;
        while (animTime >= frameStep) {
            animTime -= frameStep;
            animFrame = (animFrame + 1) % static_cast<int>(frames.size());
        }
    }

    attackSystem.update(
        dt,
        battleSim.plantsMutable(),
        battleSim.zombiesMutable(),
        battleSim.stats(),
        [](const matou::battle::PlantUnit& p) -> float {
            int r = std::clamp(p.row, 0, lawnRows - 1);
            int c = std::clamp(p.col, 0, lawnCols - 1);
            float left = kBaseGrid[static_cast<size_t>(r + 1)][static_cast<size_t>(c)].x;
            float right = kBaseGrid[static_cast<size_t>(r + 1)][static_cast<size_t>(c + 1)].x;
            return (left + right) * 0.5f;
        }
    );

    placedPlants.erase(
        std::remove_if(placedPlants.begin(), placedPlants.end(), [this](const PlacedPlant& pp) {
            for (const auto& bp : battleSim.plants()) {
                if (bp.row == pp.row && bp.col == pp.col) return false;
            }
            return true;
        }),
        placedPlants.end()
    );
}

void Level1Scene::render(sf::RenderTarget& target) {
    if (!bgTexture) return;

    rebuildLayout(target.getSize());

    float viewW = static_cast<float>(target.getSize().x);

    clampOffset(kBaseW, cachedImageWidth);
    bgSprite.setScale(cachedScale, cachedScale);
    bgSprite.setPosition(-viewOffsetX, 0.f);
    target.draw(bgSprite);

    for (const auto& p : placedPlants) {
        if (p.optionIndex < 0 || p.optionIndex >= static_cast<int>(plantOptions.size())) continue;
        if (p.row < 0 || p.row >= lawnRows || p.col < 0 || p.col >= lawnCols) continue;

        int idxTL = p.row * kPointCols + p.col;
        int idxTR = p.row * kPointCols + (p.col + 1);
        int idxBL = (p.row + 1) * kPointCols + p.col;
        int idxBR = (p.row + 1) * kPointCols + (p.col + 1);
        if (idxBR >= static_cast<int>(gridPoints.size())) continue;

        const sf::Vector2f& tl = gridPoints[static_cast<size_t>(idxTL)];
        const sf::Vector2f& tr = gridPoints[static_cast<size_t>(idxTR)];
        const sf::Vector2f& bl = gridPoints[static_cast<size_t>(idxBL)];
        const sf::Vector2f& br = gridPoints[static_cast<size_t>(idxBR)];

        const auto& option = plantOptions[static_cast<size_t>(p.optionIndex)];

        std::shared_ptr<sf::Texture> ptex = option.texture;
        auto fit = plantIdleFramesById.find(option.id);
        if (fit != plantIdleFramesById.end() && !fit->second.empty()) {
            int fidx = plantAnimFrameById[option.id] % static_cast<int>(fit->second.size());
            ptex = fit->second[static_cast<size_t>(fidx)];
        }

        if (!ptex) continue;

        sf::Sprite sp;
        sp.setTexture(*ptex);
        const auto tsz = ptex->getSize();
        if (tsz.x == 0 || tsz.y == 0) continue;

        float topW = lengthOf(tr - tl);
        float bottomW = lengthOf(br - bl);
        float leftH = lengthOf(bl - tl);
        float rightH = lengthOf(br - tr);
        float cellW = (topW + bottomW) * 0.5f;
        float cellH = (leftH + rightH) * 0.5f;

        float maxW = cellW * 0.78f;
        float maxH = cellH * 0.82f;
        float sx = maxW / static_cast<float>(tsz.x);
        float sy = maxH / static_cast<float>(tsz.y);
        float s = std::min(sx, sy);
        sp.setScale(s, s);

        float w = static_cast<float>(tsz.x) * s;
        float h = static_cast<float>(tsz.y) * s;
        sf::Vector2f bottomCenter = (bl + br) * 0.5f;
        sp.setPosition(bottomCenter.x - w * 0.5f, bottomCenter.y - h);
        target.draw(sp);
    }

    // ¶¯»­½©Ê¬£¨ÐÐ×ß/ËÀÍö£©
    for (const auto& z : battleSim.zombies()) {
        std::shared_ptr<sf::Texture> ztex;

        if (z.dying) {
            auto dit = zombieDieFramesById.find(z.zombieId);
            if (dit != zombieDieFramesById.end() && !dit->second.empty()) {
                int didx = zombieAnimFrameById[z.zombieId] % static_cast<int>(dit->second.size());
                ztex = dit->second[static_cast<size_t>(didx)];
            }
        }

        if (!ztex) {
            auto fit = zombieWalkFramesById.find(z.zombieId);
            if (fit != zombieWalkFramesById.end() && !fit->second.empty()) {
                int fidx = zombieAnimFrameById[z.zombieId] % static_cast<int>(fit->second.size());
                ztex = fit->second[static_cast<size_t>(fidx)];
            } else {
                auto nit = zombieWalkFramesById.find("normal");
                if (nit != zombieWalkFramesById.end() && !nit->second.empty()) {
                    int fidx = zombieAnimFrameById["normal"] % static_cast<int>(nit->second.size());
                    ztex = nit->second[static_cast<size_t>(fidx)];
                }
            }
        }

        if (!ztex || ztex->getSize().x == 0 || ztex->getSize().y == 0) continue;

        int r = std::clamp(z.row, 0, lawnRows - 1);
        const sf::Vector2f& leftTop = gridPoints[static_cast<size_t>(r * kPointCols)];
        const sf::Vector2f& rightTop = gridPoints[static_cast<size_t>(r * kPointCols + (kPointCols - 1))];
        const sf::Vector2f& leftBottom = gridPoints[static_cast<size_t>((r + 1) * kPointCols)];
        const sf::Vector2f& rightBottom = gridPoints[static_cast<size_t>((r + 1) * kPointCols + (kPointCols - 1))];

        float worldLeft = kBaseGrid[static_cast<size_t>(r + 1)][0].x;
        float worldRight = kBaseGrid[static_cast<size_t>(r + 1)][kPointCols - 1].x;
        float t = (worldRight - worldLeft) == 0.f ? 0.5f : (z.x - worldLeft) / (worldRight - worldLeft);
        t = std::clamp(t, 0.f, 1.f);

        float sx = z.x - viewOffsetX;
        float footY = leftBottom.y + (rightBottom.y - leftBottom.y) * t;

        sf::Sprite zs;
        zs.setTexture(*ztex);
        auto tsz = ztex->getSize();

        float rowH = std::fabs((leftBottom.y + rightBottom.y) * 0.5f - (leftTop.y + rightTop.y) * 0.5f);
        float s = (rowH * 1.1f) / static_cast<float>(tsz.y);
        zs.setScale(s, s);

        float w = static_cast<float>(tsz.x) * s;
        float h = static_cast<float>(tsz.y) * s;
        zs.setPosition(sx - w * 0.5f, footY - h);
        if (!z.dying && z.slowTimer > 0.f) zs.setColor(sf::Color(200, 230, 255));
        target.draw(zs);
    }

    // ×Óµ¯
    for (const auto& b : attackSystem.bullets()) {
        int r = std::clamp(b.row, 0, lawnRows - 1);
        const sf::Vector2f& topL = gridPoints[static_cast<size_t>(r * kPointCols)];
        const sf::Vector2f& topR = gridPoints[static_cast<size_t>(r * kPointCols + (kPointCols - 1))];
        const sf::Vector2f& bottomL = gridPoints[static_cast<size_t>((r + 1) * kPointCols)];
        const sf::Vector2f& bottomR = gridPoints[static_cast<size_t>((r + 1) * kPointCols + (kPointCols - 1))];

        float worldLeft = kBaseGrid[static_cast<size_t>(r + 1)][0].x;
        float worldRight = kBaseGrid[static_cast<size_t>(r + 1)][kPointCols - 1].x;
        float t = (worldRight - worldLeft) == 0.f ? 0.5f : (b.x - worldLeft) / (worldRight - worldLeft);
        t = std::clamp(t, 0.f, 1.f);

        float sx = b.x - viewOffsetX;
        float topY = topL.y + (topR.y - topL.y) * t;
        float bottomY = bottomL.y + (bottomR.y - bottomL.y) * t;
        float y = (topY + bottomY) * 0.5f;

        if (b.sourcePlantId == "fume") {
            float sprayW = lawnCellW * 4.0f;
            float sprayH = lawnCellH * 0.62f;

            if (bulletFumeTexture) {
                sf::Sprite fs;
                fs.setTexture(*bulletFumeTexture);
                auto tsz = bulletFumeTexture->getSize();
                if (tsz.x > 0 && tsz.y > 0) {
                    const int rowCount = 8;
                    int rowH = static_cast<int>(tsz.y / rowCount);
                    if (rowH <= 0) rowH = static_cast<int>(tsz.y);

                    float lifeRatio = 0.f;
                    if (b.maxLife > 0.0001f) {
                        lifeRatio = 1.f - std::clamp(b.life / b.maxLife, 0.f, 1.f);
                    }
                    int rowIdx = std::clamp(static_cast<int>(lifeRatio * static_cast<float>(rowCount)), 0, rowCount - 1);

                    fs.setTextureRect(sf::IntRect(0, rowIdx * rowH, static_cast<int>(tsz.x), rowH));
                    float sxScale = sprayW / static_cast<float>(tsz.x);
                    float syScale = sprayH / static_cast<float>(rowH);
                    fs.setScale(sxScale, syScale);
                    fs.setPosition(sx, y - sprayH * 0.5f);
                    fs.setColor(sf::Color(255, 255, 255, 235));
                    target.draw(fs);
                }
            } else {
                sf::RectangleShape spray;
                spray.setSize({sprayW, sprayH});
                spray.setOrigin(0.f, sprayH * 0.5f);
                spray.setPosition(sx, y);
                spray.setFillColor(sf::Color(205, 175, 235, 180));
                spray.setOutlineThickness(0.f);
                target.draw(spray);
            }
            continue;
        }

        std::shared_ptr<sf::Texture> btex = bulletTexture;
        float bulletSize = 24.f;
        if (b.sourcePlantId == "snowpea" && bulletSnowTexture) {
            btex = bulletSnowTexture;
            bulletSize = 28.f;
        } else if (b.sourcePlantId == "scaredyshroom" && bulletShroomTexture) {
            btex = bulletShroomTexture;
            bulletSize = 26.f;
        } else if (b.sourcePlantId == "wsdr") {
            bulletSize = 21.f;
        }

        if (btex) {
            sf::Sprite bs;
            bs.setTexture(*btex);
            auto tsz = btex->getSize();
            if (tsz.x > 0 && tsz.y > 0) {
                float s = bulletSize / static_cast<float>(tsz.x);
                bs.setScale(s, s);
                float w = tsz.x * s;
                float h = tsz.y * s;
                bs.setPosition(sx - w * 0.5f, y - h * 0.5f);
                if (b.sourcePlantId == "repeater" || b.sourcePlantId == "wsdr") bs.setColor(sf::Color(180, 255, 180));
                target.draw(bs);
            }
        }
    }

    // ×ó²àÊúÁÐ¿¨À¸
    {
        sf::RectangleShape bar;
        bar.setPosition(plantBarRect.left, plantBarRect.top);
        bar.setSize({plantBarRect.width, plantBarRect.height});
        bar.setFillColor(sf::Color(250, 248, 235, 220));
        bar.setOutlineColor(sf::Color(165, 145, 85));
        bar.setOutlineThickness(2.f);
        target.draw(bar);
    }

    for (size_t i = 0; i < plantCardRects.size() && i < plantOptions.size(); ++i) {
        const sf::FloatRect& r = plantCardRects[i];

        sf::RectangleShape card;
        card.setPosition(r.left, r.top);
        card.setSize({r.width, r.height});
        card.setFillColor(sf::Color(255, 255, 255, 245));
        card.setOutlineColor(sf::Color(145, 125, 80));
        card.setOutlineThickness(2.f);
        target.draw(card);

        if (plantOptions[i].texture) {
            sf::Sprite sp;
            sp.setTexture(*plantOptions[i].texture);
            auto tsz = plantOptions[i].texture->getSize();
            if (tsz.x > 0 && tsz.y > 0) {
                float maxW = r.width * 0.62f;
                float maxH = r.height * 0.70f;
                float sx = maxW / static_cast<float>(tsz.x);
                float sy = maxH / static_cast<float>(tsz.y);
                float s = std::min(sx, sy);
                sp.setScale(s, s);
                float h = static_cast<float>(tsz.y) * s;
                sp.setPosition(r.left + 6.f, r.top + (r.height - h) * 0.5f);
                target.draw(sp);
            }
        }

        if (manager && manager->getSharedFont()) {
            sf::Text name;
            name.setFont(*manager->getSharedFont());
            name.setString(GBKToSFString(plantOptions[i].name));
            name.setCharacterSize(11);
            name.setFillColor(sf::Color(45, 45, 55));
            auto b = name.getLocalBounds();
            name.setPosition(r.left + r.width * 0.38f - b.left, r.top + (r.height - b.height) * 0.5f - b.top);
            target.draw(name);
        }
    }

    if (draggingPlant && draggingPlantIndex >= 0 && draggingPlantIndex < static_cast<int>(plantOptions.size())) {
        const auto& option = plantOptions[static_cast<size_t>(draggingPlantIndex)];
        if (option.texture) {
            sf::Sprite ghost;
            ghost.setTexture(*option.texture);
            auto tsz = option.texture->getSize();
            if (tsz.x > 0 && tsz.y > 0) {
                int hoverRow = -1;
                int hoverCol = -1;
                bool inCell = findCellByPoint(dragMousePos, hoverRow, hoverCol);

                float maxW = lawnCellW * 0.90f;
                float maxH = lawnCellH * 0.90f;
                float sx = maxW / static_cast<float>(tsz.x);
                float sy = maxH / static_cast<float>(tsz.y);
                float s = std::min(sx, sy);
                ghost.setScale(s, s);
                ghost.setColor(inCell ? sf::Color(255, 255, 255, 170) : sf::Color(255, 120, 120, 170));

                float w = static_cast<float>(tsz.x) * s;
                float h = static_cast<float>(tsz.y) * s;
                ghost.setPosition(dragMousePos.x - w * 0.5f, dragMousePos.y - h * 0.5f);
                target.draw(ghost);
            }
        }
    }

    sf::Text info;
    if (manager && manager->getSharedFont()) info.setFont(*manager->getSharedFont());
    info.setString(GBKToSFString("º®±ù¼õËÙ/Ë«·¢Á¬Éä/Ó£ÌÒ±¬Õ¨/½©Ê¬ËÀÍö¶¯»­ÒÑ½ÓÈë"));
    info.setCharacterSize(12);
    info.setFillColor(sf::Color(200, 200, 200));
    auto ib = info.getLocalBounds();
    float ipadding = 12.f;
    info.setPosition(viewW - ib.width - ipadding - ib.left, 10.f - ib.top);
    target.draw(info);
}

bool Level1Scene::usesWindowBackground() const {
    return false;
}

void Level1Scene::clampOffset(float viewW, float imgW) {
    if (imgW <= viewW) {
        viewOffsetX = 0.f;
        return;
    }
    float maxOffset = imgW - viewW;
    if (viewOffsetX < 0.f) viewOffsetX = 0.f;
    if (viewOffsetX > maxOffset) viewOffsetX = maxOffset;
}

void Level1Scene::rebuildLayout(sf::Vector2u viewSize) {
    cachedViewSize = viewSize;
    if (!bgTexture) return;

    auto tsz = bgTexture->getSize();
    if (tsz.y == 0) return;

    cachedScale = kBaseH / static_cast<float>(tsz.y);
    cachedImageWidth = static_cast<float>(tsz.x) * cachedScale;
    clampOffset(kBaseW, cachedImageWidth);

    gridPoints.clear();
    gridPoints.reserve(static_cast<size_t>(kPointRows * kPointCols));

    float minX = 1e9f, minY = 1e9f;
    float maxX = -1e9f, maxY = -1e9f;

    for (int r = 0; r < kPointRows; ++r) {
        for (int c = 0; c < kPointCols; ++c) {
            sf::Vector2f p(kBaseGrid[static_cast<size_t>(r)][static_cast<size_t>(c)].x - viewOffsetX,
                           kBaseGrid[static_cast<size_t>(r)][static_cast<size_t>(c)].y);
            gridPoints.push_back(p);
            minX = std::min(minX, p.x);
            minY = std::min(minY, p.y);
            maxX = std::max(maxX, p.x);
            maxY = std::max(maxY, p.y);
        }
    }

    lawnRect = sf::FloatRect(minX, minY, std::max(1.f, maxX - minX), std::max(1.f, maxY - minY));
    lawnCellW = lawnRect.width / static_cast<float>(lawnCols);
    lawnCellH = lawnRect.height / static_cast<float>(lawnRows);

    plantBarRect = sf::FloatRect(12.f, 12.f, 132.f, 576.f);

    plantCardRects.clear();
    int count = static_cast<int>(plantOptions.size());
    if (count <= 0) return;

    float gap = 4.f;
    float cardW = 108.f;
    float cardH = (560.f - gap * static_cast<float>(count + 1)) / static_cast<float>(count);
    cardH = std::clamp(cardH, 24.f, 72.f);

    float y = 16.f + gap;
    float x = 24.f;
    for (int i = 0; i < count; ++i) {
        plantCardRects.emplace_back(x, y, cardW, cardH);
        y += cardH + gap;
    }
}

int Level1Scene::hitPlantCard(const sf::Vector2f& p) const {
    if (!plantBarRect.contains(p)) return -1;
    for (size_t i = 0; i < plantCardRects.size(); ++i) {
        if (plantCardRects[i].contains(p)) return static_cast<int>(i);
    }
    return -1;
}

bool Level1Scene::findCellByPoint(const sf::Vector2f& p, int& outRow, int& outCol) const {
    if (gridPoints.size() < static_cast<size_t>(kPointRows * kPointCols)) return false;

    for (int r = 0; r < lawnRows; ++r) {
        for (int c = 0; c < lawnCols; ++c) {
            const sf::Vector2f& tl = gridPoints[static_cast<size_t>(r * kPointCols + c)];
            const sf::Vector2f& tr = gridPoints[static_cast<size_t>(r * kPointCols + c + 1)];
            const sf::Vector2f& bl = gridPoints[static_cast<size_t>((r + 1) * kPointCols + c)];
            const sf::Vector2f& br = gridPoints[static_cast<size_t>((r + 1) * kPointCols + c + 1)];
            if (pointInQuad(p, tl, tr, br, bl)) {
                outRow = r;
                outCol = c;
                return true;
            }
        }
    }
    return false;
}

Level1Scene::PlacedPlant* Level1Scene::findPlacedPlant(int row, int col) {
    for (auto& p : placedPlants) {
        if (p.row == row && p.col == col) return &p;
    }
    return nullptr;
}

void Level1Scene::handleEvent(const sf::Event& event) {
    if (event.type == sf::Event::Resized) {
        rebuildLayout({event.size.width, event.size.height});
        return;
    }

    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Right) {
        rightDragging = true;
        lastMouseX = static_cast<float>(event.mouseButton.x);
        return;
    }
    if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Right) {
        rightDragging = false;
        return;
    }

    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f p(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));
        dragMousePos = p;
        int hit = hitPlantCard(p);
        if (hit >= 0) {
            draggingPlant = true;
            draggingPlantIndex = hit;
            return;
        }
        return;
    }

    if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f p(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));
        dragMousePos = p;

        if (draggingPlant) {
            int row = -1;
            int col = -1;
            if (findCellByPoint(p, row, col)) {
                if (!findPlacedPlant(row, col)) {
                    if (draggingPlantIndex >= 0 && draggingPlantIndex < static_cast<int>(plantOptions.size())) {
                        const auto& opt = plantOptions[static_cast<size_t>(draggingPlantIndex)];
                        bool ok = battleSim.placePlant(opt.id, row, col);
                        if (!ok) ok = battleSim.forcePlacePlant(opt.id, row, col);
                        if (ok) {
                            placedPlants.push_back({draggingPlantIndex, row, col});
                        }
                    }
                }
            }
        }

        draggingPlant = false;
        draggingPlantIndex = -1;
        return;
    }

    if (event.type == sf::Event::MouseMoved) {
        dragMousePos = {static_cast<float>(event.mouseMove.x), static_cast<float>(event.mouseMove.y)};
        if (rightDragging) {
            float mx = static_cast<float>(event.mouseMove.x);
            float dx = mx - lastMouseX;
            viewOffsetX -= dx;
            lastMouseX = mx;
            rebuildLayout(cachedViewSize);
        }
        return;
    }

    if (event.type == sf::Event::KeyPressed) {
        const float step = 100.f;
        if (event.key.code == sf::Keyboard::Left) viewOffsetX -= step;
        else if (event.key.code == sf::Keyboard::Right) viewOffsetX += step;
        else if (event.key.code == sf::Keyboard::Escape) {
            manager->requestChangeScene(std::make_unique<LevelSelectScene>(manager));
            return;
        }
        rebuildLayout(cachedViewSize);
    }
}















