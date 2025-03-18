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
        return;
    }

    std::unordered_map<std::string, std::string> currentIndex;
    readIndex(this->indexPath, currentIndex);

    std::unordered_set<std::string> stagedFiles;
    Tree rootTree;
    addToTree(rootTree, path, this->objectsPath, currentIndex, stagedFiles, *this);

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

    for (const auto& [filePath, fileHash] : currentIndex) {
        if (rootTree.files.find(filePath) == rootTree.files.end()) {
            rootTree.files[filePath] = fileHash;
        }
    }

    // Fully recursive tree persistence
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
        return;
    }
    stagedTreeFile << treeHash;
    stagedTreeFile.close();

    std::ofstream indexFile(this->indexPath, std::ios::trunc);
    if (!indexFile) {
        return;
    }
    for (const auto& [filePath, fileHash] : currentIndex) {
        indexFile << filePath << " " << fileHash << "\n";
    }
    indexFile.close();

    if (stagedFiles.empty()) {
        if (committedFiles.empty() || currentIndex == committedFiles) {
            std::cout << "already committed\n";
        } else {
            std::cout << "already added for commit\n";
        }
    } else {
        std::cout << "added for commit\n";
        for (const auto& file : stagedFiles) {
            std::cout << "  " << file << "\n";
        }
    }
}
