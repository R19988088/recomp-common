// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of recomp-common: https://github.com/tmcleroy/recomp-common
#pragma once
// The pictures of a collection's games on its Games page (launcher::Collection::art_url): each fetched once by the
// system's curl into a folder of the player's, and drawn as a texture. Nothing of a game is compiled in or shipped
// with the launcher: a picture that is not there, or cannot be fetched, is simply left out.

#include <SDL3/SDL.h>

#include <filesystem>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace launcher {

class Art {
public:
    // `folder` keeps the pictures (art/<set>.png); `url` has "%s" where the set's name goes
    Art(SDL_Window* window, const std::filesystem::path& folder, const std::string& url);
    ~Art();
    // The picture of a ROM set ("vsavj"), or nullptr while it is being fetched, or when there is none. `grey`: the
    // game's set is not there yet, and the picture is in black and white.
    SDL_Texture* texture(const std::string& set, bool grey);
    // A picture is being fetched
    bool working() const;
    // The textures are of the renderer that is about to go
    void close();

    Art(const Art&) = delete;
    Art& operator=(const Art&) = delete;

private:
    struct Picture {
        std::vector<unsigned char> pixels;  // RGBA32, one byte a channel
        int width = 0, height = 0;
        bool ready = false;   // (the pixels are there)
        bool asked = false;   // (a fetch was asked for)
        SDL_Texture* colour = nullptr;
        SDL_Texture* grey = nullptr;
    };
    // What the fetching threads read and write: they outlive this object, which may go before they do
    struct Shared {
        std::mutex mutex;
        std::map<std::string, std::unique_ptr<Picture>> pictures;
        std::vector<std::string> waiting;
        bool stopped = false;
        bool unreachable = false;  // (the server could not be reached: the rest of the pictures are not tried)
        std::filesystem::path folder;  // where the pictures are kept (art/<set>.png)
    };

    static bool fetch(const std::filesystem::path& file, const std::string& address);
    SDL_Texture* make(Picture& picture, bool grey);
    static void destroy_textures(Picture& picture);

    std::shared_ptr<Shared> shared_;
    SDL_Window* window_;
    std::string url_;
};

}  // namespace launcher
