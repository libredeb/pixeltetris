#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <windows.h>
#endif

#include <cstring>
#include <iostream>

#include "game.hpp"

static void printUsage ()
{
    std::cerr << "Pixeltetris\n"
              << "  -w, --windowed     Force a 720x720 window\n"
              << "  -f, --fullscreen   Force fullscreen (used automatically on the 720x720 handheld)\n"
              << "  -h, --help         Show this help\n";
}

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#else
int main(int argc, char **argv)
#endif
{
    Game *game = Game::getInstance();

#if !defined(WIN32) && !defined(_WIN32) && !defined(__WIN32__) && !defined(__NT__)
    for (int i = 1; i < argc; i++)
    {
        if (std::strcmp(argv[i], "-w") == 0 || std::strcmp(argv[i], "--windowed") == 0)
        {
            game->setWindowed(true, true);
        }
        else if (std::strcmp(argv[i], "-f") == 0 || std::strcmp(argv[i], "--fullscreen") == 0)
        {
            game->setWindowed(false, true);
        }
        else if (std::strcmp(argv[i], "-h") == 0 || std::strcmp(argv[i], "--help") == 0)
        {
            printUsage();
            return 0;
        }
    }
#endif

    if (game->initialize())
    {
        while (!game->isGameExiting())
        {
            game->run();
        }
        game->exit();
    }
    return 0;
}
