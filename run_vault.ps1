# PowerShell script to compile and run Vault with OpenSSL support

# Set execution policy for the session (optional)
#Set-ExecutionPolicy Bypass -Scope Process

# Define filenames
$sourceFile = "vault.cpp"
$exeFile = "vault.exe"

# Define OpenSSL paths
$opensslIncludePath = "C:\Program Files\OpenSSL-Win64\include"  # Path to OpenSSL include directory
$opensslLibPath = "C:\Program Files\OpenSSL-Win64\lib"          # Path to OpenSSL lib directory

# Compile the C++ program with OpenSSL support
Write-Host "Compiling Vault with OpenSSL..."
g++ $sourceFile -o $exeFile -I"$opensslIncludePath" -L"$opensslLibPath" -lssl -lcrypto

# Check if compilation was successful
if ($?) {
    Write-Host "Compilation successful!"
    
    # Run the program interactively
    Write-Host "Starting Vault..."
    & ".\$exeFile"  # Use the call operator (&) to execute the program
} else {
    Write-Host "Compilation failed!"
}