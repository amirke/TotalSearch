# TotalSearch Download Package Creator
# This script creates a zip file from the build/Release directory for website downloads

param(
    [string]$Version = "1.0.0",
    [string]$Platform = "Windows"
)

Write-Host "Creating TotalSearch download package..." -ForegroundColor Green

# Define paths
$BuildDir = "build\Release"
$DownloadsDir = "website\downloads"
$ZipFileName = "TotalSearch-$Platform-v$Version.zip"

# Check if build directory exists
if (-not (Test-Path $BuildDir)) {
    Write-Host "Error: Build directory '$BuildDir' not found!" -ForegroundColor Red
    Write-Host "Please run '.\build.bat' first to build the application." -ForegroundColor Yellow
    exit 1
}

# Create downloads directory if it doesn't exist
if (-not (Test-Path $DownloadsDir)) {
    New-Item -ItemType Directory -Path $DownloadsDir -Force | Out-Null
    Write-Host "Created downloads directory: $DownloadsDir" -ForegroundColor Yellow
}

# Define what to include in the zip
$IncludeFiles = @(
    "TotalSearch.exe",
    "TotalSearchWatchdog.exe", 
    "Qt6Core.dll",
    "Qt6Gui.dll",
    "Qt6Widgets.dll",
    "App.ini",
    "README.md"
)

$IncludeDirectories = @(
    "lib",
    "plugins", 
    "data",
    "docs",
    "scripts"
)

# Create temporary directory for packaging
$TempDir = "temp_package"
if (Test-Path $TempDir) {
    Remove-Item $TempDir -Recurse -Force
}
New-Item -ItemType Directory -Path $TempDir | Out-Null

Write-Host "Copying files to temporary directory..." -ForegroundColor Yellow

# Copy main files
foreach ($file in $IncludeFiles) {
    $sourcePath = Join-Path $BuildDir $file
    if (Test-Path $sourcePath) {
        Copy-Item $sourcePath $TempDir
        Write-Host "  ✓ $file" -ForegroundColor Green
    } else {
        Write-Host "  ⚠ $file (not found)" -ForegroundColor Yellow
    }
}

# Copy directories
foreach ($dir in $IncludeDirectories) {
    $sourcePath = Join-Path $BuildDir $dir
    if (Test-Path $sourcePath) {
        Copy-Item $sourcePath $TempDir -Recurse
        Write-Host "  ✓ $dir/" -ForegroundColor Green
    } else {
        Write-Host "  ⚠ $dir/ (not found)" -ForegroundColor Yellow
    }
}

# Create zip file
$ZipPath = Join-Path $DownloadsDir $ZipFileName
Write-Host "Creating zip file: $ZipPath" -ForegroundColor Yellow

try {
    # Use PowerShell's Compress-Archive
    Compress-Archive -Path "$TempDir\*" -DestinationPath $ZipPath -Force
    
    # Get file size
    $fileSize = (Get-Item $ZipPath).Length
    $fileSizeMB = [math]::Round($fileSize / 1MB, 2)
    
    Write-Host "✓ Package created successfully!" -ForegroundColor Green
    Write-Host "  File: $ZipFileName" -ForegroundColor Cyan
    Write-Host "  Size: $fileSizeMB MB" -ForegroundColor Cyan
    Write-Host "  Location: $ZipPath" -ForegroundColor Cyan
    
} catch {
    Write-Host "Error creating zip file: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
} finally {
    # Clean up temporary directory
    if (Test-Path $TempDir) {
        Remove-Item $TempDir -Recurse -Force
        Write-Host "Cleaned up temporary directory" -ForegroundColor Gray
    }
}

Write-Host "`nPackage ready for website download!" -ForegroundColor Green
Write-Host "The zip file is now available at: website/downloads/$ZipFileName" -ForegroundColor Cyan

