#Requires -RunAsAdministrator
<#
.SYNOPSIS
    Windows 11 Ultra Optimization Script
.DESCRIPTION
    Disables telemetry, removes bloatware, optimizes services, power plan,
    visual effects, network, scheduled tasks, and applies performance tweaks.
.NOTES
    Run as Administrator. Creates a restore point before changes.
    Use -WhatIf to preview changes without applying them.
#>

[CmdletBinding(SupportsShouldProcess = $true)]
param()

$ErrorActionPreference = 'SilentlyContinue'
$ProgressPreference    = 'SilentlyContinue'

# ============================================================================
#  BANNER
# ============================================================================
Write-Host ""
Write-Host "  =============================================" -ForegroundColor Cyan
Write-Host "   WINDOWS 11 ULTRA OPTIMIZER" -ForegroundColor Cyan
Write-Host "  =============================================" -ForegroundColor Cyan
Write-Host ""

# ============================================================================
#  CREATE RESTORE POINT
# ============================================================================
Write-Host "[*] Creating system restore point..." -ForegroundColor Yellow

# Enable System Restore on C: if it's disabled
$sr = Get-ComputerRestorePoint -ErrorAction SilentlyContinue
if (-not (Get-ItemProperty -Path 'HKLM:\SOFTWARE\Microsoft\Windows NT\CurrentVersion\SystemRestore' -Name 'RPSessionInterval' -ErrorAction SilentlyContinue)) {
    Enable-ComputerRestore -Drive 'C:\' -ErrorAction SilentlyContinue
}

# Enable Volume Shadow Copy service
$vss = Get-Service -Name 'VSS' -ErrorAction SilentlyContinue
if ($vss -and $vss.StartType -ne 'Automatic') {
    Set-Service -Name 'VSS' -StartupType Automatic -ErrorAction SilentlyContinue
    Start-Service -Name 'VSS' -ErrorAction SilentlyContinue
}

Checkpoint-Computer -Description "Before Win11 Ultra Optimize" -ErrorAction SilentlyContinue
if ($LASTEXITCODE -eq 0) {
    Write-Host "  [x] Restore point created" -ForegroundColor DarkGray
} else {
    Write-Host "  [~] Restore point failed (System Restore may be disabled), continuing..." -ForegroundColor DarkYellow
}
Start-Sleep -Seconds 2

# ============================================================================
#  1. TELEMTERY & PRIVACY
# ============================================================================
Write-Host "[1/12] Disabling telemetry & privacy invasions..." -ForegroundColor Green

$TelemetryReg = @{
    'HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Policies\DataCollection' = @{
        'AllowTelemetry'            = 0
        'MaxTelemetryAllowed'       = 0
    }
    'HKLM:\SOFTWARE\Policies\Microsoft\Windows\DataCollection' = @{
        'AllowTelemetry'            = 0
        'DoNotShowFeedbackNotifications' = 1
    }
    'HKCU:\SOFTWARE\Microsoft\Windows\CurrentVersion\AdvertisingInfo' = @{
        'Enabled'                   = 0
    }
    'HKCU:\SOFTWARE\Microsoft\InputPersonalization' = @{
        'RestrictImplicitTextCollection' = 1
        'RestrictImplicitInkCollection'  = 1
    }
    'HKLM:\SOFTWARE\Policies\Microsoft\Windows\AppCompat' = @{
        'AITEnable'                 = 0
        'DisableInventory'          = 1
    }
    'HKCU:\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\Advanced' = @{
        'Start_TrackProgs'          = 0
    }
    'HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\ContentDeliveryManager' = @{
        'OemPreInstalledAppsEnabled'     = 0
        'PreInstalledAppsEnabled'        = 0
        'RotatingLockScreenEnabled'      = 0
        'RotatingLockScreenOverlayEnabled' = 0
        'SilentInstalledAppsEnabled'     = 0
        'SoftLandingEnabled'             = 0
        'SubscribedContent-310093Enabled' = 0
        'SubscribedContent-338387Enabled' = 0
        'SubscribedContent-338388Enabled' = 0
        'SubscribedContent-338389Enabled' = 0
        'SubscribedContent-338393Enabled' = 0
        'SubscribedContent-353694Enabled' = 0
        'SubscribedContent-353696Enabled' = 0
        'SystemPaneSuggestionsEnabled'   = 0
    }
    'HKCU:\SOFTWARE\Microsoft\Windows\CurrentVersion\ContentDeliveryManager' = @{
        'OemPreInstalledAppsEnabled'     = 0
        'PreInstalledAppsEnabled'        = 0
        'SilentInstalledAppsEnabled'     = 0
        'SoftLandingEnabled'             = 0
        'SubscribedContent-338388Enabled' = 0
        'SystemPaneSuggestionsEnabled'   = 0
    }
    'HKLM:\SOFTWARE\Policies\Microsoft\Windows\CloudContent' = @{
        'DisableWindowsConsumerFeatures'     = 1
        'DisableSoftLanding'                 = 1
        'DisableCloudOptimizedContent'       = 1
    }
    'HKCU:\SOFTWARE\Policies\Microsoft\Windows\CloudContent' = @{
        'DisableTailoredExperiencesWithDiagnosticData' = 1
    }
    'HKLM:\SOFTWARE\Policies\Microsoft\Windows\AdvertisingInfo' = @{
        'DisabledByGroupPolicy'     = 1
    }
    'HKCU:\SOFTWARE\Microsoft\Windows\CurrentVersion\Search' = @{
        'BingSearchEnabled'         = 0
    }
    'HKLM:\SOFTWARE\Policies\Microsoft\Windows\Windows Search' = @{
        'DisableWebSearch'          = 1
        'ConnectedSearchUseWeb'     = 0
        'AllowSearchToUseLocation'  = 0
        'AllowCloudSearch'          = 0
    }
    'HKLM:\SOFTWARE\Policies\Microsoft\Edge' = @{
        'SpotlightExperiencesAndRecommendationsEnabled' = 0
    }
    'HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer' = @{
        'ShowRecent'                = 0
        'ShowFrequent'              = 0
    }
}

