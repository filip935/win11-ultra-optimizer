#Requires -RunAsAdministrator
# Fortnite Ultra-High FPS Optimizer for Windows 11 IoT Enterprise LTSC 2024

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Fortnite FPS Optimizer - LTSC 2024" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# --- Confirm ---
$confirm = Read-Host "This will apply gaming optimizations to your system. Continue? (Y/N)"
if ($confirm -notin @("Y", "y")) {
    Write-Host "Aborted." -ForegroundColor Red
    exit
}

# --- Disable Telemetry & Diagnostics ---
Write-Host "`n[1/12] Disabling telemetry & diagnostics..." -ForegroundColor Yellow
$telemetryKeys = @(
    "HKLM:\SOFTWARE\Policies\Microsoft\Windows\DataCollection",
    "HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Policies\DataCollection"
)
foreach ($key in $telemetryKeys) {
    if (!(Test-Path $key)) { New-Item -Path $key -Force | Out-Null }
}
Set-ItemProperty -Path "HKLM:\SOFTWARE\Policies\Microsoft\Windows\DataCollection" -Name "AllowTelemetry" -Value 0 -Type DWord
Set-ItemProperty -Path "HKLM:\SOFTWARE\Policies\Microsoft\Windows\DataCollection" -Name "MaxTelemetryAllowed" -Value 0 -Type DWord
Stop-Service -Name "DiagTrack" -Force -ErrorAction SilentlyContinue
Set-Service -Name "DiagTrack" -StartupType Disabled -ErrorAction SilentlyContinue
Stop-Service -Name "dmwappushservice" -Force -ErrorAction SilentlyContinue
Set-Service -Name "dmwappushservice" -StartupType Disabled -ErrorAction SilentlyContinue
Write-Host "  Done." -ForegroundColor Green

# --- Disable Bloatware Services ---
Write-Host "`n[2/12] Disabling unnecessary services..." -ForegroundColor Yellow
$servicesToDisable = @(
    "SysMain",           # Superfetch
    "WSearch",           # Windows Search Indexer
    "Spooler",           # Print Spooler
    "Fax",               # Fax
    "MapsBroker",        # Downloaded Maps Manager
    "lfsvc",             # Geolocation Service
    "SharedAccess",      # Internet Connection Sharing
    "RemoteRegistry",    # Remote Registry
    "XblAuthManager",    # Xbox Live Auth
    "XblGameSave",       # Xbox Live Game Save
    "XboxNetApiSvc",     # Xbox Live Networking
    "XboxGipSvc",        # Xbox Accessory Management
    "BthTIMER",          # Bluetooth
    "TabletInputService" # Touch Keyboard / Handwriting
)
foreach ($svc in $servicesToDisable) {
    Stop-Service -Name $svc -Force -ErrorAction SilentlyContinue
    Set-Service -Name $svc -StartupType Disabled -ErrorAction SilentlyContinue
    Write-Host "  Disabled: $svc" -ForegroundColor Gray
}

# --- Disable Game Bar & Game DVR ---
Write-Host "`n[3/12] Disabling Game Bar & Game DVR..." -ForegroundColor Yellow
$gameBarKeys = @(
    "HKCU:\SOFTWARE\Microsoft\Windows\CurrentVersion\GameDVR",
    "HKCU:\SOFTWARE\Microsoft\Windows\CurrentVersion\GameBar",
    "HKLM:\SOFTWARE\Policies\Microsoft\Windows\GameDVR",
    "HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\GameDVR"
)
foreach ($key in $gameBarKeys) {
    if (!(Test-Path $key)) { New-Item -Path $key -Force | Out-Null }
}
Set-ItemProperty -Path "HKCU:\SOFTWARE\Microsoft\Windows\CurrentVersion\GameDVR" -Name "AppCaptureEnabled" -Value 0 -Type DWord
Set-ItemProperty -Path "HKCU:\SOFTWARE\Microsoft\Windows\CurrentVersion\GameBar" -Name "AllowGameDVR" -Value 0 -Type DWord
Set-ItemProperty -Path "HKCU:\SOFTWARE\Microsoft\Windows\CurrentVersion\GameBar" -Name "UseNexusForGameBarEnabled" -Value 0 -Type DWord
Set-ItemProperty -Path "HKLM:\SOFTWARE\Policies\Microsoft\Windows\GameDVR" -Name "AllowGameDVR" -Value 0 -Type DWord
Write-Host "  Done." -ForegroundColor Green

# --- Enable Ultimate Performance Power Plan ---
Write-Host "`n[4/12] Setting Ultimate Performance power plan..." -ForegroundColor Yellow
$ultimateGUID = "e9a42b02-d5df-448d-aa00-03f14749eb61"
powercfg /duplicatescheme $ultimateGUID | Out-Null
powercfg /setactive $ultimateGUID
powercfg /change monitor-timeout-ac 0
powercfg /change standby-timeout-ac 0
powercfg /change hibernate-timeout-ac 0
Write-Host "  Ultimate Performance plan activated." -ForegroundColor Green

