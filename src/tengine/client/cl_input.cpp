

#include "client.h"
#include "protocol/protocol.h"
#include "server.h"
#include "state/state.h"
#include "utils/system/system.h"
#include <SDL_scancode.h>
#include <cstring>

namespace tungsten::client 
{

    
    void Client::update_angles(glm::vec3& angles)
    {
        int x, y;
        int hw = 0, hh = 0;

        input->getMousePos(x, y);
        window->getSize(hw, hh);

        hw >>= 1;
        hh >>= 1;

        input->setMousePos(hw, hh);

        x -= hw;
        y -= hh;

        const float sensitivity = 0.15f;

        angles.z -= glm::radians(x * sensitivity);
        angles.x -= glm::radians(y * sensitivity);

        if (angles.x > M_PI)
        {
            angles.x = M_PI;
        }
        if (angles.x < 0.0f)
        {
            angles.x = 0.0f;
        }
        if (angles.z > 2 * M_PI)
        {
            angles.z = fmodf(angles.z, 2 * M_PI);
        }
        if (angles.z < 0.0f)
        {
            angles.z = fmodf(angles.z, 2 * M_PI) + 2 * M_PI;
        }
    }
    
    glm::vec2 Client::make_wishdir()
    {
        return glm::vec2 
        {
            input->isKeyPressed(SDL_SCANCODE_D) - input->isKeyPressed(SDL_SCANCODE_A),
            input->isKeyPressed(SDL_SCANCODE_W) - input->isKeyPressed(SDL_SCANCODE_S)
        };
    }

    
    void Client::process_input_main_menu()
    {
        using enum client::UICommand;
        switch (client_state.ui_cmd) 
        {
        case singleplayer:
        {
            auto path = choose_file();
            if (!path.empty())
                start_single(path);

            break;
        }
        case multiplayer:
            break;
        default:
            break;
        }
    }

    void Client::process_input_in_game()
    {

        switch (client_state.ui_cmd)
        {
        case UICommand::disconnect:
            disconnect();
            break;
        default:
            break;
        }

        if (input->isKeyJustPressed(SDL_SCANCODE_ESCAPE))
        {
            client_state.menu = !client_state.menu;
        }
    }

    void Client::process_input()
    {
        using enum MainStage;
        switch (client_state.main_stage) 
        {
        //case boot:
        //    break;
        case main_menu:
            process_input_main_menu();
            break;
        case in_game:
            process_input_in_game();
            break;
        //case shutdown:
        //    break;
        default:
            break;
        }
    }



}
