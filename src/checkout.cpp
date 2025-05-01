#include "vault.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <functional>

namespace fs = std::filesystem;

void Vault::checkout(const std::string& commitHash) {
    std::string commitFilePath = this->commitsPath + "/" + commitHash;
    if (!fs::exists(commitFilePath)) {
        return;
    }

    std::ifstream commitFile(commitFilePath);
    std::string line, treeHash;
    while (std::getline(commitFile, line)) {
        if (line.find("Tree: ") == 0) {
            treeHash = line.substr(6);
            break;
        }
    }
    commitFile.close();

    if (treeHash.empty()) {
        return;
    }

    Tree tree;
    readTree(treeHash, tree);

    std::unordered_map<std::string, std::string> allFiles;
    flattenTree(tree, allFiles);

    for (const auto& [filePath, fileHash] : allFiles) {
        // Skip files in the .vault directory
        if (filePath.find("/.vault/") != std::string::npos || filePath.find("\\.vault\\") != std::string::npos) {
            continue;
        }

        std::string objectFilePath = this->objectsPath + "/" + fileHash;
        if (!fs::exists(objectFilePath)) {
            continue;
        }

        std::ifstream objectFile(objectFilePath, std::ios::binary);
        if (!objectFile) {
            continue;
        }

        std::string content((std::istreambuf_iterator<char>(objectFile)),
                            std::istreambuf_iterator<char>());
        objectFile.close();

        fs::create_directories(fs::path(filePath).parent_path());
        if (fs::exists(filePath)) {
            fs::remove(filePath);
        }

        std::ofstream outFile(filePath, std::ios::binary);
        if (!outFile) {
            continue;
        }
        outFile << content;
        outFile.close();
    }

    std::ofstream headFile(this->headPath);
    if (!headFile) {
        return;
    }
    headFile << commitHash;
    headFile.close();

    committedHashes.clear();
    flattenTree(tree, committedHashes);

    std::cout << "Checked out commit: " << commitHash << "\n";
}