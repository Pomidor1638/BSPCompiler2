#include "engine.h"


int main(int argc, char** argv) 
{

    using namespace Engine;

    Init(argc, argv);

    while (running)
    {
        Frame();
    }
    
    Quit();
    
    return EXIT_SUCCESS;
}