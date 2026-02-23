//
// Created by UBER_USER on 29.12.2025.
//

#ifndef MEGAGAME_AUDIO_H
#define MEGAGAME_AUDIO_H

#include <SDL2/SDL.h>


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


#endif //MEGAGAME_AUDIO_H