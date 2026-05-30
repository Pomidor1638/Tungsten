#include "input.h"
#include <iostream>
#include <cstring>
namespace tungsten::input
{
    Input::GamePad::GamePad(int index)
    {
        handler = SDL_GameControllerOpen(index);
        memset(cur_btns, 0, sizeof(cur_btns));
        memset(last_btns, 0, sizeof(last_btns));
        memset(axes, 0, sizeof(axes));
    }

    Input::GamePad::~GamePad()
    {
        if (handler)
        {
            SDL_GameControllerClose(handler);
        }
    }


    bool Input::init()
    {
        last_keys.fill(0);

        gamepads.resize(4);

        for (int i = 0; i < SDL_NumJoysticks(); i++)
        {
            if (SDL_IsGameController(i))
            {
                gamepads[i] = std::make_unique<GamePad>(i);
            }
        }

        return true;
    }

    Input::Input(SDL_Window* w)
        : window(w)
    {

        
    }

    Input::~Input()
    {}


    void Input::processEvent(const SDL_Event& e)
    {
        processFocusDepended(e);
        processFocusIndepended(e);
    }

    bool Input::isKeyPressed(const SDL_Keycode& key) const
    {
        return cur_keys[key];
    }

    bool Input::isKeyJustPressed(const SDL_Keycode& key) const
    {
        return !last_keys[key] && cur_keys[key];
    }

    bool Input::isKeyJustReleased(const SDL_Keycode& key) const
    {
        return last_keys[key] && !cur_keys[key];
    }

    bool Input::isGamePadButtonPressed(int gamepad_index, GamePadButton button) const
    {
        if (gamepad_index < 0 || gamepad_index >= (int)gamepads.size() || !gamepads[gamepad_index])
            return false;

        auto btn = static_cast<SDL_GameControllerButton>(button);
        return gamepads[gamepad_index]->cur_btns[btn];
    }

    bool Input::isGamePadButtonJustPressed(int gamepad_index, GamePadButton button) const
    {
        if (gamepad_index < 0 || gamepad_index >= (int)gamepads.size() || !gamepads[gamepad_index])
            return false;

        auto& gamepad = gamepads[gamepad_index];
        auto btn = static_cast<SDL_GameControllerButton>(button);
        return !gamepad->last_btns[btn] && gamepad->cur_btns[btn];
    }

    bool Input::isGamePadButtonJustReleased(int gamepad_index, GamePadButton button) const
    {
        if (gamepad_index < 0 || gamepad_index >= (int)gamepads.size() || !gamepads[gamepad_index])
            return false;

        auto& gamepad = gamepads[gamepad_index];
        auto btn = static_cast<SDL_GameControllerButton>(button);
        return gamepad->last_btns[btn] && !gamepad->cur_btns[btn];
    }

    float Input::getGamePadAxisMotion(int gamepad_index, GamePadAxis axis) const
    {
        if (gamepad_index < 0 || gamepad_index >= (int)gamepads.size() || !gamepads[gamepad_index])
            return 0.0f;

        auto& gamepad = gamepads[gamepad_index];
        return gamepad->axes[(SDL_GameControllerAxis)axis] / 32768.0f;
    }

    int Input::getGamePadAxisMotionRaw(int gamepad_index, GamePadAxis axis) const
    {
        if (gamepad_index < 0 || gamepad_index >= (int)gamepads.size() || !gamepads[gamepad_index])
            return 0;

        auto& gamepad = gamepads[gamepad_index];
        return gamepad->axes[(SDL_GameControllerAxis)axis];
    }


    void Input::getGamePadLeftAxisState(int gamepad_index, float& x, float& y) const
    {
        x = getGamePadAxisMotion(gamepad_index, GamePadAxis::LeftX);
        y = getGamePadAxisMotion(gamepad_index, GamePadAxis::LeftY);
    }
    void Input::getGamePadRightAxisState(int gamepad_index, float& x, float& y) const
    {
        x = getGamePadAxisMotion(gamepad_index, GamePadAxis::RightX);
        y = getGamePadAxisMotion(gamepad_index, GamePadAxis::RightY);
    }


    void Input::getGamePadLeftAxisStateRaw(int gamepad_index, int& x, int& y) const
    {
        x = getGamePadAxisMotionRaw(gamepad_index, GamePadAxis::LeftX);
        y = getGamePadAxisMotionRaw(gamepad_index, GamePadAxis::LeftY);
    }
    void Input::getGamePadRightAxisStateRaw(int gamepad_index, int& x, int& y) const
    {
        x = getGamePadAxisMotionRaw(gamepad_index, GamePadAxis::RightX);
        y = getGamePadAxisMotionRaw(gamepad_index, GamePadAxis::RightY);
    }

    void Input::applyRadialDeadZone(float& x, float& y, float deadzone_radius)
    {
        double length_sq = x * x + y * y;
        double length = sqrt(length_sq);

        if (length_sq > 1.0f)
        {
            x /= length;
            y /= length;
        }

        if (length < deadzone_radius)
        {
            x = y = 0.0f;
            return;
        }


        x /= length;
        y /= length;

        float scale = (length - deadzone_radius) / (1.0f - deadzone_radius);
        x *= scale;
        y *= scale;
    }


    bool Input::isGamePadConnected(int gamepad_index) const
    {
        return gamepad_index >= 0 && gamepad_index < (int)gamepads.size() && gamepads[gamepad_index] && gamepads[gamepad_index]->handler;
    }

    int Input::getConnectedGamePadCount() const
    {
        int count = 0;
        for (const auto& gamepad : gamepads)
        {
            if (gamepad && gamepad->handler)
                count++;
        }
        return count;
    }

