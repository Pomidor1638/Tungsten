#ifndef _MEGA_GAME_SYSTEM_H_
#define _MEGA_GAME_SYSTEM_H_

#include <SDL2/SDL.h>
#include <string>
#include <list>
#include <filesystem>
#include <cstdint>
#include <vector>

bool ShowCheckboxInfo
(
    const std::string& title,
    const std::string& message,
    const std::string& checkboxText,
    bool defaultChecked = false
);

bool ShowCheckboxWarning
(
    const std::string& title,
    const std::string& message,
    const std::string& checkboxText,
    bool defaultChecked = false
);

bool ShowCheckboxError
(
    const std::string& title,
    const std::string& message,
    const std::string& checkboxText,
    bool defaultChecked = false
);

bool ShowCheckboxQuestion
(
    const std::string& title,
    const std::string& message,
    const std::string& checkboxText,
    bool defaultChecked = false
);

// bool system_question();
// bool system_error();
// bool sys_warning();
// bool sys_info();

std::list<std::filesystem::path> getFiles(const std::filesystem::path& dir);

std::vector<uint8_t> load_file(const std::string& path);

std::string choose_file();

#endif // _MEGA_GAME_SYSTEM_H_