# --- GPU Scheduling & Fullscreen Optimizations ---
Write-Host "`n[5/12] Enabling GPU scheduling & disabling fullscreen optimizations..." -ForegroundColor Yellow
$gpuKey = "HKLM:\SYSTEM\CurrentControlSet\Control\GraphicsDrivers"
if (!(Test-Path $gpuKey)) { New-Item -Path $gpuKey -Force | Out-Null }
Set-ItemProperty -Path $gpuKey -Name "HwSchMode" -Value 2 -Type DWord

# Disable fullscreen optimizations globally
$explorerKey = "HKCU:\System\GameConfigStore"
if (!(Test-Path $explorerKey)) { New-Item -Path $explorerKey -Force | Out-Null }
Set-ItemProperty -Path $explorerKey -Name "GameDVR_FSEBehaviorMode" -Value 2 -Type DWord
Set-ItemProperty -Path $explorerKey -Name "GameDVR_HonorUserFSEBehaviorMode" -Value 1 -Type DWord
Set-ItemProperty -Path $explorerKey -Name "GameDVR_FSEBehavior" -Value 2 -Type DWord
Set-ItemProperty -Path $explorerKey -Name "GameDVR_DXGIHonorFSEWindowsCompatible" -Value 1 -Type DWord
Set-ItemProperty -Path $explorerKey -Name "GameDVR_EFSEFeatureFlags" -Value 0 -Type DWord
Write-Host "  Done." -ForegroundColor Green

# --- Mouse Settings ---
Write-Host "`n[6/12] Optimizing mouse settings..." -ForegroundColor Yellow
$mouseKey = "HKCU:\Control Panel\Mouse"
Set-ItemProperty -Path $mouseKey -Name "MouseSpeed" -Value "0"
Set-ItemProperty -Path $mouseKey -Name "MouseThreshold1" -Value "0"
Set-ItemProperty -Path $mouseKey -Name "MouseThreshold2" -Value "0"
Write-Host "  Mouse acceleration disabled." -ForegroundColor Green

# --- Disable Notifications ---
Write-Host "`n[7/12] Disabling notifications..." -ForegroundColor Yellow
$notifKey = "HKCU:\SOFTWARE\Microsoft\Windows\CurrentVersion\PushNotifications"
if (!(Test-Path $notifKey)) { New-Item -Path $notifKey -Force | Out-Null }
Set-ItemProperty -Path $notifKey -Name "ToastEnabled" -Value 0 -Type DWord
Write-Host "  Done." -ForegroundColor Green

# --- Disable Visual Effects ---
Write-Host "`n[8/12] Disabling visual effects..." -ForegroundColor Yellow
$visualKey = "HKCU:\Software\Microsoft\Windows\CurrentVersion\Explorer\VisualEffects"
if (!(Test-Path $visualKey)) { New-Item -Path $visualKey -Force | Out-Null }
Set-ItemProperty -Path $visualKey -Name "VisualFXSetting" -Value 2 -Type DWord

# Disable animations, transparency, shadows
Set-ItemProperty -Path "HKCU:\Control Panel\Desktop" -Name "UserPreferencesMask" -Value ([byte[]](0x90,0x12,0x03,0x80,0x10,0x00,0x00,0x00)) -Type Binary
Set-ItemProperty -Path "HKCU:\Control Panel\Desktop\WindowMetrics" -Name "MinAnimate" -Value "0"
Set-ItemProperty -Path "HKCU:\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\Advanced" -Name "ListviewAlphaSelect" -Value 0 -Type DWord
Set-ItemProperty -Path "HKCU:\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\Advanced" -Name "ListviewShadow" -Value 0 -Type DWord
Set-ItemProperty -Path "HKCU:\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\Advanced" -Name "TaskbarAnimations" -Value 0 -Type DWord
Set-ItemProperty -Path "HKCU:\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\Advanced" -Name "IconsOnly" -Value 1 -Type DWord

# Disable transparency
$personalizeKey = "HKCU:\SOFTWARE\Microsoft\Windows\CurrentVersion\Themes\Personalize"
if (!(Test-Path $personalizeKey)) { New-Item -Path $personalizeKey -Force | Out-Null }
Set-ItemProperty -Path $personalizeKey -Name "EnableTransparency" -Value 0 -Type DWord
Write-Host "  Visual effects minimized." -ForegroundColor Green

