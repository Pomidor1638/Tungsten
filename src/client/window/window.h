#ifndef MEGAGAME_WINDOW_H
#define MEGAGAME_WINDOW_H

#include <SDL2/SDL.h>
#include <string>

class Window
{
public:
    Window();
    Window(const std::string& title, int x, int y, int width, int height, Uint32 flags);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    void processEvent(SDL_Event& event);

    void setSize(int  width, int  height);
    void getSize(int& width, int& height);
    void setTitle(const std::string& title);

    void setRelativeMode(bool relative);
    [[nodiscard]] bool getRelativeMode() const;

    void setFullscreen(bool fullscreen);
    [[nodiscard]] bool getFullscreen() const;

    [[nodiscard]] SDL_Window* getWindow() const;

private:
    SDL_Window* window = nullptr;
};

#endif // MEGAGAME_WINDOW_H
