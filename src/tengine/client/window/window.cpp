#include "window.h"
#include <stdexcept>

namespace tungsten::window
{

// ==================== Конструкторы ====================

Window::Window() = default;

Window::Window(
    const std::string& title,
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
        title.c_str(),
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


void Window::processEvent(const SDL_Event& event)
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

void Window::setSize(int width, int height)
{
    if (window)
        SDL_SetWindowSize(window, width, height);
}


void Window::getSize(int& width, int& height)
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

void Window::setTitle(const std::string& title)
{
    if (window)
        SDL_SetWindowTitle(window, title.c_str());
}

void Window::setRelativeMode(bool relative)
{
    SDL_SetRelativeMouseMode(relative ? SDL_TRUE : SDL_FALSE);
}

bool Window::getRelativeMode() const
{
    return SDL_GetRelativeMouseMode() == SDL_TRUE;
}

void Window::setFullscreen(bool fullscreen)
{
    if (!window)
        return;

    SDL_SetWindowFullscreen
    (
        window,
        fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0
    );
}

bool Window::getFullscreen() const
{
    if (!window)
        return false;

    return (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN_DESKTOP) != 0;
}


SDL_Window* Window::getWindow() const
{
    return window;
}

}
