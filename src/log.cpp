#include "Vault.h"
#include <iostream>
#include <fstream>

void Vault::log() {
    std::cout << "Commit History:\n";
    std::cout << "--------------------------\n";
    
    for (const auto& entry : fs::directory_iterator(this->commitsPath)) {
        std::string commitHash = entry.path().filename().string();
        std::string commitFilePath = this->commitsPath + "/" + commitHash;
        
        if (fs::exists(commitFilePath)) {
            std::ifstream commitFile(commitFilePath);
            if (commitFile.is_open()) {
                std::string line;
                std::string message, parent, tree, timestamp;
                while (std::getline(commitFile, line)) {
                    if (line.find("Message: ") == 0) message = line.substr(9);
                    else if (line.find("Parent: ") == 0) parent = line.substr(8);
                    else if (line.find("Tree: ") == 0) tree = line.substr(6);
                    else if (line.find("Timestamp: ") == 0) timestamp = line.substr(11);
                }
                commitFile.close();
                
                std::cout << "Commit: " << commitHash << "\n";
                std::cout << "Message: " << message << "\n";
                std::cout << "Parent: " << (parent.empty() ? "" : parent) << "\n";
                std::cout << "Tree: " << tree << "\n";
                std::cout << "Timestamp: " << timestamp << "\n";
                std::cout << "--------------------------\n";
            }
        }
    }
}