#include "vault.h"
#include "../deps/include/openssl/sha.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <iomanip>
#include <vector>
#include <numeric>

namespace fs = std::filesystem;

Vault::Vault(const std::string& repoPath) {
    this->repoPath = repoPath + "/.vault";
    this->objectsPath = this->repoPath + "/objects";
    this->commitsPath = this->repoPath + "/commits";
    this->treesPath = this->repoPath + "/trees";
    this->headPath = this->repoPath + "/HEAD";
    this->indexPath = this->repoPath + "/index";
    this->stagedTreePath = this->repoPath + "/staged_tree";
    this->headCommit = nullptr;
    loadCommitHistory();
}

Vault::~Vault() {
    while (headCommit != nullptr) {
        CommitNode* temp = headCommit;
        headCommit = headCommit->next;
        delete temp;
    }
}

std::string Vault::sha1Hash(const std::string& content) {
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(content.c_str()), content.size(), hash);

    std::stringstream ss;
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }

    return ss.str();
}

void Vault::flattenTree(const Tree& tree, std::unordered_map<std::string, std::string>& files) {
    for (const auto& [filePath, fileHash] : tree.files) {
        files[filePath] = fileHash;
    }
    for (const auto& [dirName, subtree] : tree.subtrees) {
        std::string prefix = dirName + "/";
        flattenTree(*subtree, files);
        std::unordered_map<std::string, std::string> updatedFiles;
        for (const auto& [filePath, fileHash] : files) {
            if (filePath.find(prefix) == 0) {
                std::string fullPath = dirName + filePath.substr(prefix.length());
                updatedFiles[fullPath] = fileHash;
            } else {
                updatedFiles[filePath] = fileHash;
            }
        }
        files = updatedFiles;
    }
}

void Vault::writeTree(const Tree& tree, const std::string& path) {
    std::stringstream treeContent;
    for (const auto& [filePath, fileHash] : tree.files) {
        treeContent << "file " << filePath << " " << fileHash << "\n";
    }
    for (const auto& [dirName, subtree] : tree.subtrees) {
        std::string subtreeHash = this->sha1Hash(dirName);
        treeContent << "tree " << dirName << " " << subtreeHash << "\n";

        std::string subtreePath = this->treesPath + "/" + subtreeHash;
        writeTree(*subtree, subtreePath);
    }

    std::ofstream treeFile(path);
    if (!treeFile) {
        return;
    }
    treeFile << treeContent.str();
    treeFile.close();
}

void Vault::readTree(const std::string& hash, Tree& tree) {
    std::string treeFilePath = this->treesPath + "/" + hash;
    if (!fs::exists(treeFilePath)) {
        return;
    }

    std::ifstream treeFile(treeFilePath);
    if (!treeFile) {
        return;
    }

    std::string line;
    while (std::getline(treeFile, line)) {
        std::istringstream iss(line);
        std::string type, pathOrName, hashStr;
        if (!(iss >> type >> pathOrName >> hashStr)) {
            continue;
        }

        if (type == "file") {
            tree.files[pathOrName] = hashStr;
        } else if (type == "tree") {
            Tree* subtree = new Tree();
            readTree(hashStr, *subtree);
            tree.subtrees[pathOrName] = subtree;
        }
    }
    treeFile.close();
}

void Vault::readIndex(const std::string& indexPath, std::unordered_map<std::string, std::string>& index) {
    index.clear();
    std::ifstream indexFile(indexPath);
    if (indexFile.is_open()) {
        std::string line;
        while (std::getline(indexFile, line)) {
            std::istringstream iss(line);
            std::string filePath, fileHash;
            if (iss >> filePath >> fileHash) {
                index[filePath] = fileHash;
            }
        }
        indexFile.close();
    }
}

void Vault::loadCommitHistory() {
    if (!fs::exists(headPath)) {
        return;
    }

    std::ifstream headFile(headPath);
    if (!headFile) {
        return;
    }

    std::string lastCommitHash;
    std::getline(headFile, lastCommitHash);
    headFile.close();

    if (lastCommitHash.empty()) {
        return;
    }

    std::vector<CommitNode*> commitNodes;
    std::string currentHash = lastCommitHash;

    while (!currentHash.empty()) {
        std::string commitFilePath = commitsPath + "/" + currentHash;
        if (!fs::exists(commitFilePath)) {
            break;
        }

        std::ifstream commitFile(commitFilePath);
        if (!commitFile) {
            break;
        }

        std::string line, commitHash, message, parentHash, treeHash, timestamp;
        while (std::getline(commitFile, line)) {
            if (line.find("Commit: ") == 0) {
                commitHash = line.substr(8);
            } else if (line.find("Message: ") == 0) {
                message = line.substr(9);
            } else if (line.find("Parent: ") == 0) {
                parentHash = line.substr(8);
            } else if (line.find("Tree: ") == 0) {
                treeHash = line.substr(6);
            } else if (line.find("Timestamp: ") == 0) {
                timestamp = line.substr(11);
                break;
            }
        }
        commitFile.close();

        CommitNode* newCommit = new CommitNode(commitHash, message, parentHash, treeHash);
        newCommit->timestamp = timestamp;
        commitNodes.push_back(newCommit);

        currentHash = parentHash;
    }

    while (headCommit != nullptr) {
        CommitNode* temp = headCommit;
        headCommit = headCommit->next;
        delete temp;
    }

    if (!commitNodes.empty()) {
        headCommit = commitNodes[0];
        for (size_t i = 1; i < commitNodes.size(); ++i) {
            commitNodes[i - 1]->next = commitNodes[i];
        }
        commitNodes.back()->next = nullptr;

        // Populate committedHashes with all files from the tree, including subtrees
        Tree tree;
        readTree(headCommit->treeHash, tree);
        committedHashes.clear();
        flattenTree(tree, committedHashes);
    }
}