# --- Network Optimization ---
Write-Host "`n[9/12] Optimizing network for gaming..." -ForegroundColor Yellow
# Disable Nagle's Algorithm (reduces latency)
$interfaces = Get-ChildItem "HKLM:\SYSTEM\CurrentControlSet\Services\Tcpip\Parameters\Interfaces" -ErrorAction SilentlyContinue
foreach ($iface in $interfaces) {
    Set-ItemProperty -Path $iface.PSPath -Name "TcpAckFrequency" -Value 1 -Type DWord -ErrorAction SilentlyContinue
    Set-ItemProperty -Path $iface.PSPath -Name "TCPNoDelay" -Value 1 -Type DWord -ErrorAction SilentlyContinue
    Set-ItemProperty -Path $iface.PSPath -Name "TcpDelAckTicks" -Value 0 -Type DWord -ErrorAction SilentlyContinue
}

# Disable Network Throttling
$netThrottle = "HKLM:\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Multimedia\SystemProfile"
if (!(Test-Path $netThrottle)) { New-Item -Path $netThrottle -Force | Out-Null }
Set-ItemProperty -Path $netThrottle -Name "NetworkThrottlingIndex" -Value 0xFFFFFFFF -Type DWord
Set-ItemProperty -Path $netThrottle -Name "SystemResponsiveness" -Value 0 -Type DWord

# Gaming priority
$gameProfile = "HKLM:\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Multimedia\SystemProfile\Tasks\Games"
if (!(Test-Path $gameProfile)) { New-Item -Path $gameProfile -Force | Out-Null }
Set-ItemProperty -Path $gameProfile -Name "GPU Priority" -Value 8 -Type DWord
Set-ItemProperty -Path $gameProfile -Name "Priority" -Value 6 -Type DWord
Set-ItemProperty -Path $gameProfile -Name "Scheduling Category" -Value "High" -Type String
Set-ItemProperty -Path $gameProfile -Name "SFIO Priority" -Value "High" -Type String
Write-Host "  Network optimized for low latency." -ForegroundColor Green

# --- Priority for Fortnite ---
Write-Host "`n[10/12] Setting Fortnite priority..." -ForegroundColor Yellow
$regPath = "HKLM:\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options\FortniteClient-Win64-Shipping.exe\PerfOptions"
if (!(Test-Path $regPath)) { New-Item -Path $regPath -Force | Out-Null }
Set-ItemProperty -Path $regPath -Name "CpuPriorityClass" -Value 3 -Type DWord
Write-Host "  Fortnite set to High CPU priority." -ForegroundColor Green

# --- Disable Startup Apps ---
Write-Host "`n[11/12] Checking startup apps..." -ForegroundColor Yellow
Write-Host "  (Review these manually in Task Manager > Startup)" -ForegroundColor Gray
Write-Host "  Disable anything unnecessary (Discord overlay, OneDrive, etc.)" -ForegroundColor Gray

# --- Cleanup ---
Write-Host "`n[12/12] Running system cleanup..." -ForegroundColor Yellow
Remove-Item -Path "$env:TEMP\*" -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item -Path "C:\Windows\Temp\*" -Recurse -Force -ErrorAction SilentlyContinue
Write-Host "  Temp files cleaned." -ForegroundColor Green

# --- Fortnite Config ---
Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "  Applying Fortnite config..." -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

$fortniteConfig = "$env:LOCALAPPDATA\FortniteGame\Saved\Config\WindowsClient"
$gameUserSettings = "$fortniteConfig\GameUserSettings.ini"

if (Test-Path $gameUserSettings) {
    $content = Get-Content $gameUserSettings -Raw

    # Performance settings
    $replacements = @{
        "sg.ResolutionQuality="       = "sg.ResolutionQuality=100.000000"
        "sg.ViewDistanceQuality="     = "sg.ViewDistanceQuality=0"
        "sg.AntiAliasingQuality="     = "sg.AntiAliasingQuality=0"
        "sg.ShadowQuality="          = "sg.ShadowQuality=0"
        "sg.PostProcessQuality="     = "sg.PostProcessQuality=0"
        "sg.TextureQuality="         = "sg.TextureQuality=0"
        "sg.EffectsQuality="         = "sg.EffectsQuality=0"
        "sg.FoliageQuality="         = "sg.FoliageQuality=0"
        "bShowFPS="                  = "bShowFPS=True"
        "bUseVSync="                 = "bUseVSync=False"
        "bMotionBlur="               = "bMotionBlur=False"
    }

    foreach ($key in $replacements.Keys) {
        if ($content -match [regex]::Escape($key)) {
            $content = $content -replace [regex]::Escape($key), $replacements[$key]
        }
    }

    Set-Content -Path $gameUserSettings -Value $content -Force
    Write-Host "  Fortnite config optimized." -ForegroundColor Green
} else {
    Write-Host "  Fortnite config not found. Launch Fortnite once first." -ForegroundColor Yellow
    Write-Host "  Config will be at: $gameUserSettings" -ForegroundColor Gray
}

# --- Final ---
Write-Host "`n========================================" -ForegroundColor Green
Write-Host "  All optimizations applied!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "Restart your PC for all changes to take effect." -ForegroundColor Cyan
Write-Host ""
Read-Host "Press Enter to exit"
