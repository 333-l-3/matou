#ifndef RESOURCEMANAGER_HPP
#define RESOURCEMANAGER_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>
#include <memory>
#include <iostream>
#include <algorithm>
#include <vector>
#include <deque>
#include <cctype>
#include <cstdlib>
#ifdef _WIN32
#include <windows.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
#endif

class ResourceManager {
public:
    struct GifAnimation {
        std::vector<std::shared_ptr<sf::Texture>> frames;
        std::vector<float> delays;
    };

    ResourceManager() {
#ifdef _WIN32
        Gdiplus::GdiplusStartupInput gdiplusStartupInput;
        if (Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr) == Gdiplus::Ok) {
            gdiplusReady = true;
        }
#endif
    }

    ~ResourceManager() {
#ifdef _WIN32
        if (gdiplusReady) {
            Gdiplus::GdiplusShutdown(gdiplusToken);
        }
#endif
    }

    std::shared_ptr<sf::Texture> getTexture(const std::string& path) {
        auto it = textures.find(path);
        if (it != textures.end()) {
            return it->second;
        }

        std::string lowerPath = path;
        std::transform(lowerPath.begin(), lowerPath.end(), lowerPath.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        const bool isGif = lowerPath.size() >= 4 && lowerPath.substr(lowerPath.size() - 4) == ".gif";
        if (isGif) {
            auto anim = getGifAnimation(path);
            if (anim && !anim->frames.empty()) {
                textures[path] = anim->frames.front();
                return anim->frames.front();
            }
        }

        auto tex = std::make_shared<sf::Texture>();

        sf::Image image;
        if (!image.loadFromFile(path)) {
            std::cerr << "ResourceManager: failed to load texture '" << path << "'" << std::endl;
            return nullptr;
        }

        // Many extracted PvZ GIF assets encode transparency as black.
        // Strip pure-black background for Plants/Zombies runtime sprites.
        std::string lower = path;
        std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        bool shouldMaskBlack =
            (lower.find("res\\images\\plants\\") != std::string::npos ||
             lower.find("res\\images\\zombies\\") != std::string::npos) &&
            (lower.size() >= 4 && lower.substr(lower.size() - 4) == ".gif");
        if (shouldMaskBlack) {
            const sf::Vector2u sz = image.getSize();
            if (sz.x > 0 && sz.y > 0) {
                const sf::Color key = image.getPixel(0, 0);
                auto nearKey = [&](const sf::Color& c) -> bool {
                    if (c.a == 0) return false;
                    const int dr = std::abs(static_cast<int>(c.r) - static_cast<int>(key.r));
                    const int dg = std::abs(static_cast<int>(c.g) - static_cast<int>(key.g));
                    const int db = std::abs(static_cast<int>(c.b) - static_cast<int>(key.b));
                    const bool nearCornerColor = (dr + dg + db) <= 30;
                    const bool nearBlack = (static_cast<int>(c.r) + static_cast<int>(c.g) + static_cast<int>(c.b)) <= 35;
                    return nearCornerColor || nearBlack;
                };

                const size_t total = static_cast<size_t>(sz.x) * static_cast<size_t>(sz.y);
                std::vector<unsigned char> visited(total, 0);
                std::deque<sf::Vector2u> q;

                auto pushIfBg = [&](unsigned x, unsigned y) {
                    const size_t idx = static_cast<size_t>(y) * static_cast<size_t>(sz.x) + static_cast<size_t>(x);
                    if (visited[idx]) return;
                    const sf::Color c = image.getPixel(x, y);
                    if (!nearKey(c)) return;
                    visited[idx] = 1;
                    q.push_back({x, y});
                };

                for (unsigned x = 0; x < sz.x; ++x) {
                    pushIfBg(x, 0);
                    pushIfBg(x, sz.y - 1);
                }
                for (unsigned y = 0; y < sz.y; ++y) {
                    pushIfBg(0, y);
                    pushIfBg(sz.x - 1, y);
                }

                while (!q.empty()) {
                    const sf::Vector2u p = q.front();
                    q.pop_front();
                    image.setPixel(p.x, p.y, sf::Color(0, 0, 0, 0));

                    if (p.x > 0) pushIfBg(p.x - 1, p.y);
                    if (p.x + 1 < sz.x) pushIfBg(p.x + 1, p.y);
                    if (p.y > 0) pushIfBg(p.x, p.y - 1);
                    if (p.y + 1 < sz.y) pushIfBg(p.x, p.y + 1);
                }
            }
        }

        if (!tex->loadFromImage(image)) {
            std::cerr << "ResourceManager: failed to create texture from image '" << path << "'" << std::endl;
            return nullptr;
        }
        textures[path] = tex;
        return tex;
    }

    std::shared_ptr<GifAnimation> getGifAnimation(const std::string& path) {
        auto it = gifAnimations.find(path);
        if (it != gifAnimations.end()) return it->second;

#ifdef _WIN32
        if (!gdiplusReady) return nullptr;

        const std::wstring wpath = toWide(path);
        Gdiplus::Image image(wpath.c_str());
        if (image.GetLastStatus() != Gdiplus::Ok) return nullptr;

        UINT dimCount = image.GetFrameDimensionsCount();
        if (dimCount == 0) return nullptr;

        std::vector<GUID> dims(dimCount);
        image.GetFrameDimensionsList(dims.data(), dimCount);
        if (dims.empty()) return nullptr;

        const UINT frameCount = image.GetFrameCount(&dims[0]);
        if (frameCount == 0) return nullptr;

        std::vector<UINT> delaysCs(frameCount, 10); // centiseconds
        const UINT propSize = image.GetPropertyItemSize(PropertyTagFrameDelay);
        if (propSize > 0) {
            std::vector<unsigned char> propBuf(propSize);
            auto* prop = reinterpret_cast<Gdiplus::PropertyItem*>(propBuf.data());
            if (image.GetPropertyItem(PropertyTagFrameDelay, propSize, prop) == Gdiplus::Ok && prop->length >= frameCount * 4) {
                const UINT* values = reinterpret_cast<const UINT*>(prop->value);
                for (UINT i = 0; i < frameCount; ++i) {
                    delaysCs[i] = values[i] == 0 ? 10 : values[i];
                }
            }
        }

        auto anim = std::make_shared<GifAnimation>();
        anim->frames.reserve(frameCount);
        anim->delays.reserve(frameCount);

        const UINT w = image.GetWidth();
        const UINT h = image.GetHeight();
        if (w == 0 || h == 0) return nullptr;

        for (UINT i = 0; i < frameCount; ++i) {
            if (image.SelectActiveFrame(&dims[0], i) != Gdiplus::Ok) continue;

            Gdiplus::Bitmap frameBmp(w, h, PixelFormat32bppARGB);
            Gdiplus::Graphics g(&frameBmp);
            g.DrawImage(&image, 0, 0, w, h);

            Gdiplus::Rect rect(0, 0, static_cast<INT>(w), static_cast<INT>(h));
            Gdiplus::BitmapData bd{};
            if (frameBmp.LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bd) != Gdiplus::Ok) continue;

            sf::Image sfimg;
            sfimg.create(w, h, sf::Color::Transparent);

            for (UINT y = 0; y < h; ++y) {
                const unsigned char* row = static_cast<const unsigned char*>(bd.Scan0) + y * bd.Stride;
                for (UINT x = 0; x < w; ++x) {
                    const unsigned char b = row[x * 4 + 0];
                    const unsigned char gch = row[x * 4 + 1];
                    const unsigned char r = row[x * 4 + 2];
                    const unsigned char a = row[x * 4 + 3];
                    sfimg.setPixel(x, y, sf::Color(r, gch, b, a));
                }
            }

            frameBmp.UnlockBits(&bd);

            auto tex = std::make_shared<sf::Texture>();
            if (!tex->loadFromImage(sfimg)) continue;
            anim->frames.push_back(tex);
            anim->delays.push_back(static_cast<float>(delaysCs[i]) / 100.f);
        }

        if (anim->frames.empty()) return nullptr;
        gifAnimations[path] = anim;
        return anim;
#else
        (void)path;
        return nullptr;
#endif
    }

    std::shared_ptr<sf::Font> getFont(const std::string& path) {
        auto it = fonts.find(path);
        if (it != fonts.end()) {
            return it->second;
        }
        auto f = std::make_shared<sf::Font>();
        if (!f->loadFromFile(path)) {
            std::cerr << "ResourceManager: failed to load font '" << path << "'" << std::endl;
            return nullptr;
        }
        fonts[path] = f;
        return f;
    }

private:
#ifdef _WIN32
    std::wstring toWide(const std::string& s) const {
        if (s.empty()) return std::wstring();
        int size = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
        if (size <= 0) {
            size = MultiByteToWideChar(CP_ACP, 0, s.c_str(), -1, nullptr, 0);
            if (size <= 0) return std::wstring(s.begin(), s.end());
            std::wstring w(static_cast<size_t>(size), L'\0');
            MultiByteToWideChar(CP_ACP, 0, s.c_str(), -1, w.data(), size);
            if (!w.empty() && w.back() == L'\0') w.pop_back();
            return w;
        }
        std::wstring w(static_cast<size_t>(size), L'\0');
        MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, w.data(), size);
        if (!w.empty() && w.back() == L'\0') w.pop_back();
        return w;
    }
#endif

    std::unordered_map<std::string, std::shared_ptr<sf::Texture>> textures;
    std::unordered_map<std::string, std::shared_ptr<sf::Font>> fonts;
    std::unordered_map<std::string, std::shared_ptr<GifAnimation>> gifAnimations;
#ifdef _WIN32
    ULONG_PTR gdiplusToken = 0;
    bool gdiplusReady = false;
#endif
};

#endif

