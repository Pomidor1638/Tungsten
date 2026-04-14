
#ifndef MEGAGAME_COMMON_H
#define MEGAGAME_COMMON_H

#include "logger/logger.h"
#include "system/system.h"
#include "allocator/allocator.h"

extern LinearAllocator hunk;


extern union SDL_Event;

class Module {
public:
    
    Module() = default;
    virtual ~Module() = default;

    Module& operator=(const Module& ) = delete;
    Module           (const Module& ) = delete;
    Module           (      Module&&) = delete;

    bool init() 
    {
        if (_initialized) 
            return true;

        return _initialized = onInit();
    }


    void quit()
    {
        if (!_initialized)
            return;

        onQuit();
        _initialized = false;
    }

    virtual void startProcessEvent() = 0;
    virtual void processEvent(const SDL_Event& event) = 0;
    virtual void endProcessEvent() = 0;

    bool isInitialized() const 
    {
        return _initialized; 
    }

protected:
    virtual bool onInit() = 0;
    virtual void onQuit() = 0;
private:
    bool _initialized = false;
};



#endif