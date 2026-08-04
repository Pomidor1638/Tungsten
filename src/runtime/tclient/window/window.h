#pragma once

#include <SDL2/SDL.h>
#include <string_view>

namespace tungsten::window
{
    class Window
    {
    public:
        Window();
        Window(const char* title, int x, int y, int width, int height, Uint32 flags);
        virtual ~Window();

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        void process_event(const SDL_Event& event);

        void set_size(int  width, int  height);
        void get_size(int& width, int& height);
        void set_title(const std::string& title);

        void set_relative_mode(bool relative);
        bool get_relative_mode() const;

        void set_fullscreen(bool fullscreen);
        bool get_fullscreen() const;

        SDL_Window* get_handle() const;

    private:
        SDL_Window* window = nullptr;
    };
}