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
#include <thread>
#include <vector>

namespace launcher {

class Art {
public:
    // `folder` keeps the pictures (art/<set>.png); `url` has "%s" where the set's name goes
    Art(SDL_Window* window, const std::filesystem::path& folder, const std::string& url);
    ~Art();
    // The picture of a ROM set ("vsavj"), or nullptr while it is being fetched, or when there is none. `grey`: the
    // game's set is not there, and the picture is drawn in black and white. The set's picture is asked for once.
    SDL_Texture* texture(const std::string& set, bool grey);
    // A picture is being fetched
    bool working() const;
    // Before the window and its renderer go: the textures are the renderer's
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
    // What this object and the fetching thread share: the thread may outlive the object
    struct Shared {
        std::mutex mutex;
        std::map<std::string, std::unique_ptr<Picture>> pictures;
        std::vector<std::string> queue;  // (the sets to fetch, in the order they were asked for)
        std::filesystem::path folder;    // where the pictures are kept (art/<set>.png)
        bool stopped = false;
        bool no_server = false;  // (the server could not be reached: nothing more is fetched)
    };

    SDL_Texture* make(Picture& picture, bool grey) const;
    static void destroy_textures(Picture& picture);
    // The fetching thread: one picture at a time, so that a first run does not start them all at once
    static void fetch_all(std::shared_ptr<Shared> shared, std::string url);

    std::shared_ptr<Shared> shared_;
    SDL_Window* window_;
    std::string url_;
    std::thread worker_;
};

}  // namespace launcher
