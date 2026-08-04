
#include <cstdlib>
#include <iostream>
#include "../common/formats/twad/twad.h"
#include <list>
#include <string>

using namespace tungsten;


std::string twad_filename{};

std::list<std::string> texture_paths{};
std::list<std::string> sound_paths{};
std::list<std::string> model_paths{};

void TWAD_Init()
{
}


int main(int argc, char* argv[])
{

    for (int i = 0; i < argc; i++)
    {
        std::cout << "argv " << i << ' ' << argv[i] << std::endl;
    }

    return EXIT_SUCCESS;
}