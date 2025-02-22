# Vault-VCS

Vault is a simple version control system implemented in C++. It allows you to initialize a repository, add files to a staging area, and commit changes with messages. The project is a lightweight alternative to more complex systems like Git, focusing on basic version control functionality.

## Features

* Initialize Repository: Create a new .vault repository with necessary directories and files.
* Add Files: Stage files or directories for commit.
* Commit Changes: Save staged changes with a commit message.
* Interactive Mode: Command-line interface with helpful commands.

## Commands

* init: Initialize a new Vault repository.
* add <path>: Add a file or directory to the staging area.
* commit <message>: Commit staged changes with a message.
* help: Display available commands.
* exit: Exit the Vault interactive mode.


## Run Locally
### Prerequisites
* Windows PowerShell
* C++ compiler supporting C++17 or later
Clone the project

```bash
  git clone https://github.com/aniruddhapanchal07/Vault-VCS.git
```

Go to the project directory

```bash
  cd Vault-VCS
```

### Compile and Run

Use the provided PowerShell script to compile and run the vault.cpp file:

```bash
  .\run_vault.ps1
```


## File Structure
* .vault/objects: Stores file contents and commit data.
* .vault/HEAD: Tracks the latest commit.
* .vault/index: Temporary storage for staged files