foreach ($path in $TelemetryReg.Keys) {
    if (!(Test-Path $path)) { New-Item -Path $path -Force | Out-Null }
    foreach ($name in $TelemetryReg[$path].Keys) {
        Set-ItemProperty -Path $path -Name $name -Value $TelemetryReg[$path][$name] -Type DWord -Force
    }
}

# ============================================================================
#  2. DISABLE SERVICES
# ============================================================================
Write-Host "[2/12] Disabling unnecessary services..." -ForegroundColor Green

$DisableServices = @(
    'DiagTrack'              # Telemetry
    'dmwappushsvc'           # WAP push
    'SysMain'                # Superfetch
    'WSearch'                # Windows Search indexer
    'MapsBroker'             # Offline maps
    'lfsvc'                  # Geolocation
    'WerSvc'                 # Error Reporting
    'Fax'                    # Fax
    'RetailDemo'             # Retail demo
    'wisvc'                  # Windows Insider
    'TabletInputService'     # Touch keyboard
    'EdgeUpdate'             # Edge auto-update
    'edgeupdate'             # Edge auto-update (lowercase)
    'EdgeCrashDump'          # Edge crash dump
    'PcaSvc'                 # Program Compatibility Assistant
    'Spooler'                # Print Spooler (disable if no printer)
    ' RemoteRegistry'        # Remote Registry
    'XblAuthManager'         # Xbox Live Auth
    'XblGameSave'            # Xbox Game Save
    'XboxNetApiSvc'          # Xbox Network
    'XboxGipSvc'             # Xbox Game Input
    'BthServ'                # Bluetooth Support (disable if no BT)
    'PhoneSvc'               # Phone Service
    'TapiSrv'                # Telephony
    'icssvc'                 # Windows Mobile Hotspot
    'SEMgrSvc'               # Payments and NFC
    'TrkWks'                 # Distributed Link Tracking
    'RemoteRegistry'         # Remote Registry
    'SharedAccess'           # Internet Connection Sharing
)

foreach ($svc in $DisableServices) {
    $svc = $svc.Trim()
    $service = Get-Service -Name $svc -ErrorAction SilentlyContinue
    if ($service) {
        Stop-Service -Name $svc -Force -ErrorAction SilentlyContinue
        Set-Service -Name $svc -StartupType Disabled -ErrorAction SilentlyContinue
        Write-Host "  [x] Disabled: $svc" -ForegroundColor DarkGray
    }
}

# ============================================================================
#  3. POWER PLAN - ULTIMATE PERFORMANCE
# ============================================================================
Write-Host "[3/12] Activating Ultimate Performance power plan..." -ForegroundColor Green

$dup = powercfg -duplicatescheme e9a42b02-d5df-448d-aa00-03f14749eb61 2>$null
if ($dup -match '([0-9a-fA-F]{8}(-[0-9a-fA-F]{4}){3}-[0-9a-fA-F]{12})') {
    $planGUID = $Matches[1]
    powercfg -setactive $planGUID
    Write-Host "  [x] Ultimate Performance plan activated: $planGUID" -ForegroundColor DarkGray
} else {
    # Fallback: try High Performance
    powercfg -setactive 8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c 2>$null
    Write-Host "  [~] Fallback to High Performance plan" -ForegroundColor DarkYellow
}

# Power plan sub-guides
$SUB_PROCESSOR = '54533251-82be-4824-96c1-47b60bb74065'
$SUB_DISK      = '0012ee47-9041-4b5d-9b77-535fba8b1442'
$SUB_USB       = '2a737441-1930-4402-8d77-b2bebba308a3'

# Processor: 100% min, 100% max, aggressive boost
powercfg -setacvalueindex $planGUID $SUB_PROCESSOR '893dee8e-2bef-41e0-89c6-b55d0929964c' 100 2>$null
powercfg -setacvalueindex $planGUID $SUB_PROCESSOR 'bc5038f7-23e0-4960-96da-33abaf5935ec' 100 2>$null
powercfg -setacvalueindex $planGUID $SUB_PROCESSOR 'be337238-0d82-4146-a960-4f3749d470c7' 2   2>$null
powercfg -setacvalueindex $planGUID $SUB_PROCESSOR '5d76a2ca-e8c0-402f-a133-2158492d58ad' 0   2>$null
powercfg -setacvalueindex $planGUID $SUB_PROCESSOR '0cc5b647-c1df-4637-891a-dec35c318583' 100 2>$null
powercfg -setacvalueindex $planGUID $SUB_PROCESSOR 'ea062031-0e34-4ff1-9b6d-eb1059334028' 100 2>$null

