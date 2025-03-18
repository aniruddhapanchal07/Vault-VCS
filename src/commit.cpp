#include "Vault.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>

namespace fs = std::filesystem;

void Vault::commit(const std::string& message) {
    std::ifstream indexFile(this->indexPath);
    if (!indexFile) {
        std::cerr << "Error: Unable to open index file.\n";
        return;
    }

    std::string indexContent((std::istreambuf_iterator<char>(indexFile)), std::istreambuf_iterator<char>());
    indexFile.close();

    if (indexContent.empty()) {
        std::cout << "No changes staged for commit.\n";
        return;
    }

    std::ifstream stagedTreeFile(this->stagedTreePath);
    if (!stagedTreeFile) {
        std::cerr << "Error: Unable to open staged tree file.\n";
        return;
    }

    std::string treeHash;
    std::getline(stagedTreeFile, treeHash);
    stagedTreeFile.close();

    if (treeHash.empty()) {
        std::cerr << "Error: Staged tree hash is empty.\n";
        return;
    }

    std::string commitHash = sha1Hash(indexContent + message + treeHash);
    std::string parentHash = (headCommit != nullptr) ? headCommit->commitHash : "";

    std::string commitFilePath = this->commitsPath + "/" + commitHash;
    if (fs::exists(commitFilePath)) {
        std::cout << "Commit already exists: " << commitHash << "\n";
        return;
    }

    CommitNode* newCommit = new CommitNode(commitHash, message, parentHash, treeHash);
    newCommit->next = this->headCommit;
    this->headCommit = newCommit;

    std::ofstream commitFile(commitFilePath);
    if (!commitFile) {
        std::cerr << "Error: Unable to create commit file.\n";
        delete newCommit;
        return;
    }

    commitFile << "Commit: " << commitHash << "\n";
    commitFile << "Message: " << message << "\n";
    commitFile << "Parent: " << parentHash << "\n";
    commitFile << "Tree: " << treeHash << "\n";
    commitFile << "Timestamp: " << newCommit->timestamp << "\n";
    commitFile << indexContent;
    commitFile.close();

    std::ofstream headFile(this->headPath);
    if (!headFile) {
        std::cerr << "Error: Unable to update HEAD file.\n";
        return;
    }
    headFile << commitHash;
    headFile.close();

    std::ofstream clearIndexFile(this->indexPath, std::ios::trunc);
    if (!clearIndexFile) {
        std::cerr << "Error: Unable to clear index file.\n";
        return;
    }
    clearIndexFile.close();

    std::ofstream clearStagedTreeFile(this->stagedTreePath, std::ios::trunc);
    if (!clearStagedTreeFile) {
        std::cerr << "Error: Unable to clear staged tree file.\n";
        return;
    }
    clearStagedTreeFile.close();

    Tree tree;
    readTree(treeHash, tree);
    committedHashes.clear();
    flattenTree(tree, committedHashes);

    std::cout << "Commit created successfully: " << commitHash << "\n";
}