#Requires -RunAsAdministrator
# Bloatware Remover for Windows 11 IoT Enterprise LTSC 2024

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  LTSC Bloatware Remover" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "  NOTE: windows.old will be DELETED. File History will NOT be touched." -ForegroundColor Yellow
Write-Host ""

# --- Confirm ---
$confirm = Read-Host "Continue? (Y/N)"
if ($confirm -notin @("Y", "y")) {
    Write-Host "Aborted." -ForegroundColor Red
    exit
}

# --- Temp Files ---
Write-Host "`n[1/4] Cleaning temp files..." -ForegroundColor Yellow
Remove-Item -Path "$env:TEMP\*" -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item -Path "C:\Windows\Temp\*" -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item -Path "C:\Windows\Prefetch\*" -Recurse -Force -ErrorAction SilentlyContinue
Write-Host "  Temp files cleaned." -ForegroundColor Green

# --- Delete windows.old ---
Write-Host "`n[2/4] Deleting windows.old (old Windows 10 install)..." -ForegroundColor Yellow
if (Test-Path "C:\Windows.old") {
    Remove-Item -Path "C:\Windows.old" -Recurse -Force -ErrorAction SilentlyContinue
    if (!(Test-Path "C:\Windows.old")) {
        Write-Host "  windows.old deleted." -ForegroundColor Green
    } else {
        Write-Host "  Could not fully delete windows.old. Run Disk Cleanup as admin." -ForegroundColor Red
    }
} else {
    Write-Host "  windows.old not found." -ForegroundColor Gray
}

# --- Remove Xbox Packages ---
Write-Host "`n[3/4] Removing Xbox packages..." -ForegroundColor Yellow
$xboxPackages = @(
    "Microsoft.GamingApp",
    "Microsoft.Xbox.TCUI",
    "Microsoft.XboxGameOverlay",
    "Microsoft.XboxGamingOverlay",
    "Microsoft.XboxIdentityProvider",
    "Microsoft.XboxSpeechToTextOverlay",
    "Microsoft.Xbox"
)
foreach ($pkg in $xboxPackages) {
    Get-AppxPackage -Name $pkg -AllUsers -ErrorAction SilentlyContinue | Remove-AppxPackage -AllUsers -ErrorAction SilentlyContinue
    Get-AppxProvisionedPackage -Online | Where-Object {$_.PackageName -like "*$pkg*"} | Remove-AppxProvisionedPackage -Online -ErrorAction SilentlyContinue
    Write-Host "  Removed: $pkg" -ForegroundColor Gray
}
Write-Host "  Xbox packages removed." -ForegroundColor Green

# --- Remove Microsoft Edge ---
Write-Host "`n[4/4] Removing Microsoft Edge..." -ForegroundColor Yellow
$edgePath = "C:\Program Files (x86)\Microsoft\Edge\Application\*\Installer\setup.exe"
$edgeSetup = Get-Item $edgePath -ErrorAction SilentlyContinue | Select-Object -First 1

if ($edgeSetup) {
    Write-Host "  Running Edge uninstaller..." -ForegroundColor Gray
    Start-Process -FilePath $edgeSetup.FullName -ArgumentList "--uninstall --system-level --verbose-logging" -Wait -ErrorAction SilentlyContinue
    Write-Host "  Edge uninstalled." -ForegroundColor Green
} else {
    Write-Host "  Edge installer not found, trying alternative..." -ForegroundColor Gray
    $edgeKeys = @(
        "HKLM:\SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\Microsoft Edge",
        "HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Microsoft Edge"
    )
    foreach ($key in $edgeKeys) {
        if (Test-Path $key) {
            $uninstallStr = (Get-ItemProperty $key -ErrorAction SilentlyContinue).UninstallString
            if ($uninstallStr) {
                Start-Process cmd -ArgumentList "/c $uninstallStr" -Wait -ErrorAction SilentlyContinue
            }
        }
    }
}

# Remove Edge shortcuts
Remove-Item -Path "$env:USERPROFILE\Desktop\Microsoft Edge.lnk" -Force -ErrorAction SilentlyContinue
Remove-Item -Path "C:\Users\Public\Desktop\Microsoft Edge.lnk" -Force -ErrorAction SilentlyContinue
Remove-Item -Path "$env:APPDATA\Microsoft\Internet Explorer\Quick Launch\User Pinned\TaskBar\Microsoft Edge.lnk" -Force -ErrorAction SilentlyContinue

Write-Host "  Edge removal complete." -ForegroundColor Green

# --- Final ---
Write-Host "`n========================================" -ForegroundColor Green
Write-Host "  Cleanup complete!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "  windows.old: DELETED" -ForegroundColor Gray
Write-Host "  File History: NOT deleted" -ForegroundColor Gray
Write-Host ""
Read-Host "Press Enter to exit"