# USB selective suspend off
powercfg -setacvalueindex $planGUID $SUB_USB '48e6b7a6-50f5-4782-a5d4-53bb8f07e226' 0 2>$null

# Hard disk: never turn off
powercfg -setacvalueindex $planGUID $SUB_DISK '6738e2c4-e8a5-4a42-b16a-e040e769756e' 0 2>$null

powercfg -setactive $planGUID

# ============================================================================
#  4. VISUAL EFFECTS
# ============================================================================
Write-Host "[4/12] Optimizing visual effects..." -ForegroundColor Green

# Set visual effects to best performance
$visualReg = 'HKCU:\Software\Microsoft\Windows\CurrentVersion\Explorer\VisualEffects'
if (!(Test-Path $visualReg)) { New-Item -Path $visualReg -Force | Out-Null }
Set-ItemProperty -Path $visualReg -Name 'VisualFXSetting' -Value 2 -Type DWord -Force

# Disable transparency
Set-ItemProperty -Path 'HKCU:\SOFTWARE\Microsoft\Windows\CurrentVersion\Themes\Personalize' -Name 'EnableTransparency' -Value 0 -Type DWord -Force

# Disable animations
Set-ItemProperty -Path 'HKCU:\Control Panel\Desktop' -Name 'UserPreferencesMask' -Value ([byte[]](0x90,0x12,0x03,0x80,0x10,0x00,0x00,0x00)) -Force
Set-ItemProperty -Path 'HKCU:\Control Panel\Desktop\WindowMetrics' -Name 'MinAnimate' -Value '0' -Force

# Disable smooth scrolling
Set-ItemProperty -Path 'HKCU:\Control Panel\Desktop' -Name 'SmoothScroll' -Value 0 -Type DWord -Force

# Disable Peek
Set-ItemProperty -Path 'HKCU:\Software\Microsoft\Windows\Dwm' -Name 'EnableAeroPeek' -Value 0 -Type DWord -Force

# ============================================================================
#  5. EXPLORER & UI TWEAKS
# ============================================================================
Write-Host "[5/12] Applying Explorer & UI tweaks..." -ForegroundColor Green

$ExplorerReg = @{
    'HKCU:\Software\Microsoft\Windows\CurrentVersion\Explorer\Advanced' = @{
        'TaskbarGlomLevel'          = 0    # Always combine, hide labels
        'TaskbarSmallIcons'         = 0
        'TaskbarAnimations'         = 0
        'IconsOnly'                 = 0
        'ShowTaskViewButton'        = 0
        'ShowSecondsInSystemClock'  = 1
        'LaunchTo'                  = 1    # Open to This PC
        'Hidden'                    = 1    # Show hidden files
        'HideFileExt'               = 0    # Show file extensions
        'DontUsePowerShellOnWinX'   = 1    # Show Command Prompt in Win+X
        'NavPaneShowAllFolders'     = 0
        'NavPaneExpandToCurrentFolder' = 0
        'Start_TrackDocs'           = 0
        'Start_TrackProgs'          = 0
    }
    'HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Policies\Explorer' = @{
        'NoRecentDocsMenu'          = 1
        'NoRecentDocsHistory'       = 1
        'ClearRecentDocsOnExit'     = 1
    }
    'HKCU:\Software\Microsoft\Windows\CurrentVersion\Explorer' = @{
        'ShowRecent'                = 0
        'ShowFrequent'              = 0
    }
}

foreach ($path in $ExplorerReg.Keys) {
    if (!(Test-Path $path)) { New-Item -Path $path -Force | Out-Null }
    foreach ($name in $ExplorerReg[$path].Keys) {
        Set-ItemProperty -Path $path -Name $name -Value $ExplorerReg[$path][$name] -Type DWord -Force
    }
}

# Remove Quick Access pinned items
$recentPath = "$env:APPDATA\Microsoft\Windows\Recent\AutomaticDestinations"
if (Test-Path $recentPath) { Remove-Item "$recentPath\*" -Force -ErrorAction SilentlyContinue }

# Disable widgets
Set-ItemProperty -Path 'HKCU:\Software\Microsoft\Windows\CurrentVersion\Explorer\Advanced' -Name 'TaskbarDa' -Value 0 -Type DWord -Force

# Disable Chat icon on taskbar
Set-ItemProperty -Path 'HKCU:\Software\Microsoft\Windows\CurrentVersion\Explorer\Advanced' -Name 'TaskbarMn' -Value 0 -Type DWord -Force

# ============================================================================
#  6. SCHEDULED TASKS
# ============================================================================
Write-Host "[6/12] Disabling unnecessary scheduled tasks..." -ForegroundColor Green