    void Input::setMousePos(int x, int y)
    {
        SDL_WarpMouseInWindow(window, x, y);
    }

    void Input::getMousePos(int& x, int& y) const
    {
        x = mouse_x;
        y = mouse_y;
    }

    Uint32 Input::getMousePressedButtons() const
    {
        return cur_mouse_buttons;
    }

    Uint32 Input::getMouseJustPressedButtons() const
    {
        return ~last_mouse_buttons & cur_mouse_buttons;
    }

    Uint32 Input::getMouseJustReleasedButtons() const
    {
        return last_mouse_buttons & ~cur_mouse_buttons;
    }

    float Input::getMouseWheelDelta() const
    {
        return static_cast<float>(mouse_wheel_y);
    }

    int Input::getMouseWheelDeltaY() const
    {
        return mouse_wheel_y;
    }

    int Input::getMouseWheelDeltaX() const
    {
        return mouse_wheel_x;
    }

    void Input::showCursor(bool show)
    {
        SDL_ShowCursor(show ? SDL_ENABLE : SDL_DISABLE);
    }

    void Input::setCursorRelativeMode(bool relative)
    {
        SDL_SetRelativeMouseMode(relative ? SDL_TRUE : SDL_FALSE);
    }

    void Input::processFocusDepended(const SDL_Event& e)
    {
        switch (e.type)
        {
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            processKeyboardEvent(e);
            break;

        case SDL_MOUSEMOTION:
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEWHEEL:
            processMouseEvent(e);
            break;

        case SDL_CONTROLLERAXISMOTION:
        case SDL_CONTROLLERBUTTONDOWN:
        case SDL_CONTROLLERBUTTONUP:
            processGamePadEvent(e);
            break;

        default:
            break;
        }
    }

    void Input::processFocusIndepended(const SDL_Event& e)
    {
        switch (e.type)
        {
        case SDL_CONTROLLERDEVICEADDED:
            handleDeviceAdded(e.cdevice.which);
            break;
        case SDL_CONTROLLERDEVICEREMOVED:
            handleDeviceRemoved(e.cdevice.which);
            break;
        default:
            break;
        }
    }

    void Input::processKeyboardEvent(const SDL_Event& e)
    {
        /* switch (e.type)
         {
         case SDL_KEYDOWN:
             cur_keys[e.key.keysym.scancode] = true;
             break;
         case SDL_KEYUP:
             cur_keys[e.key.keysym.scancode] = false;
             break;
         default:
             break;
         }*/
    }

    void Input::processMouseEvent(const SDL_Event& e)
    {
        switch (e.type)
        {
            /* case SDL_MOUSEMOTION:
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                break;*/

        case SDL_MOUSEWHEEL:
            mouse_wheel_x = e.wheel.x;
            mouse_wheel_y = e.wheel.y;
            break;

        default:
            break;
        }
    }

    void Input::processGamePadEvent(const SDL_Event& e)
    {
        int device_index = -1;

        switch (e.type)
        {
        case SDL_CONTROLLERBUTTONDOWN:
        case SDL_CONTROLLERBUTTONUP:
            device_index = e.cbutton.which;
            if (device_index >= 0 && device_index < (int)gamepads.size() && gamepads[device_index])
            {
                gamepads[device_index]->cur_btns[e.cbutton.button] = (e.type == SDL_CONTROLLERBUTTONDOWN);
            }
            break;

        case SDL_CONTROLLERAXISMOTION:
            device_index = e.caxis.which;
            if (device_index >= 0 && device_index < (int)gamepads.size() && gamepads[device_index])
            {
                Sint16 value = e.caxis.value;
                gamepads[device_index]->axes[e.caxis.axis] = value;
            }
            break;

        default:
            break;
        }
    }

    void Input::handleDeviceAdded(int device_index)
    {
        if (device_index < 0)
            return;

        if (device_index >= (int)gamepads.size())
        {
            return;
        }

        if (SDL_IsGameController(device_index))
        {
            if (!gamepads[device_index])
            {
                gamepads[device_index] = std::make_unique<GamePad>(device_index);
                if (gamepads[device_index]->handler)
                {
                    std::cout << "Gamepad connected at slot " << device_index << std::endl;
                }
            }
        }
    }

    void Input::handleDeviceRemoved(int device_index)
    {
        if (device_index < 0 || device_index >= (int)gamepads.size())
            return;

        if (gamepads[device_index])
        {
            std::cout << "Gamepad disconnected from slot " << device_index << std::endl;
            gamepads[device_index].reset();
        }
    }

    void Input::startProcessEvent()
    {
        last_mouse_buttons = cur_mouse_buttons;
        memcpy(last_keys.data(), cur_keys, SDL_NUM_SCANCODES);

        mouse_wheel_x = 0;
        mouse_wheel_y = 0;

        for (auto& gamepad : gamepads)
        {
            if (gamepad)
            {
                memcpy(gamepad->last_btns, gamepad->cur_btns, sizeof(gamepad->last_btns));
            }
        }
    }


    void Input::endProcessEvent()
    {
        cur_mouse_buttons = SDL_GetMouseState(&mouse_x, &mouse_y);
        for (size_t i = 0; i < gamepads.size(); i++)
        {
            if (gamepads[i] && gamepads[i]->handler)
            {
                if (!SDL_GameControllerGetAttached(gamepads[i]->handler))
                {
                    gamepads[i].reset();
                    std::cout << "Gamepad " << i << " disconnected (without event)" << std::endl;
                }
            }
        }
    }

    int Input::findFreeGamePadSlot() const
    {
        for (size_t i = 0; i < gamepads.size(); i++)
        {
            if (!gamepads[i])
                return static_cast<int>(i);
        }
        return -1;
    }

}
