// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of recomp-common: https://github.com/tmcleroy/recomp-common
#pragma once
// Running the game: the port binary next to the launcher, in the player's data folder.

#include <atomic>
#include <filesystem>
#include <string>
#include <thread>
#include <vector>

class GameProcess {
public:
    ~GameProcess();
    // Starts `program args...` in `folder`; false (with `error`) if it could not start.
    // Its output (stdout and stderr) goes to `log` when given.
    bool start(const std::filesystem::path& program, const std::vector<std::string>& args, const std::filesystem::path& folder, std::string& error,
               const std::filesystem::path& log = {});
    bool running() const { return running_; }
    int exit_code() const { return exit_code_; }
    // Asks the game to end, as closing its window does (a signal: it saves what it keeps, then quits). False if it is
    // not running, or the system could not be asked.
    bool stop();
    // Runs `program args...` and returns its standard output (for --list-features).
    static std::string output_of(const std::filesystem::path& program, const std::vector<std::string>& args);

private:
    std::thread waiter_;
    std::atomic<bool> running_{false};
    std::atomic<int> exit_code_{0};
    std::atomic<long long> process_{0};  // (the pid, or the handle on Windows: stopped by stop())
};