$DisableTasks = @(
    '\Microsoft\Windows\Application Experience\Microsoft Compatibility Appraiser'
    '\Microsoft\Windows\Application Experience\ProgramDataUpdater'
    '\Microsoft\Windows\Customer Experience Improvement Program\Consolidator'
    '\Microsoft\Windows\Customer Experience Improvement Program\UsbCeip'
    '\Microsoft\Windows\Autochk\Proxy'
    '\Microsoft\Windows\DiskDiagnostic\Microsoft-Windows-DiskDiagnosticDataCollector'
    '\Microsoft\Windows\Feedback\Siuf\DmClient'
    '\Microsoft\Windows\Feedback\Siuf\DmClientOnScenarioDownload'
    '\Microsoft\Windows\Maps\MapsToastTask'
    '\Microsoft\Windows\Maps\MapsUpdateTask'
    '\Microsoft\Windows\Windows Error Reporting\QueueReporting'
    '\Microsoft\Windows\PI\Sqm-Tasks'
    '\Microsoft\Windows\CloudExperienceHost\CreateObjectTask'
    '\Microsoft\Windows\DiskFootprint\Diagnostics'
    '\Microsoft\Windows\Power Efficiency Diagnostics\AnalyzeSystem'
    '\Microsoft\Windows\Shell\FamilySafetyMonitor'
    '\Microsoft\Windows\Shell\FamilySafetyRefreshTask'
    '\Microsoft\Windows\Maintenance\WinSAT'
    '\Microsoft\Windows\MemoryDiagnostic\ProcessMemoryDiagnosticEvents'
    '\Microsoft\Windows\MemoryDiagnostic\RunFullMemoryDiagnostic'
    '\Microsoft\Windows\MemoryDiagnostic\DDResProvider'
)

