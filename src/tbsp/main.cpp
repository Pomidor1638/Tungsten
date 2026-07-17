#include <iostream>

int main(int argc, char** argv)
{
    std::cout << "tbspcompiler started\n";

    if (argc < 2)
    {
        std::cout << "usage: tbspcompiler <input.tmap> <output name>\n";
        return 1;
    }

    std::cout << "input map: " << argv[1] << "\n";
    return 0;
}
