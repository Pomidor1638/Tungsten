#pragma once

#include <SDL2/SDL.h>
#include <array>
#include <memory>
#include <vector>

namespace tungsten::input
{

    class Input
    {
    public:

        bool init();

        // main event process
        void processEvent(const SDL_Event& e);

        void startProcessEvent();
        void endProcessEvent();

        // keyboard
        bool isKeyPressed(const SDL_Keycode& key) const;
        bool isKeyJustPressed(const SDL_Keycode& key) const;
        bool isKeyJustReleased(const SDL_Keycode& key) const;

        // gamepad
        enum class GamePadButton
        {
            A             = SDL_CONTROLLER_BUTTON_A,
            B             = SDL_CONTROLLER_BUTTON_B,
            X             = SDL_CONTROLLER_BUTTON_X,
            Y             = SDL_CONTROLLER_BUTTON_Y,
            Back          = SDL_CONTROLLER_BUTTON_BACK,
            Guide         = SDL_CONTROLLER_BUTTON_GUIDE,
            Start         = SDL_CONTROLLER_BUTTON_START,
            LeftStick     = SDL_CONTROLLER_BUTTON_LEFTSTICK,
            RightStick    = SDL_CONTROLLER_BUTTON_RIGHTSTICK,
            LeftShoulder  = SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
            RightShoulder = SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
            DPadUp        = SDL_CONTROLLER_BUTTON_DPAD_UP,
            DPadDown      = SDL_CONTROLLER_BUTTON_DPAD_DOWN,
            DPadLeft      = SDL_CONTROLLER_BUTTON_DPAD_LEFT,
            DPadRight     = SDL_CONTROLLER_BUTTON_DPAD_RIGHT
        };

        enum class GamePadAxis
        {
            LeftX        = SDL_CONTROLLER_AXIS_LEFTX,
            LeftY        = SDL_CONTROLLER_AXIS_LEFTY,
            RightX       = SDL_CONTROLLER_AXIS_RIGHTX,
            RightY       = SDL_CONTROLLER_AXIS_RIGHTY,
            TriggerLeft  = SDL_CONTROLLER_AXIS_TRIGGERLEFT,
            TriggerRight = SDL_CONTROLLER_AXIS_TRIGGERRIGHT
        };

        bool isGamePadButtonPressed(int gamepad_index, GamePadButton button) const;
        bool isGamePadButtonJustPressed(int gamepad_index, GamePadButton button) const;
        bool isGamePadButtonJustReleased(int gamepad_index, GamePadButton button) const;

        float getGamePadAxisMotion(int gamepad_index, GamePadAxis axis) const;
        int getGamePadAxisMotionRaw(int gamepad_index, GamePadAxis axis) const;

        void getGamePadLeftAxisState(int gamepad_index, float& x, float& y) const;
        void getGamePadRightAxisState(int gamepad_index, float& x, float& y) const;

        void getGamePadLeftAxisStateRaw(int gamepad_index, int& x, int& y) const;
        void getGamePadRightAxisStateRaw(int gamepad_index, int& x, int& y) const;

        static void applyRadialDeadZone(float& x, float& y, float deadzone_radius = 0.1f);

        bool isGamePadConnected(int gamepad_index) const;
        int getConnectedGamePadCount() const;

        // mouse
        void setMousePos(int x, int y);
        void getMousePos(int& x, int& y) const;

        Uint32 getMousePressedButtons() const;
        Uint32 getMouseJustPressedButtons() const;
        Uint32 getMouseJustReleasedButtons() const;

        float getMouseWheelDelta() const;
        int getMouseWheelDeltaY() const;
        int getMouseWheelDeltaX() const;

        void showCursor(bool show);
        void setCursorRelativeMode(bool relative);

        Input(SDL_Window* w);
        virtual ~Input();
        Input(const Input&) = delete;
        Input(Input&&) = delete;

    private:

        SDL_Window* window = nullptr;
        std::array<uint8_t, SDL_NUM_SCANCODES> last_keys;
        const uint8_t* cur_keys = SDL_GetKeyboardState(nullptr);

        // mouse
        Uint32 last_mouse_buttons = 0;
        Uint32 cur_mouse_buttons  = 0;
        int    mouse_x            = 0;
        int    mouse_y            = 0;
        int    mouse_wheel_x      = 0;
        int    mouse_wheel_y      = 0;

        // gamepads
        struct GamePad
        {
            SDL_GameController* handler = nullptr;
            bool cur_btns[SDL_CONTROLLER_BUTTON_MAX]{};
            bool last_btns[SDL_CONTROLLER_BUTTON_MAX]{};
            Sint16 axes[SDL_CONTROLLER_AXIS_MAX]{};

            GamePad(int index);
            virtual ~GamePad();

            GamePad(const GamePad&) = delete;
            GamePad(GamePad&&) = delete;
        };
        
        std::vector<std::unique_ptr<GamePad>> gamepads;

        // process funcs 
        void processFocusDepended(const SDL_Event& e);
        void processFocusIndepended(const SDL_Event& e);

        void processMouseEvent(const SDL_Event& e);
        void processKeyboardEvent(const SDL_Event& e);
        void processGamePadEvent(const SDL_Event& e);

        void handleDeviceAdded(int device_index);
        void handleDeviceRemoved(int device_index);

        int findFreeGamePadSlot() const;
    };

}
