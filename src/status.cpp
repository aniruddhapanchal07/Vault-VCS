#include "vault.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

void Vault::status(const std::string& path) {
    if (!fs::exists(path)) {
        std::cerr << "Error: Path does not exist: " << path << "\n";
        return;
    }

    std::unordered_map<std::string, std::string> currentIndex;
    readIndex(this->indexPath, currentIndex);

    std::unordered_map<std::string, std::string> committedFiles;
    std::ifstream headFile(this->headPath);
    if (headFile) {
        std::string headCommitHash;
        std::getline(headFile, headCommitHash);
        headFile.close();
        if (!headCommitHash.empty()) {
            std::string commitFilePath = this->commitsPath + "/" + headCommitHash;
            std::ifstream commitFile(commitFilePath);
            if (commitFile) {
                std::string line, treeHash;
                while (std::getline(commitFile, line)) {
                    if (line.find("Tree: ") == 0) {
                        treeHash = line.substr(6);
                        break;
                    }
                }
                commitFile.close();
                if (!treeHash.empty()) {
                    Tree commitTree;
                    readTree(treeHash, commitTree);
                    flattenTree(commitTree, committedFiles);
                }
            }
        }
    }

    std::unordered_map<std::string, std::string> currentFiles;
    Tree statusTree;
    addToTreeForStatus(statusTree, fs::path(path), this->objectsPath, currentFiles, *this);

    bool anyOutput = false;
    for (const auto& [filePath, fileHash] : currentFiles) {
        std::string normalizedPath = fs::canonical(filePath).string();
        auto itCommitted = committedFiles.find(normalizedPath);
        auto itCurrent = currentIndex.find(normalizedPath);

        if (itCurrent != currentIndex.end()) {
            if (itCommitted != committedFiles.end()) {
                if (itCurrent->second == fileHash) {
                    if (itCurrent->second == itCommitted->second) {
                        std::cout << "Committed: " << normalizedPath << "\n";
                        anyOutput = true;
                    } else {
                        std::cout << "Staged: " << normalizedPath << "\n";
                        anyOutput = true;
                    }
                } else {
                    std::cout << "Modified: " << normalizedPath << "\n";
                    anyOutput = true;
                }
            } else {
                std::cout << "Staged: " << normalizedPath << "\n";
                anyOutput = true;
            }
        } else if (itCommitted != committedFiles.end()) {
            if (fileHash == itCommitted->second) {
                std::cout << "Committed: " << normalizedPath << "\n";
                anyOutput = true;
            } else {
                std::cout << "Modified: " << normalizedPath << "\n";
                anyOutput = true;
            }
        } else {
            std::cout << "Untracked: " << normalizedPath << "\n";
            anyOutput = true;
        }
    }

    if (!anyOutput && !currentIndex.empty()) {
        std::cout << "No changes detected: all files are committed\n";
    } else if (!anyOutput && currentIndex.empty() && !committedFiles.empty()) {
        std::cout << "No tracked files: all files are untracked\n";
    }
}

void Vault::addToTreeForStatus(Tree& tree, const fs::path& path, const std::string& objectsPath,
                              std::unordered_map<std::string, std::string>& currentFiles, Vault& vault) {
    for (const auto& entry : fs::directory_iterator(path)) {
        // Skip the .vault directory
        if (entry.path().filename().string() == ".vault") {
            continue;
        }

        std::string canonicalPath = fs::canonical(entry.path()).string();
        if (fs::is_directory(entry.path())) {
            addToTreeForStatus(tree, entry.path(), objectsPath, currentFiles, vault);
        } else if (fs::is_regular_file(entry.path())) {
            std::ifstream file(entry.path(), std::ios::binary);
            if (!file) {
                continue;
            }
            std::string content((std::istreambuf_iterator<char>(file)),
                                std::istreambuf_iterator<char>());
            file.close();
            std::string fileHash = vault.sha1Hash(content);
            currentFiles[canonicalPath] = fileHash;
        }
    }
}