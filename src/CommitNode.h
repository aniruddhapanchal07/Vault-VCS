#ifndef COMMITNODE_H
#define COMMITNODE_H

#include <string>

struct CommitNode {
    std::string commitHash;
    std::string message;
    std::string parentHash;
    std::string treeHash;
    std::string timestamp;

    CommitNode* next;

    CommitNode(const std::string& hash, const std::string& msg, const std::string& parent, const std::string& treeHash);
};

#endif // COMMITNODE_H