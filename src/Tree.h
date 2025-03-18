#ifndef TREE_H
#define TREE_H

#include <unordered_map>
#include <string>

struct Tree {
    std::unordered_map<std::string, std::string> files;

    std::unordered_map<std::string, Tree*> subtrees;

    ~Tree() {
        for (auto& [dirName, subtree] : subtrees) {
            delete subtree;
        }
    }
};

#endif // TREE_H