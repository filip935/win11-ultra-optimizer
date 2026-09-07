# Win11 Ultra Optimizer

A comprehensive PowerShell script that optimizes Windows 11 for maximum performance, privacy, and minimal bloatware.

## What It Does

| Step | Category | Changes |
|------|----------|---------|
| 1 | **Telemetry & Privacy** | Disables telemetry, ads ID, Cortana, Bing search, activity history, content delivery |
| 2 | **Services** | Stops 30+ unnecessary services (Superfetch, Search Indexer, Xbox, Geolocation, etc.) |
| 3 | **Power Plan** | Activates Ultimate Performance with 100% CPU, no USB suspend |
| 4 | **Visual Effects** | Best performance mode, disables transparency, animations, Peek |
| 5 | **Explorer & UI** | Shows file extensions, hidden files, hides widgets/chat, seconds in clock |
| 6 | **Scheduled Tasks** | Disables 20+ telemetry/diagnostic tasks |
| 7 | **Network** | Disables Nagle's algorithm, optimizes TCP/IP for low latency |
| 8 | **Bloatware** | Removes 50+ pre-installed apps (Cortana, Teams, Candy Crush, TikTok, etc.) |
| 9 | **Hosts Block** | Blocks 20+ Microsoft telemetry/tracking domains |
| 10 | **Disk & SSD** | Enables TRIM, disables SuperFetch, cleans temp files |
| 11 | **NVIDIA GPU** | Disables telemetry, overlay, sets max performance mode via registry |
| 12 | **AMD GPU** | Disables telemetry, overlay, ULPS, sets performance mode |

## Requirements

- Windows 11 (any edition, works great on IoT Enterprise LTSC)
- PowerShell 5.1+
- Administrator privileges

## Usage

Open PowerShell **as Administrator** and run:

```powershell
Set-ExecutionPolicy Unrestricted -Scope CurrentUser -Force
cd "C:\path\to\script"
.\Win11-UltraOptimize.ps1
```

To preview changes without applying:

```powershell
.\Win11-UltraOptimize.ps1 -WhatIf
```

## Safety

- A **System Restore Point** is created before changes (if System Restore is enabled)
- Hosts file is backed up to `hosts.bak`
- All changes are reversible (see Recovery below)

## Recovery

| Change | How to Undo |
|--------|-------------|
| Registry tweaks | Restore from `hosts.bak` or use System Restore |
| Disabled services | `Set-Service -Name <svc> -StartupType Manual` |
| Removed apps | Reinstall from Microsoft Store |
| Hosts file | Copy `hosts.bak` back to `hosts` |
| GPU settings | GPU settings revert on driver update or reinstall |

## GPU Optimization Details

### NVIDIA
- Disables `NvTelemetryContainer` and `NvContainerLocalSystem` services
- Blocks NVIDIA telemetry hosts
- Sets PowerMizer to maximum performance via registry
- Disables in-game overlay
- Disables NVIDIA scheduled telemetry tasks

### AMD
- Disables `AMD External Events Utility` and `AUEPUF` services
- Blocks AMD telemetry hosts
- Disables ULPS (Ultra Low Power State) for lower latency
- Disables HDMI link power saving
- Disables AMD overlay and web content
- Disables AMD scheduled tasks

## License

MIT
