#ifndef VAULT_H
#define VAULT_H

#include <string>
#include <filesystem>
#include <unordered_map>
#include "CommitNode.h"
#include "Tree.h"

namespace fs = std::filesystem;

class Vault {
private:
    std::string repoPath;
    std::string objectsPath;
    std::string commitsPath;
    std::string treesPath;
    std::string headPath;
    std::string indexPath;
    std::string stagedTreePath;
    CommitNode* headCommit;
    std::unordered_map<std::string, std::string> committedHashes;

    void loadCommitHistory();
    void writeTree(const Tree& tree, const std::string& path);
    void readTree(const std::string& hash, Tree& tree);
    void flattenTree(const Tree& tree, std::unordered_map<std::string, std::string>& files);
    void readIndex(const std::string& indexPath, std::unordered_map<std::string, std::string>& index);
public:
    Vault(const std::string& repoPath = ".");
    std::string sha1Hash(const std::string& content);
    ~Vault();

    void init();
    void add(const std::string& path);
    void commit(const std::string& message);
    void log();
    void status(const std::string& path = ".");
    void checkout(const std::string& commitHash);

    void addToTreeForStatus(Tree& tree, const fs::path& path, const std::string& objectsPath,
        std::unordered_map<std::string, std::string>& currentFiles, Vault& vault);
};

#endif // VAULT_H