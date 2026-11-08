// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of recomp-common: https://github.com/tmcleroy/recomp-common
#pragma once
// The pictures of a collection's games on its Games page: one a set, by its name ("vsavj"), in an `art` folder beside
// the launcher (art/<set>.png). They are read as a tile is first drawn, and kept. A set with no picture there has no
// tile picture: the game's name and its set's name are drawn alone.

#include <SDL3/SDL.h>

#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace launcher {

class Art {
public:
    // `folder`: the launcher's own (the pictures are in its `art` folder)
    Art(SDL_Window* window, const std::filesystem::path& folder);
    ~Art();
    // The picture of a ROM set, or nullptr when there is none: then it is drawn in black and white (`grey`).
    SDL_Texture* texture(const std::string& set, bool grey);
    // Before the window and its renderer go: the textures are the renderer's
    void close();

    Art(const Art&) = delete;
    Art& operator=(const Art&) = delete;

private:
    struct Picture {
        bool read = false;  // (the file was looked for: it is there, or it is not)
        std::vector<unsigned char> pixels;  // RGBA32, one byte a channel
        int width = 0, height = 0;
        SDL_Texture* colour = nullptr;
        SDL_Texture* grey = nullptr;
    };
    void load(Picture& picture, const std::string& set);
    SDL_Texture* make(Picture& picture, bool grey) const;

    SDL_Window* window_;
    std::filesystem::path folder_;
    std::map<std::string, Picture> pictures_;
};

}  // namespace launcher
