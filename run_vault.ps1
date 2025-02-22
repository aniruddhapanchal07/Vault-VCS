# PowerShell script to compile and run Vault

# Set execution policy for the session (optional)
#Set-ExecutionPolicy Bypass -Scope Process

# Define filenames
$sourceFile = "vault.cpp"
$exeFile = "vault.exe"

# Compile the C++ program
Write-Host "Compiling Vault..."
g++ $sourceFile -o $exeFile

# Check if compilation was successful
if ($?) {
    Write-Host "Compilation successful!"
    
    # Run the program interactively
    Write-Host "Starting Vault..."
    .\vault
} else {
    Write-Host "Compilation failed!"
}
