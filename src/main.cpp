#include "engine.h"

#include <iostream>

int main(void)
{
    Gluon::Engine::Engine engine;

    while (!engine.EngineShouldQuit())
    {
        std::string command;
        std::getline(std::cin, command);

        engine.HandleUCICommand(command);
    }

    return 0;
}