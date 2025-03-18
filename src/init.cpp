#include "Vault.h"
#include <iostream>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

void Vault::init() {
    // Create the repository structure
    if (!fs::exists(this->repoPath)) {
        fs::create_directories(this->repoPath);
        fs::create_directories(this->objectsPath);
        fs::create_directories(this->commitsPath);
        fs::create_directories(this->treesPath);
        
        std::ofstream headFile(this->headPath);
        if (!headFile) {
            std::cerr << "Error: Unable to create HEAD file.\n";
            return;
        }
        headFile.close();
        
        std::ofstream indexFile(this->indexPath);
        if (!indexFile) {
            std::cerr << "Error: Unable to create index file.\n";
            return;
        }
        indexFile.close();
        
        std::ofstream stagedTreeFile(this->stagedTreePath);
        if (!stagedTreeFile) {
            std::cerr << "Error: Unable to create staged tree file.\n";
            return;
        }
        stagedTreeFile.close();
        
        std::cout << "Initialized empty Vault repository in " << this->repoPath << "\n";
    } else {
        std::cout << "Vault repository already exists in " << this->repoPath << "\n";
    }
}