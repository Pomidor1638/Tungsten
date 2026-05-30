#pragma once

#include <SDL2/SDL.h>
#include <string>

namespace tungsten::window
{
    class Window
    {
    public:
        Window();
        Window(const std::string& title, int x, int y, int width, int height, Uint32 flags);
        virtual ~Window();

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        void processEvent(const SDL_Event& event);

        void setSize(int  width, int  height);
        void getSize(int& width, int& height);
        void setTitle(const std::string& title);

        void setRelativeMode(bool relative);
        bool getRelativeMode() const;

        void setFullscreen(bool fullscreen);
        bool getFullscreen() const;

        SDL_Window* getWindow() const;

    private:
        SDL_Window* window = nullptr;
    };
}