#include "window.h"
#include <stdexcept>

namespace tungsten::window
{

// ==================== Конструкторы ====================

Window::Window() = default;

Window::Window(
    const char* title,
    int x,
    int y,
    int width,
    int height,
    Uint32 flags
)
    : window(nullptr)
{
    SDL_ClearError();

    this->window = SDL_CreateWindow(
        title.data(),
        x,
        y,
        width,
        height,
        flags
    );
    if (!this->window)
    {
        throw std::runtime_error(
            std::string("SDL_CreateWindow failed: ") + SDL_GetError()
        );
    }
}

Window::~Window()
{
    if (window)
    {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
}


void Window::process_event(const SDL_Event& event)
{
    switch (event.type)
    {
        case SDL_QUIT:
            break;
        case SDL_WINDOWEVENT:
            switch (event.window.event)
            {
                case SDL_WINDOWEVENT_CLOSE:
                    break;
                case SDL_WINDOWEVENT_RESIZED:
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

// ==================== Управление окном ====================

void Window::set_size(int width, int height)
{
    if (window)
        SDL_SetWindowSize(window, width, height);
}


void Window::get_size(int& width, int& height)
{
    if (window)
    {
        SDL_GetWindowSize(window, &width, &height);
    }
    else
    {
        width = 0;
        height = 0;
    }
}

void Window::set_title(const std::string& title)
{
    if (window)
        SDL_SetWindowTitle(window, title.c_str());
}

void Window::set_relative_mode(bool relative)
{
    SDL_SetRelativeMouseMode(relative ? SDL_TRUE : SDL_FALSE);
}

bool Window::get_relative_mode() const
{
    return SDL_GetRelativeMouseMode() == SDL_TRUE;
}

void Window::set_fullscreen(bool fullscreen)
{
    if (!window)
        return;

    SDL_SetWindowFullscreen
    (
        window,
        fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0
    );
}

bool Window::get_fullscreen() const
{
    if (!window)
        return false;

    return (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN_DESKTOP) != 0;
}


SDL_Window* Window::get_handle() const
{
    return window;
}

}
