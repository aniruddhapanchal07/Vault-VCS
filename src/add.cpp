#include "vault.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <functional>

namespace fs = std::filesystem;

void addToTree(Tree& tree, const fs::path& path, const std::string& objectsPath,
               std::unordered_map<std::string, std::string>& currentIndex,
               std::unordered_set<std::string>& stagedFiles, Vault& vault)
{
    for (const auto& entry : fs::directory_iterator(path))
    {
        // Skip the .vault directory
        if (entry.path().filename().string() == ".vault") {
            continue;
        }

        std::string canonicalPath = fs::canonical(entry.path()).string();
        if (fs::is_directory(entry.path()))
        {
            Tree* subtree = new Tree();
            addToTree(*subtree, entry.path(), objectsPath, currentIndex, stagedFiles, vault);

            std::stringstream subtreeContent;
            for (const auto& [subFilePath, subFileHash] : subtree->files)
            {
                subtreeContent << "file " << subFilePath << " " << subFileHash << "\n";
            }
            for (const auto& [subDirName, subSubtree] : subtree->subtrees)
            {
                std::string subSubtreeHash = vault.sha1Hash(subDirName);
                subtreeContent << "tree " << subDirName << " " << subSubtreeHash << "\n";
            }
            std::string subtreeHash = vault.sha1Hash(subtreeContent.str());

            tree.subtrees[entry.path().filename().string()] = subtree;
        }
        else if (fs::is_regular_file(entry.path()))
        {
            std::ifstream file(entry.path(), std::ios::binary);
            if (!file)
            {
                continue;
            }
            std::string content((std::istreambuf_iterator<char>(file)),
                                std::istreambuf_iterator<char>());
            file.close();
            std::string fileHash = vault.sha1Hash(content);

            auto it = currentIndex.find(canonicalPath);
            if (it != currentIndex.end())
            {
                tree.files[canonicalPath] = it->second;
                if (it->second != fileHash)
                {
                    tree.files[canonicalPath] = fileHash;
                    stagedFiles.insert(canonicalPath);
                    currentIndex[canonicalPath] = fileHash;

                    std::string objectFilePath = objectsPath + "/" + fileHash;
                    if (!fs::exists(objectFilePath))
                    {
                        std::ofstream objectFile(objectFilePath, std::ios::binary);
                        if (objectFile)
                        {
                            objectFile << content;
                            objectFile.close();
                        }
                    }
                }
            }
            else
            {
                tree.files[canonicalPath] = fileHash;
                stagedFiles.insert(canonicalPath);
                currentIndex[canonicalPath] = fileHash;

                std::string objectFilePath = objectsPath + "/" + fileHash;
                if (!fs::exists(objectFilePath))
                {
                    std::ofstream objectFile(objectFilePath, std::ios::binary);
                    if (objectFile)
                    {
                        objectFile << content;
                        objectFile.close();
                    }
                }
            }
        }
    }
}

