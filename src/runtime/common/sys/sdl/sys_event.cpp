
#include "../sys_event.h"

#include <SDL2/SDL.h>

namespace tungsten::sys
{
	namespace
	{

	}

	bool poll_event(sys_event& event)
	{
		SDL_Event e;

		if (!SDL_PollEvent(&e))
			return false;

		switch (e.type)
		{
		case SDL_QUIT:
			event.type = sys_event_type::quit;
			break;
        case SDL_APP_TERMINATING:
        case SDL_APP_LOWMEMORY:
        case SDL_APP_WILLENTERBACKGROUND:
        case SDL_APP_DIDENTERBACKGROUND:
        case SDL_APP_WILLENTERFOREGROUND:
        case SDL_APP_DIDENTERFOREGROUND:
        case SDL_LOCALECHANGED:
        case SDL_DISPLAYEVENT:
        case SDL_WINDOWEVENT:
        case SDL_SYSWMEVENT:
        case SDL_KEYDOWN:
        case SDL_KEYUP:
        case SDL_TEXTEDITING:
        case SDL_TEXTINPUT:
        case SDL_KEYMAPCHANGED:
        case SDL_TEXTEDITING_EXT:
        case SDL_MOUSEMOTION:
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEWHEEL:
        case SDL_JOYAXISMOTION:
        case SDL_JOYBALLMOTION:
        case SDL_JOYHATMOTION:
        case SDL_JOYBUTTONDOWN:
        case SDL_JOYBUTTONUP:
        case SDL_JOYDEVICEADDED:
        case SDL_JOYDEVICEREMOVED:
        case SDL_JOYBATTERYUPDATED:
        case SDL_CONTROLLERAXISMOTION:
        case SDL_CONTROLLERBUTTONDOWN:
        case SDL_CONTROLLERBUTTONUP:
        case SDL_CONTROLLERDEVICEADDED:
        case SDL_CONTROLLERDEVICEREMOVED:
        case SDL_CONTROLLERDEVICEREMAPPED:
        case SDL_CONTROLLERTOUCHPADDOWN:
        case SDL_CONTROLLERTOUCHPADMOTION:
        case SDL_CONTROLLERTOUCHPADUP:
        case SDL_CONTROLLERSENSORUPDATE:
        case SDL_CONTROLLERUPDATECOMPLETE_RESERVED_FOR_SDL3:
        case SDL_CONTROLLERSTEAMHANDLEUPDATED:
        case SDL_FINGERDOWN:
        case SDL_FINGERUP:
        case SDL_FINGERMOTION:
        case SDL_DOLLARGESTURE:
        case SDL_DOLLARRECORD:
        case SDL_MULTIGESTURE:
        case SDL_CLIPBOARDUPDATE:
        case SDL_DROPFILE:
        case SDL_DROPTEXT:
        case SDL_DROPBEGIN:
        case SDL_DROPCOMPLETE:
        case SDL_AUDIODEVICEADDED:
        case SDL_AUDIODEVICEREMOVED:
        case SDL_SENSORUPDATE:
        case SDL_RENDER_TARGETS_RESET:
        case SDL_RENDER_DEVICE_RESET:
        case SDL_POLLSENTINEL:
        case SDL_USEREVENT:
		default:
            event.type = sys_event_type::none;
            break;
		}

		return true;
	}
}