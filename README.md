# Vault-VCS

## Overview
This project is an implementation of a local version control system "Vault" written in c++. It's meant to be a simpler alternative to Git, focusing on just the core features you need to track changes in your code.

## Features

- **Repository Management**: Initialize a new .vault repository with a structured directory layout
- **File Tracking**: Hash-based file content tracking for efficient storage
- **Commit History**: Maintain a linked history of commits with timestamps and messages
- **Tree Structure**: Recursive directory tree representation of your project
- **Content Addressing**: SHA-1 based content addressing for file integrity
- **Command-line Interface**: Simple, intuitive commands for version control operations

## Commands

- **vault init**: Initialize a new Vault repository
- **vault add <path>**: Add files or directories to the staging area
- **vault commit <message>**: Commit staged changes with a message
- **vault log**: Display commit history with timestamps and messages

## Project Structure

```
project/
├── build/
├── deps/
│   ├── include/
│   ├── ├── openssl/
│   ├── ├── ├── sha.h 
│   ├── lib
├── src/
│   ├── add.cpp
│   ├── commit.cpp
│   ├── CommitNode.cpp
│   ├── CommitNode.h
│   ├── init.cpp
│   ├── log.cpp
│   ├── main.cpp
│   ├── Tree.h
│   ├── vault.cpp
│   ├── vault.h
├── CMakeLists.txt
├── README.md
```

## Dependencies

- **OpenSSl**: For SHA-1 Hashing.
- **CMake**: For building the project.


## Build Instructions

### Windows

- Install dependencies (e.g., via vcpkg or other package manager).

- Clone the repository:

  ```bash
  git clone https://github.com/aniruddhapanchal07/Vault-VCS.git
  cd Vault-VCS
  ```

- Build the project:

  ```bash
  mkdir build
  cd build
  cmake .. -G "Unix Makefiles"
  make
  ```

## System-wide Access

To use Vault from anywhere in your system (like Git), you need to add the Vault executable to your system's PATH:

### Windows

- Copy the vault.exe to a permanent location (e.g., C:\Program Files\Vault\)
- Open System Properties (Win+X > System)
- Click on "Advanced system settings"
- Click on "Environment Variables"
- Under "System variables", find and select "Path", then click "Edit"
- Click "New" and add the path to your Vault executable (e.g., C:\Program Files\Vault\)
- Click "OK" on all dialogs to save

### Linux/macOS

- Copy the vault executable to /usr/local/bin/ (may require sudo)

  ```bash
  sudo cp ./vault /usr/local/bin/
  ```

- Make sure it's executable:

  ```bash
  sudo chmod +x /usr/local/bin/vault
  ```

## Usage Examples

  ```bash
  # Initialize a new repository
  vault init

  # Add files to staging area
  vault add myfile.txt
  vault add src/

  # Commit changes
  vault commit "Initial commit"

  # View commit history
  vault log
  ```

## File Structure
  ```bash
  .vault/
├── objects/      # Stores file contents by hash
├── commits/      # Commit information and metadata
├── trees/        # Directory tree structure objects
├── HEAD          # Points to the current commit
├── index         # Staging area for changes
└── staged_tree   # Temporary tree for staging
  ```
