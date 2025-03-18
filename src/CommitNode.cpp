#include "CommitNode.h"
#include <ctime>
#include <sstream>
#include <iomanip>

CommitNode::CommitNode(const std::string& hash, const std::string& msg, const std::string& parent, const std::string& treeHash)
    : commitHash(hash), message(msg), parentHash(parent), treeHash(treeHash), next(nullptr) {
    
        std::time_t now = std::time(nullptr);
    std::tm timeinfo;

    #ifdef _WIN32
        localtime_s(&timeinfo, &now);
    #else
        localtime_r(&now, &timeinfo);
    #endif
    
    std::ostringstream oss;
    oss << std::put_time(&timeinfo, "%a %b %d %H:%M:%S %Y");
    timestamp = oss.str();
}