void Vault::add(const std::string& path) {
    if (!fs::exists(path)) {
        std::cerr << "Error: Path does not exist: " << path << "\n";
        return;
    }

    std::unordered_map<std::string, std::string> currentIndex;
    readIndex(this->indexPath, currentIndex);

    std::unordered_map<std::string, std::string> newFiles;
    std::unordered_set<std::string> stagedFiles;
    Tree rootTree;
    for (const auto& entry : fs::directory_iterator(path)) {
        if (entry.path().filename().string() == ".vault") {
            continue;
        }

        std::string canonicalPath = fs::canonical(entry.path()).string();
        if (fs::is_directory(entry.path())) {
            Tree* subtree = new Tree();
            addToTree(*subtree, entry.path(), this->objectsPath, currentIndex, stagedFiles, *this);
            rootTree.subtrees[entry.path().filename().string()] = subtree;
        } else if (fs::is_regular_file(entry.path())) {
            std::ifstream file(entry.path(), std::ios::binary);
            if (!file) {
                continue;
            }
            std::string content((std::istreambuf_iterator<char>(file)),
                                std::istreambuf_iterator<char>());
            file.close();
            std::string fileHash = this->sha1Hash(content);
            newFiles[canonicalPath] = fileHash;

            // Check if file needs to be staged
            auto it = currentIndex.find(canonicalPath);
            if (it == currentIndex.end() || it->second != fileHash) {
                stagedFiles.insert(canonicalPath);
                rootTree.files[canonicalPath] = fileHash;

                std::string objectFilePath = this->objectsPath + "/" + fileHash;
                if (!fs::exists(objectFilePath)) {
                    std::ofstream objectFile(objectFilePath, std::ios::binary);
                    if (objectFile) {
                        objectFile << content;
                        objectFile.close();
                    }
                }
            } else {
                rootTree.files[canonicalPath] = fileHash;
            }
        }
    }

    // Check if all files are already staged
    bool allFilesAlreadyStaged = true;
    for (const auto& [filePath, fileHash] : newFiles) {
        auto it = currentIndex.find(filePath);
        if (it == currentIndex.end() || it->second != fileHash) {
            allFilesAlreadyStaged = false;
            break;
        }
    }

    if (allFilesAlreadyStaged && !currentIndex.empty() && !newFiles.empty()) {
        std::cout << "already added for commit\n";
        return;
    }

    // Check if files are already committed (only if index is empty)
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

    bool allFilesCommitted = true;
    for (const auto& [filePath, fileHash] : newFiles) {
        auto it = committedFiles.find(filePath);
        if (it == committedFiles.end() || it->second != fileHash) {
            allFilesCommitted = false;
            break;
        }
    }

    if (allFilesCommitted && currentIndex.empty() && !newFiles.empty()) {
        std::cout << "already committed\n";
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
        return;
    }

    // Update index with new or modified files
    for (const auto& [filePath, fileHash] : newFiles) {
        currentIndex[filePath] = fileHash;
    }

    // Build tree content
    std::function<void(Tree&, std::stringstream&)> buildTreeContent = [&](Tree& tree, std::stringstream& content) {
        for (const auto& [filePath, fileHash] : tree.files) {
            content << "file " << filePath << " " << fileHash << "\n";
        }
        for (const auto& [dirName, subtree] : tree.subtrees) {
            std::stringstream subtreeContent;
            buildTreeContent(*subtree, subtreeContent);
            std::string subtreeHash = this->sha1Hash(subtreeContent.str());
            std::string subtreePath = this->treesPath + "/" + subtreeHash;
            if (!fs::exists(subtreePath)) {
                std::ofstream subtreeFile(subtreePath);
                if (subtreeFile) {
                    subtreeFile << subtreeContent.str();
                    subtreeFile.close();
                }
            }
            content << "tree " << dirName << " " << subtreeHash << "\n";
        }
    };

    std::stringstream rootContent;
    buildTreeContent(rootTree, rootContent);
    std::string treeHash = this->sha1Hash(rootContent.str());
    std::string treePath = this->treesPath + "/" + treeHash;
    if (!fs::exists(treePath)) {
        std::ofstream treeFile(treePath);
        if (treeFile) {
            treeFile << rootContent.str();
            treeFile.close();
        }
    }

    std::ofstream stagedTreeFile(this->stagedTreePath);
    if (!stagedTreeFile) {
        std::cerr << "Error: Unable to write staged tree file.\n";
        return;
    }
    stagedTreeFile << treeHash;
    stagedTreeFile.close();

    std::ofstream indexFile(this->indexPath, std::ios::trunc);
    if (!indexFile) {
        std::cerr << "Error: Unable to write index file.\n";
        return;
    }
    for (const auto& [filePath, fileHash] : currentIndex) {
        indexFile << filePath << " " << fileHash << "\n";
    }
    indexFile.close();

    if (stagedFiles.empty() && !newFiles.empty()) {
        std::cout << "already added for commit\n";
    } else {
        std::cout << "added for commit\n";
        for (const auto& file : stagedFiles) {
            std::cout << "  " << file << "\n";
        }
    }
}