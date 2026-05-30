//
// Created by UBER_USER on 29.12.2025.
//

#pragma once

#include <SDL2/SDL.h>

namespace tungsten::audio
{
    class Audio
    {
    public:

        void processEvent(const SDL_Event& e);

    private:

    public:
        Audio();
        virtual ~Audio();

        Audio(const Audio& audio) = delete;
        Audio(Audio&& audio) = delete;
    };
}