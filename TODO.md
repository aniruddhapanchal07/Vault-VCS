# TODO

## Basic Workflow & Tracking

###  (1.0.0-alpha.1)
* Init (Done)
* Add/Stage (Done)
* Commit (Done)
* Status (Done)

### (1.0.0-alpha.2)
* .ignore
* Diff


## History & Inspection

### (1.1.0-alpha.0)
* Clean
* Tag

### (1.1.0-alpha.1)
* Log/Histroy

### (1.1.0-alpha.2)
* Checkout/Switch/Restore
* Revert
* Reset

### (1.1.0-alpha.3)
* Show/Cat/Instpect Object
* Blame/Annotate

## Remote Operations (2.0.0-alpha.4)
* Config
* Init/Clone
* Fetch
* Pull 
* Push 


# Bugs
* [vault add *]  on a dir containing .png file
    ```
    λ vanilla Search-bar-Autocomplete →  vault add *
    terminate called after throwing an instance of 'std::filesystem::__cxx11::filesystem_error'
    what():  filesystem error: directory iterator cannot open directory: Not a directory [AutoComplete.png]
    [1]    70968 IOT instruction (core dumped)  vault add *
    ```
* [valut add *] only some of the dir are being added to stageing area insted of all the files.
    ```
    λ vanilla Search-bar-Autocomplete →  vault status
    Staged: /home/aman/Projects/cloned/Search-bar-Autocomplete/data/test_data_sample.csv
    Staged: /home/aman/Projects/cloned/Search-bar-Autocomplete/data/data_generator/mydata.csv
    Staged: /home/aman/Projects/cloned/Search-bar-Autocomplete/data/data_generator/sample.csv
    Staged: /home/aman/Projects/cloned/Search-bar-Autocomplete/data/data_generator/Untitled.ipynb
    Staged: /home/aman/Projects/cloned/Search-bar-Autocomplete/data/data_generator/datagenerator.py
    Staged: /home/aman/Projects/cloned/Search-bar-Autocomplete/data/data_generator/.ipynb_checkpoints/Untitled-checkpoint.ipynb
    Untracked: /home/aman/Projects/cloned/Search-bar-Autocomplete/README.md
    Untracked: /home/aman/Projects/cloned/Search-bar-Autocomplete/src/trie.py
    Untracked: /home/aman/Projects/cloned/Search-bar-Autocomplete/src/app.py
    Untracked: /home/aman/Projects/cloned/Search-bar-Autocomplete/src/utils.py
    Untracked: /home/aman/Projects/cloned/Search-bar-Autocomplete/src/__pycache__/utils.cpython-313.pyc
    Untracked: /home/aman/Projects/cloned/Search-bar-Autocomplete/src/__pycache__/trie.cpython-313.pyc
    Untracked: /home/aman/Projects/cloned/Search-bar-Autocomplete/src/templates/index.html
    λ vanilla Search-bar-Autocomplete →  vault status
    Staged: /home/aman/Projects/cloned/Search-bar-Autocomplete/data/test_data_sample.csv
    Staged: /home/aman/Projects/cloned/Search-bar-Autocomplete/data/data_generator/mydata.csv
    Staged: /home/aman/Projects/cloned/Search-bar-Autocomplete/data/data_generator/sample.csv
    Staged: /home/aman/Projects/cloned/Search-bar-Autocomplete/data/data_generator/Untitled.ipynb
    Staged: /home/aman/Projects/cloned/Search-bar-Autocomplete/data/data_generator/datagenerator.py
    Staged: /home/aman/Projects/cloned/Search-bar-Autocomplete/data/data_generator/.ipynb_checkpoints/Untitled-checkpoint.ipynb
    Untracked: /home/aman/Projects/cloned/Search-bar-Autocomplete/README.md
    Untracked: /home/aman/Projects/cloned/Search-bar-Autocomplete/src/trie.py
    Untracked: /home/aman/Projects/cloned/Search-bar-Autocomplete/src/app.py
    Untracked: /home/aman/Projects/cloned/Search-bar-Autocomplete/src/utils.py
    Untracked: /home/aman/Projects/cloned/Search-bar-Autocomplete/src/__pycache__/utils.cpython-313.pyc
    Untracked: /home/aman/Projects/cloned/Search-bar-Autocomplete/src/__pycache__/trie.cpython-313.pyc
    Untracked: /home/aman/Projects/cloned/Search-bar-Autocomplete/src/templates/index.html
    ```
