// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of recomp-common: https://github.com/tmcleroy/recomp-common
#include "art.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <system_error>

namespace fs = std::filesystem;

namespace launcher {

namespace {

// RGBA32, as a texture holds its pixels; read by SDL itself, so nothing is compiled in for it
bool read_png(const fs::path& file, std::vector<unsigned char>& bytes, int& width, int& height) {
    SDL_Surface* surface = SDL_LoadPNG(file.u8string().c_str());
    if (!surface) {
        return false;
    }
    SDL_Surface* rgba = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
    SDL_DestroySurface(surface);
    if (!rgba) {
        return false;
    }
    width = rgba->w;
    height = rgba->h;
    bytes.resize(static_cast<size_t>(rgba->w) * static_cast<size_t>(rgba->h) * 4);
    for (int y = 0; y < rgba->h; y++) {
        const auto* row = static_cast<const unsigned char*>(rgba->pixels) + static_cast<size_t>(y) * static_cast<size_t>(rgba->pitch);
        std::copy(row, row + static_cast<size_t>(rgba->w) * 4, bytes.begin() + static_cast<size_t>(y) * static_cast<size_t>(rgba->w) * 4);
    }
    SDL_DestroySurface(rgba);
    return width > 0 && height > 0;
}

}  // namespace

Art::Art(SDL_Window* window, const fs::path& folder) : window_(window), folder_(folder) {}

Art::~Art() {
    close();
}

void Art::close() {
    for (auto& [set, picture] : pictures_) {
        SDL_DestroyTexture(picture.colour);
        SDL_DestroyTexture(picture.grey);
    }
    pictures_.clear();
}

void Art::load(Picture& picture, const std::string& set) {
    picture.read = true;
    const fs::path file = folder_ / "art" / (set + ".png");
    std::error_code error;
    if (!fs::exists(file, error)) {
        return;
    }
    read_png(file, picture.pixels, picture.width, picture.height);
}

SDL_Texture* Art::make(Picture& picture, bool grey) const {
    SDL_Texture*& which = grey ? picture.grey : picture.colour;
    if (which) {
        return which;
    }
    std::vector<unsigned char> pixels = picture.pixels;
    if (grey) {
        // (the same picture with its colour gone: the game's set is not there)
        for (size_t at = 0; at + 3 < pixels.size(); at += 4) {
            const auto level =
                static_cast<unsigned char>(std::min(255.0f, 0.299f * pixels[at] + 0.587f * pixels[at + 1] + 0.114f * pixels[at + 2] + 0.5f));
            pixels[at] = pixels[at + 1] = pixels[at + 2] = level;
        }
    }
    which = SDL_CreateTexture(SDL_GetRenderer(window_), SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, picture.width, picture.height);
    if (which) {
        SDL_UpdateTexture(which, nullptr, pixels.data(), picture.width * 4);
        SDL_SetTextureBlendMode(which, SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(which, SDL_SCALEMODE_LINEAR);
    }
    return which;
}

SDL_Texture* Art::texture(const std::string& set, bool grey) {
    if (set.empty() || folder_.empty()) {
        return nullptr;
    }
    auto [it, inserted] = pictures_.try_emplace(set);
    Picture& picture = it->second;
    if (inserted) {
        load(picture, set);
    }
    if (!picture.width || !picture.height) {
        return nullptr;
    }
    return make(picture, grey);
}

}  // namespace launcher
