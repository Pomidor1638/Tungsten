#ifndef _MEGA_GAME_SYSTEM_H_
#define _MEGA_GAME_SYSTEM_H_

#include <SDL2/SDL.h>
#include <string>
#include <list>
#include <filesystem>
#include <cstdint>

typedef uint8_t byte;

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


std::list<std::filesystem::path> getFiles(const std::filesystem::path& dir);

std::vector<byte> load_file(const std::string& path);

std::string choose_file();

#endif // _MEGA_GAME_SYSTEM_H_