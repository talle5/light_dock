#include "shell/UnityShell.h"

#include <iostream>

int main()
{
    if (auto shell = UnityShell::Create())
    {
        std::cout << "Dock inicializada. Iniciando UnityShell..." << std::endl;
        shell.value()->Run();
        return 0;
    }

    std::cerr << "Falha ao iniciar a dock pelo UnityShell." << std::endl;
    return 1;
}
