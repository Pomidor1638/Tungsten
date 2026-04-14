#include "system.h"
#include <sstream>
#include <fstream>
#include "tinyfiledialogs.h"

namespace 
{
    bool ShowSDLMessageBoxInternal
    (
        SDL_MessageBoxFlags type,
        const std::string& title,
        const std::string& message,
        const std::string& checkboxText,
        bool defaultChecked
    ) {

        std::string checkboxStatus = defaultChecked ? "[X] " : "[ ] ";
        std::string fullMessage;

        if (checkboxText.empty())
        {
            fullMessage = message;
        }
        else
        {
            fullMessage = message;
            if (!checkboxText.empty()) 
            {
                fullMessage += "\n" + checkboxStatus + checkboxText;
            }
        }

        const SDL_MessageBoxButtonData buttons[] =
        {
            { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "OK" },
            { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, "Cancel" }
        };

        SDL_MessageBoxData msgbox =
        {
            type,
            NULL,
            title.c_str(),
            fullMessage.c_str(),
            type == SDL_MESSAGEBOX_ERROR ? 1 : SDL_arraysize(buttons),
            buttons,
            NULL
        };

        int buttonID = 0;
        if (SDL_ShowMessageBox(&msgbox, &buttonID) < 0)
        {
            return defaultChecked;
        }

        return (buttonID == 1) ? defaultChecked : false;
    }
}


bool ShowCheckboxInfo
(
    const std::string& title,
    const std::string& message,
    const std::string& checkboxText,
    bool defaultChecked
) {
    return ShowSDLMessageBoxInternal(SDL_MESSAGEBOX_INFORMATION, title, message, checkboxText, defaultChecked);
}

bool ShowCheckboxWarning
(
    const std::string& title,
    const std::string& message,
    const std::string& checkboxText,
    bool defaultChecked
) {
    return ShowSDLMessageBoxInternal(SDL_MESSAGEBOX_WARNING,
        title, message, checkboxText, defaultChecked);
}

bool ShowCheckboxError
(
    const std::string& title,
    const std::string& message,
    const std::string& checkboxText,
    bool defaultChecked
) {
    return ShowSDLMessageBoxInternal(SDL_MESSAGEBOX_ERROR,
        title, message, checkboxText, defaultChecked);
}

bool ShowCheckboxQuestion
(
    const std::string& title,
    const std::string& message,
    const std::string& checkboxText,
    bool defaultChecked
) {
    return ShowSDLMessageBoxInternal(SDL_MESSAGEBOX_WARNING,
        title, message, checkboxText, defaultChecked);
}


std::list<std::filesystem::path> getFiles(const std::filesystem::path& dir)
{
    std::list<std::filesystem::path> files;
    if (!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir))
    {
        return files;
    }
    try
    {
        for (const auto& entry : std::filesystem::directory_iterator(dir))
        {
            if (entry.is_regular_file())
            {
                files.push_back(entry.path());
            }
        }
        files.sort
        (
            [](const auto& a, const auto& b) 
            {
                return a.filename().string() < b.filename().string();
            }
        );

    }
    catch (...)
    {
    }

    return files;
}


std::vector<byte> load_file(const std::string& path)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);

    if (!file.is_open())
    {
        file.close();
        throw std::runtime_error(std::string(__FUNCSIG__) + ": can't open file " + path);
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<byte> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size))
    {
        file.close();
        throw std::runtime_error(std::string(__FUNCSIG__) + ": can't read file " + path);
    }

    file.close();

    return buffer;
}

std::string choose_file()
{
    const char* lFilterPatterns[1] = { "*.bsp" };

    const char* lSelectedFile = tinyfd_openFileDialog(
        "Select Map File",
        "maps/",          
        1,
        lFilterPatterns,  
        "BSP Map Files",
        0
    );

    if (!lSelectedFile) {
        return ""; 
    }

    return std::string(lSelectedFile);
}