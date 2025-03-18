#include "vault.h"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: vault <command> [args]\n";
        std::cerr << "Available commands: init, add, commit, log\n";
        return 1;
    }

    Vault vault;
    std::string command = argv[1];

    if (command == "init") {
        vault.init();
    } else if (command == "add") {
        if (argc < 3) {
            std::cerr << "Usage: vault add <file/directory>\n";
            return 1;
        }
        vault.add(argv[2]);
    } else if (command == "commit") {
        if (argc < 3) {
            std::cerr << "Usage: vault commit <message>\n";
            return 1;
        }
        vault.commit(argv[2]);
    } else if (command == "log") {
        vault.log();
    } else {
        std::cerr << "Unknown command: " << command << "\n";
        std::cerr << "Available commands: init, add, commit, log\n";
        return 1;
    }

    return 0;
}