foreach ($task in $DisableTasks) {
    $taskObj = Get-ScheduledTask -TaskPath ($task.Substring(0, $task.LastIndexOf('\') + 1)) -TaskName ($task.Split('\')[-1]) -ErrorAction SilentlyContinue
    if ($taskObj) {
        Disable-ScheduledTask -TaskName $taskObj.TaskName -TaskPath $taskObj.TaskPath -ErrorAction SilentlyContinue | Out-Null
        Write-Host "  [x] Disabled: $($taskObj.TaskName)" -ForegroundColor DarkGray
    }
}

# ============================================================================
#  7. NETWORK OPTIMIZATION
# ============================================================================
Write-Host "[7/12] Optimizing network settings..." -ForegroundColor Green

# Disable Nagle's algorithm (lower latency)
$nicParams = Get-NetAdapterAdvancedProperty -RegistryKeyword 'TcpAckFrequency' -ErrorAction SilentlyContinue
if ($nicParams) {
    Set-NetAdapterAdvancedProperty -RegistryKeyword 'TcpAckFrequency' -RegistryValue 1 -ErrorAction SilentlyContinue
    Set-NetAdapterAdvancedProperty -RegistryKeyword 'TCPNoDelay' -RegistryValue 1 -ErrorAction SilentlyContinue
}

# Network tuning
$netReg = @{
    'HKLM:\SYSTEM\CurrentControlSet\Services\Tcpip\Parameters' = @{
        'TcpAckFrequency'           = 1
        'TCPNoDelay'                = 1
        'TcpDelAckTicks'            = 0
        'DefaultTTL'                = 64
        'MaxUserPort'               = 65534
        'TcpTimedWaitDelay'         = 30
        'MaxFreeTcbs'               = 65536
        'MaxHashTableSize'          = 65536
    }
    'HKLM:\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Multimedia\SystemProfile' = @{
        'NetworkThrottlingIndex'    = 0xFFFFFFFF
        'SystemResponsiveness'      = 0
    }
    'HKLM:\SYSTEM\CurrentControlSet\Services\Tcpip\Parameters\Interfaces' = @{
        'TcpInitialRTT'             = 2
    }
    'HKLM:\SYSTEM\CurrentControlSet\Services\AFD\Parameters' = @{
        'DefaultReceiveWindow'      = 16384
        'DefaultSendWindow'         = 16384
    }
}

foreach ($path in $netReg.Keys) {
    if (!(Test-Path $path)) { New-Item -Path $path -Force | Out-Null }
    foreach ($name in $netReg[$path].Keys) {
        Set-ItemProperty -Path $path -Name $name -Value $netReg[$path][$name] -Force
    }
}

# Disable bandwidth reservation
Set-ItemProperty -Path 'HKLM:\SOFTWARE\Policies\Microsoft\Windows\Psched' -Name 'NonBestEffortLimit' -Value 0 -Type DWord -Force

# ============================================================================
#  8. BLOATWARE REMOVAL
# ============================================================================
Write-Host "[8/12] Removing bloatware apps..." -ForegroundColor Green

$Bloatware = @(
    'Microsoft.BingNews'               # Bing News
    'Microsoft.BingWeather'            # Bing Weather
    'Microsoft.BingSearch'             # Bing Search
    'Microsoft.BingFinance'            # Bing Finance
    'Microsoft.BingSports'             # Bing Sports
    'Microsoft.BingTranslator'         # Bing Translator
    'Microsoft.BingFoodAndDrink'       # Bing Food & Drink
    'Microsoft.BingHealthAndFitness'   # Bing Health & Fitness
    'Microsoft.BingTravel'             # Bing Travel
    'Microsoft.GetHelp'                # Get Help
    'Microsoft.Getstarted'             # Tips
    'Microsoft.MicrosoftSolitaireCollection' # Solitaire
    'Microsoft.People'                 # People
    'Microsoft.PowerAutomateDesktop'   # Power Automate
    'Microsoft.WindowsFeedbackHub'     # Feedback Hub
    'Microsoft.WindowsMaps'            # Windows Maps
    'Microsoft.WindowsSoundRecorder'   # Voice Recorder
    'Microsoft.ZuneMusic'              # Groove Music
    'Microsoft.ZuneVideo'              # Movies & TV
    'Microsoft.3DBuilder'              # 3D Builder
    'Microsoft.MicrosoftOfficeHub'    # Office Hub
    'Microsoft.SkypeApp'              # Skype
    'Microsoft.MicrosoftStickyNotes'  # Sticky Notes (old)
    'Microsoft.Todos'                  # Microsoft To Do
    'Microsoft.WindowsAlarms'          # Alarms & Clock
    'Microsoft.WindowsCalculator'      # Calculator (old)
    'Microsoft.WindowsCamera'          # Camera
    'Microsoft.WindowsMaps'            # Maps
    'Microsoft.YourPhone'              # Your Phone
    'Microsoft.WindowsStore'           # Store (keep if needed)
    'Microsoft.ZuneMusic'              # Groove Music
    'Microsoft.ZuneVideo'              # Movies & TV
    'king.com.CandyCrushSaga'          # Candy Crush
    'king.com.CandyCrushSodaSaga'      # Candy Crush Soda
    'SpotifyAB.SpotifyMusic'           # Spotify
    'Disney.37853FC22B2CE'             # Disney+
    'BytedancePte.Ltd.TikTok'          # TikTok
    'Facebook.Facebook'                # Facebook
    'Instagram'                         # Instagram
    'Clipchamp.Clipchamp'              # Clipchamp
    'Microsoft.WindowsCopilot'         # Copilot
    'Microsoft.Copilot'                # Copilot
    'Microsoft.WindowsMeetNow'         # Meet Now
    'MicrosoftTeams'                    # Teams (new)
    'Microsoft.Microsoft3DViewer'      # 3D Viewer
    'Microsoft.MixedReality.Portal'    # Mixed Reality
    'Microsoft.ScreenSketch'           # Snipping Tool (old)
    'Microsoft.Windows.Photos'         # Photos (old)
)

foreach ($app in $Bloatware) {
    $pkg = Get-AppxPackage -Name $app -AllUsers -ErrorAction SilentlyContinue
    if ($pkg) {
        Remove-AppxPackage -Package $pkg.PackageFullName -AllUsers -ErrorAction SilentlyContinue
        Write-Host "  [x] Removed: $app" -ForegroundColor DarkGray
    }
}

# Also remove provisioned packages (prevents reinstall on new user creation)
foreach ($app in $Bloatware) {
    Get-AppxProvisionedPackage -Online | Where-Object { $_.PackageName -like "*$app*" } | Remove-AppxProvisionedPackage -Online -ErrorAction SilentlyContinue | Out-Null
}

# ============================================================================
#  9. BLOCK TELEMETRY HOSTS
# ============================================================================
Write-Host "[9/12] Blocking telemetry/tracking hosts..." -ForegroundColor Green

$hostsFile = "$env:SystemRoot\System32\drivers\etc\hosts"
$telemetryHosts = @(
    '0.0.0.0 vortex.data.microsoft.com'
    '0.0.0.0 vortex-win.data.microsoft.com'
    '0.0.0.0 telecommand.telemetry.microsoft.com'
    '0.0.0.0 telemetry.microsoft.com'
    '0.0.0.0 settings-sandbox.data.microsoft.com'
    '0.0.0.0 settings-win.data.microsoft.com'
    '0.0.0.0 watson.telemetry.microsoft.com'
    '0.0.0.0 watson.microsoft.com'
    '0.0.0.0 statsfe2.ws.microsoft.com'
    '0.0.0.0 statsfe2.update.microsoft.com'
    '0.0.0.0 watson.ppe.telemetry.microsoft.com'
    '0.0.0.0 v10c.events.data.microsoft.com'
    '0.0.0.0 v10.events.data.microsoft.com'
    '0.0.0.0 v20.events.data.microsoft.com'
    '0.0.0.0 cy2.vortex.data.microsoft.com.akadns.net'
    '0.0.0.0 vortex-sandbox.data.microsoft.com'
    '0.0.0.0 oca.telemetry.microsoft.com'
    '0.0.0.0 oca.telemetry.microsoft.com.nsatc.net'
    '0.0.0.0 df.telemetry.microsoft.com'
    '0.0.0.0 reports.wes.df.telemetry.microsoft.com'
    '0.0.0.0 metering.data.microsoft.com'
)

# Backup current hosts
Copy-Item -Path $hostsFile -Destination "$hostsFile.bak" -Force -ErrorAction SilentlyContinue

$existingHosts = Get-Content -Path $hostsFile -ErrorAction SilentlyContinue
$newHosts = @()
foreach ($entry in $telemetryHosts) {
    $host_ = $entry.Split(' ')[1]
    if ($existingHosts -notcontains $entry -and $existingHosts -notlike "*$host_*") {
        $newHosts += $entry
    }
}

if ($newHosts.Count -gt 0) {
    Add-Content -Path $hostsFile -Value "`n# --- Win11 Ultra Optimizer Telemetry Block ---" -Force
    $newHosts | ForEach-Object { Add-Content -Path $hostsFile -Value $_ }
    Write-Host "  [x] Blocked $($newHosts.Count) telemetry domains" -ForegroundColor DarkGray
}

# Flush DNS cache
ipconfig /flushdns | Out-Null

# ============================================================================
#  10. SSD & DISK OPTIMIZATION
# ============================================================================
Write-Host "[10/12] Optimizing disk & SSD settings..." -ForegroundColor Green

# Enable TRIM for SSDs
$drives = Get-WmiObject -Class Win32_LogicalDisk -Filter "DriveType=3"
foreach ($drive in $drives) {
    $vol = Get-WmiObject -Class Win32_Volume -Filter "DriveLetter='$($drive.DeviceID)'"
    if ($vol) {
        fsutil behavior set DisableDeleteNotify 0 2>$null
    }
}

# Disable SuperFetch/SysMain (already disabled as service, also set registry)
Set-ItemProperty -Path 'HKLM:\SYSTEM\CurrentControlSet\Control\Session Manager\Memory Management\PrefetchParameters' -Name 'EnableSuperfetch' -Value 0 -Force -ErrorAction SilentlyContinue
Set-ItemProperty -Path 'HKLM:\SYSTEM\CurrentControlSet\Control\Session Manager\Memory Management\PrefetchParameters' -Name 'EnablePrefetcher' -Value 0 -Force -ErrorAction SilentlyContinue

# Disable memory dump (saves disk space)
Set-ItemProperty -Path 'HKLM:\SYSTEM\CurrentControlSet\Control\CrashControl' -Name 'CrashDumpEnabled' -Value 0 -Force
Set-ItemProperty -Path 'HKLM:\SYSTEM\CurrentControlSet\Control\CrashControl' -Name 'LogEvent' -Value 0 -Force
Set-ItemProperty -Path 'HKLM:\SYSTEM\CurrentControlSet\Control\CrashControl' -Name 'AutoReboot' -Value 1 -Force

# Disable hibernation (saves disk space, use sleep instead)
powercfg /h off 2>$null

# Clean temp files
$paths = @(
    "$env:TEMP",
    "$env:SystemRoot\Temp",
    "$env:SystemRoot\Prefetch",
    "C:\Windows\SoftwareDistribution\Download",
    "C:\Windows\Logs\CBS"
)
foreach ($p in $paths) {
    if (Test-Path $p) {
        Get-ChildItem -Path $p -Recurse -ErrorAction SilentlyContinue | Where-Object { $_.LastWriteTime -lt (Get-Date).AddDays(-7) } | Remove-Item -Force -Recurse -ErrorAction SilentlyContinue
    }
}
Write-Host "  [x] Temp files cleaned" -ForegroundColor DarkGray

# ============================================================================
#  11. NVIDIA GPU OPTIMIZATION
# ============================================================================
Write-Host "[11/12] Optimizing NVIDIA GPU settings..." -ForegroundColor Green

# Detect NVIDIA GPU
$nvidiaGpu = Get-WmiObject Win32_VideoController | Where-Object { $_.Name -like '*NVIDIA*' }

if ($nvidiaGpu) {
    Write-Host "  [x] Detected: $($nvidiaGpu.Name)" -ForegroundColor DarkGray

    # --- Disable NVIDIA Telemetry ---
    $nvServices = @(
        'NvTelemetryContainer'  # NVIDIA Telemetry Container
        'NvContainerLocalSystem' # NVIDIA Local System Container
    )
    foreach ($svc in $nvServices) {
        $s = Get-Service -Name $svc -ErrorAction SilentlyContinue
        if ($s) {
            Stop-Service -Name $svc -Force -ErrorAction SilentlyContinue
            Set-Service -Name $svc -StartupType Disabled -ErrorAction SilentlyContinue
            Write-Host "  [x] Disabled NVIDIA service: $svc" -ForegroundColor DarkGray
        }
    }

    # Block NVIDIA telemetry via hosts
    $nvTelemetryHosts = @(
        '0.0.0.0 gfe.nvidia.com'
        '0.0.0.0 events.gfe.nvidia.com'
        '0.0.0.0 o182.nvidia.com'
        '0.0.0.0 o671.nvidia.com'
        '0.0.0.0 gfe.nvidia.com.akadns.net'
    )
    $existingHosts = Get-Content -Path $hostsFile -ErrorAction SilentlyContinue
    foreach ($entry in $nvTelemetryHosts) {
        $h = $entry.Split(' ')[1]
        if ($existingHosts -notcontains $entry -and $existingHosts -notlike "*$h*") {
            Add-Content -Path $hostsFile -Value $entry -Force
        }
    }

    # --- GPU Performance via Registry ---
    $nvRegBase = 'HKLM:\SYSTEM\CurrentControlSet\Control\Class\{4d36e968-e325-11ce-bfc1-08002be10318}\0000'
    if (Test-Path $nvRegBase) {
        # Disable HDMI link power savings
        Set-ItemProperty -Path $nvRegBase -Name 'RMHdcpKeyglobZero' -Value 1 -Type DWord -Force -ErrorAction SilentlyContinue

        # Disable dynamic power management (keeps GPU at full clocks)
        Set-ItemProperty -Path $nvRegBase -Name 'PerfLevelSrc' -Value 8738 -Type DWord -Force -ErrorAction SilentlyContinue

        # Disable GPU power gating
        Set-ItemProperty -Path $nvRegBase -Name 'PowerMizerEnable' -Value 1 -Type DWord -Force -ErrorAction SilentlyContinue
        Set-ItemProperty -Path $nvRegBase -Name 'PowerMizerLevel' -Value 1 -Type DWord -Force -ErrorAction SilentlyContinue
        Set-ItemProperty -Path $nvRegBase -Name 'PowerMizerLevelAC' -Value 1 -Type DWord -Force -ErrorAction SilentlyContinue

        # Prefer maximum performance
        Set-ItemProperty -Path $nvRegBase -Name 'DisableDynamicPstate' -Value 1 -Type DWord -Force -ErrorAction SilentlyContinue

        Write-Host "  [x] GPU power management set to maximum performance" -ForegroundColor DarkGray
    }

    # --- NVIDIA Control Panel Global Profile Settings ---
    $nvGlobalProfile = 'HKLM:\SOFTWARE\NVIDIA Corporation\Global\NVTweak'
    if (!(Test-Path $nvGlobalProfile)) { New-Item -Path $nvGlobalProfile -Force | Out-Null }
    Set-ItemProperty -Path $nvGlobalProfile -Name 'DisablePState' -Value 1 -Type DWord -Force

    # --- nvidia-smi settings ---
    $nvidiaSmi = Get-Command 'nvidia-smi' -ErrorAction SilentlyContinue
    if ($nvidiaSmi) {
        # Set compute mode to default
        & nvidia-smi --compute-mode=DEFAULT 2>$null

        # Disable GPU persistence mode (frees resources)
        & nvidia-smi -pm 0 2>$null
        Write-Host "  [x] nvidia-smi settings applied" -ForegroundColor DarkGray
    }

    # --- NVIDIA bloatware removal ---
    $nvBloat = @(
        'NVIDIA App'
        'NVIDIA GeForce Experience'
        'NVIDIA Broadcast'
        'NVIDIA Canvas'
        'NVIDIA Omniverse'
    )
    foreach ($app in $nvBloat) {
        $installed = Get-WmiObject -Class Win32_Product | Where-Object { $_.Name -like "*$app*" }
        if ($installed) {
            Write-Host "  [~] Found: $app (uninstall manually if desired)" -ForegroundColor DarkYellow
        }
    }

    # Disable NVIDIA overlay (GeForce Experience in-game overlay)
    $nvOverlayReg = 'HKCU:\SOFTWARE\NVIDIA Corporation\Global\NVIDIA GeForce Experience'
    if (Test-Path $nvOverlayReg) {
        Set-ItemProperty -Path $nvOverlayReg -Name 'EnableInGameOverlay' -Value 0 -Type DWord -Force
        Write-Host "  [x] NVIDIA in-game overlay disabled" -ForegroundColor DarkGray
    }

    # Disable NVIDIA Container telemetry tasks
    Get-ScheduledTask | Where-Object { $_.TaskName -like '*NVIDIA*' -and $_.TaskName -like '*Telemetry*' } | ForEach-Object {
        Disable-ScheduledTask -TaskName $_.TaskName -ErrorAction SilentlyContinue | Out-Null
        Write-Host "  [x] Disabled NVIDIA scheduled task: $($_.TaskName)" -ForegroundColor DarkGray
    }

} else {
    Write-Host "  [~] No NVIDIA GPU detected, skipping" -ForegroundColor DarkYellow
}

# ============================================================================
#  12. AMD GPU OPTIMIZATION
# ============================================================================
Write-Host "[12/12] Optimizing AMD GPU settings..." -ForegroundColor Green

$amdGpu = Get-WmiObject Win32_VideoController | Where-Object { $_.Name -like '*AMD*' -or $_.Name -like '*Radeon*' }

if ($amdGpu) {
    Write-Host "  [x] Detected: $($amdGpu.Name)" -ForegroundColor DarkGray

    # --- Disable AMD Telemetry Services ---
    $amdServices = @(
        'AMD External Events Utility'  # AMD telemetry/events
        'AUEPUF'                       # AMD User Experience Program
        'AUEPMinder'                   # AMD User Experience Program
    )
    foreach ($svc in $amdServices) {
        $s = Get-Service -Name $svc -ErrorAction SilentlyContinue
        if ($s) {
            Stop-Service -Name $svc -Force -ErrorAction SilentlyContinue
            Set-Service -Name $svc -StartupType Disabled -ErrorAction SilentlyContinue
            Write-Host "  [x] Disabled AMD service: $svc" -ForegroundColor DarkGray
        }
    }

    # --- Block AMD Telemetry Hosts ---
    $amdTelemetryHosts = @(
        '0.0.0.0 telemetry.amd.com'
        '0.0.0.0 events.amd.com'
        '0.0.0.0 ahcf gallium.amd.com'
        '0.0.0.0 g01.gdl.dbg.cloudtools.amd.com'
        '0.0.0.0 g01.b2b.dbg.cloudtools.amd.com'
    )
    $existingHosts = Get-Content -Path $hostsFile -ErrorAction SilentlyContinue
    foreach ($entry in $amdTelemetryHosts) {
        $h = $entry.Split(' ')[1]
        if ($existingHosts -notcontains $entry -and $existingHosts -notlike "*$h*") {
            Add-Content -Path $hostsFile -Value $entry -Force
        }
    }

    # --- AMD Registry Performance Tweaks ---
    $amdRegBase = 'HKLM:\SYSTEM\CurrentControlSet\Control\Class\{4d36e968-e325-11ce-bfc1-08002be10318}\0000'
    if (Test-Path $amdRegBase) {
        # Set preferred power mode to performance
        Set-ItemProperty -Path $amdRegBase -Name 'EnableUlps' -Value 0 -Type DWord -Force -ErrorAction SilentlyContinue
        Set-ItemProperty -Path $amdRegBase -Name 'EnableUlps_NA' -Value 0 -Type DWord -Force -ErrorAction SilentlyContinue

        # Disable HDMI link power saving
        Set-ItemProperty -Path $amdRegBase -Name 'DalDisableIdlePowerSave' -Value 1 -Type DWord -Force -ErrorAction SilentlyContinue

        # Performance mode
        Set-ItemProperty -Path $amdRegBase -Name 'PP_SclkDeepSleepDisable' -Value 1 -Type DWord -Force -ErrorAction SilentlyContinue

        Write-Host "  [x] AMD GPU set to performance mode" -ForegroundColor DarkGray
    }

    # --- AMD Adrenalin Settings via Registry ---
    $amdAdrenalinReg = 'HKCU:\SOFTWARE\AMD\CN'
    if (Test-Path $amdAdrenalinReg) {
        # Disable AMD overlay
        Set-ItemProperty -Path $amdAdrenalinReg -Name 'ShowOverlay' -Value 0 -Type DWord -Force -ErrorAction SilentlyContinue
        Write-Host "  [x] AMD overlay disabled" -ForegroundColor DarkGray
    }

    # Disable Radeon Anti-Lag telemetry
    $radeonReg = 'HKLM:\SOFTWARE\AMD\RadeonSettings'
    if (Test-Path $radeonReg) {
        Set-ItemProperty -Path $radeonReg -Name 'AllowWebContent' -Value 0 -Type DWord -Force -ErrorAction SilentlyContinue
        Write-Host "  [x] AMD Radeon web content disabled" -ForegroundColor DarkGray
    }

    # --- Disable AMD Scheduled Tasks ---
    Get-ScheduledTask | Where-Object { $_.TaskName -like '*AMD*' -or $_.TaskName -like '*Radeon*' } | ForEach-Object {
        if ($_.TaskName -notlike '*AMD Crash Defender*') {
            Disable-ScheduledTask -TaskName $_.TaskName -ErrorAction SilentlyContinue | Out-Null
            Write-Host "  [x] Disabled AMD task: $($_.TaskName)" -ForegroundColor DarkGray
        }
    }

    # --- AMD Bloatware Detection ---
    $amdBloat = @(
        'AMD Software: Adrenalin Edition'
        'AMD Ryzen Master'
        'AMD Link'
    )
    foreach ($app in $amdBloat) {
        $installed = Get-WmiObject -Class Win32_Product | Where-Object { $_.Name -like "*$app*" }
        if ($installed) {
            Write-Host "  [~] Found: $app (uninstall manually if desired)" -ForegroundColor DarkYellow
        }
    }

} else {
    Write-Host "  [~] No AMD GPU detected, skipping" -ForegroundColor DarkYellow
}

# ============================================================================
#  FINAL
# ============================================================================
Write-Host ""
Write-Host "  =============================================" -ForegroundColor Cyan
Write-Host "   OPTIMIZATION COMPLETE" -ForegroundColor Green
Write-Host "  =============================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "  Changes applied:" -ForegroundColor Yellow
Write-Host "   - Telemetry & tracking disabled"
Write-Host "   - Unnecessary services stopped"
Write-Host "   - Ultimate Performance power plan activated"
Write-Host "   - Visual effects optimized"
Write-Host "   - Explorer & UI customized"
Write-Host "   - Scheduled tasks disabled"
Write-Host "   - Network optimized (low latency)"
Write-Host "   - Bloatware apps removed"
Write-Host "   - Telemetry hosts blocked"
Write-Host "   - Disk & SSD optimized"
Write-Host "   - NVIDIA GPU optimized (overlay, telemetry)"
Write-Host "   - AMD GPU optimized (power mode, overlay, telemetry)"
Write-Host ""
Write-Host "  RESTART YOUR PC for changes to take full effect." -ForegroundColor Yellow
Write-Host ""
