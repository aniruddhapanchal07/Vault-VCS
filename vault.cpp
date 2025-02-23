#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <openssl/sha.h> // SHA-1 hashing

namespace fs = std::filesystem;

// Node structure for the linked list representing commit history
struct CommitNode {
    std::string commitHash;
    std::string message;
    std::string parentHash;
    std::unordered_map<std::string, std::string> files;

    CommitNode* next; // Pointer to the next commit in the list

    CommitNode(const std::string& hash, const std::string& msg, const std::string& parent)
        : commitHash(hash), message(msg), parentHash(parent), next(nullptr) {}
};

class Vault {
private:
    std::string repoPath;
    std::string objectsPath;
    std::string headPath;
    std::string indexPath;
    CommitNode* headCommit;

public:

    Vault(const std::string& repoPath = ".") {
        this->repoPath = repoPath + "/.vault";
        this->objectsPath = this->repoPath + "/objects";
        this->headPath = this->repoPath + "/HEAD";
        this->indexPath = this->repoPath + "/index";
        this->headCommit = nullptr;

        if (!fs::exists(this->repoPath)) {
            init(); // Initialize the repository if it doesn't exist
        } else {
            std::cerr << "Error: Repository already initialized!\n";
        }
    }

    // Destructor: Clean up the linked list to avoid memory leaks
    ~Vault() {
        while (headCommit != nullptr) {
            CommitNode* temp = headCommit;
            headCommit = headCommit->next;
            delete temp;
        }
    }

    // Initialize a new repository
    void init() {
        if (!fs::exists(this->repoPath)) {
            try {
                fs::create_directories(objectsPath);
                std::ofstream headFile(headPath, std::ios::trunc);
                std::ofstream indexFile(indexPath, std::ios::trunc);
                std::cout << "Initialized .vault repository" << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Error initializing repository: " << e.what() << std::endl;
            }
        } else {
            std::cerr << "Error: Repository already initialized!\n";
        }
    }

    // Generate a SHA-1 hash for file content
    std::string sha1Hash(const std::string& content) {
        unsigned char hash[SHA_DIGEST_LENGTH];
        SHA1(reinterpret_cast<const unsigned char*>(content.c_str()), content.size(), hash);

        std::stringstream ss;
        for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
        }

        return ss.str();
    }

    // Add a file to the staging area (index)
    void addFile(const std::string& filePath) {
        std::ifstream file(filePath);
        if (!file) {
            std::cerr << "Error: Could not open file " << filePath << std::endl;
            return;
        }
        
        // Buffers used to read the file content and write to memory and store it in another content file
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();
        std::string fileHash = sha1Hash(content);

        // Convert to relative path before storing to store the relative path to index(Staging Area)
        std::string relativePath = fs::relative(filePath, fs::current_path()).string();

        std::ofstream objectFile(objectsPath + "/" + fileHash);
        objectFile << content;
        objectFile.close();

        std::ofstream indexFile(indexPath, std::ios::app);
        indexFile << relativePath << " " << fileHash << "\n";
        indexFile.close();

        std::cout << "Added " << relativePath << std::endl;
    }

    // Add a file or directory to the staging area
    void add(const std::string& path) {
        if (fs::is_directory(path)) {
            for (const auto& entry : fs::recursive_directory_iterator(path)) {
                if (fs::is_regular_file(entry.path())) {
                    addFile(entry.path().string());
                }
            }
        } else if (fs::is_regular_file(path)) {
            addFile(path);
        } else {
            std::cerr << "Invalid file or directory: " << path << std::endl;
        }
    }

    // Commit staged changes with a message
    void commit(const std::string& message) {
        std::ifstream indexFile(indexPath);
        std::stringstream buffer;
        buffer << indexFile.rdbuf();
        std::string indexContent = buffer.str();

        if (indexContent.empty()) {
            std::cout << "No changes to commit." << std::endl;
            return;
        }

        std::string commitHash = sha1Hash(indexContent + message);

        // Create a new commit node
        std::string parentHash = (headCommit != nullptr) ? headCommit->commitHash : "";
        CommitNode* newCommit = new CommitNode(commitHash, message, parentHash);

        // Parse the index file to populate the files map in the commit
        std::istringstream iss(indexContent);
        std::string line;
        while (std::getline(iss, line)) {
            std::istringstream lineStream(line);
            std::string filePath, fileHash;
            if (lineStream >> filePath >> fileHash) {
                newCommit->files[filePath] = fileHash;
            }
        }

        // Add the new commit to the linked list
        newCommit->next = headCommit;
        headCommit = newCommit;

        // Save the commit to the objects directory
        std::string commitFilePath = objectsPath + "/" + commitHash;
        std::ofstream commitFile(commitFilePath);
        commitFile << "Commit: " << commitHash << "\n";
        commitFile << "Message: " << message << "\n";
        commitFile << "Parent: " << parentHash << "\n";
        commitFile << indexContent;
        commitFile.close();

        // Update the HEAD file
        std::ofstream headFileOut(headPath);
        headFileOut << commitHash;
        headFileOut.close();

        // Clear the index file
        std::ofstream clearIndex(indexPath, std::ios::trunc);
        clearIndex.close();

        std::cout << "Commit successfully created: " << commitHash << std::endl;
    }
};

int main() {
    Vault vault;
    std::string input;

    std::cout << "Vault Interactive Mode. Type 'help' for commands, 'exit' to quit." << std::endl;

    while (true) {
        std::cout << "vault> ";
        std::getline(std::cin, input);

        if (input == "exit") {
            break;
        } else if (input == "help") {
            std::cout << "\nAvailable commands:\n";
            std::cout << "  init           - Initialize a new repository\n";
            std::cout << "  add <path>     - Add a file or directory to staging\n";
            std::cout << "  commit <msg>   - Commit staged changes with a message\n";
            std::cout << "  exit           - Exit Vault\n\n";
        } else if (input == "init") {
            vault.init();
        } else if (input.rfind("add ", 0) == 0) {
            vault.add(input.substr(4));
        } else if (input.rfind("commit ", 0) == 0) {
            vault.commit(input.substr(7));
        } else {
            std::cout << "Unknown command. Type 'help' for a list of commands.\n";
        }
    }

    std::cout << "Exiting Vault.\n";
    return 0;
}