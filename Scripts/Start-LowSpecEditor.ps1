#requires -Version 5.1
[CmdletBinding()]
param(
    [string]$StartDirectory,
    [string]$EngineRoot = $env:UE_ENGINE_ROOT,
    [ValidatePattern('^/[A-Za-z0-9_/.]+$')]
    [string]$Map = '/Engine/Maps/Entry',
    [switch]$SampleMap,
    [ValidateRange(1, 128)][int]$ShaderWorkers = 4,
    [ValidateRange(1, 240)][int]$FPS = 30,
    [ValidateRange(10, 100)][int]$Screen = 50,
    [ValidateRange(64, 8192)][int]$PoolMB = 256,
    [switch]$Mute,
    [switch]$KeepSourceControl,
    [switch]$EnableLiveCoding,
    [switch]$DetectOnly
)

$ErrorActionPreference = 'Stop'

function Find-LowSpecProject([string]$Directory) {
    $cursor = Get-Item -LiteralPath $Directory
    if (-not $cursor.PSIsContainer) { throw 'StartDirectory must be a directory.' }
    while ($null -ne $cursor) {
        $projects = @(Get-ChildItem -LiteralPath $cursor.FullName -Filter '*.uproject' -File)
        if ($projects.Count -gt 1) { throw "Multiple .uproject files in $($cursor.FullName)." }
        if ($projects.Count -eq 1) { return $projects[0] }
        $cursor = $cursor.Parent
    }
    throw 'No .uproject found above the feature directory.'
}

function Get-LowSpecEngine([string]$Root, [string]$Origin) {
    if ([string]::IsNullOrWhiteSpace($Root)) { return }
    try {
        $fullRoot = [IO.Path]::GetFullPath([Environment]::ExpandEnvironmentVariables($Root))
        $editor = Join-Path $fullRoot 'Engine\Binaries\Win64\UnrealEditor.exe'
        if (-not (Test-Path -LiteralPath $editor -PathType Leaf)) { return }
        $versionFile = Join-Path $fullRoot 'Engine\Build\Build.version'
        $data = Get-Content -LiteralPath $versionFile -Raw | ConvertFrom-Json
        $version = [version]::new([int]$data.MajorVersion, [int]$data.MinorVersion, [int]$data.PatchVersion)
        return [pscustomobject]@{ Root = $fullRoot; Editor = $editor; Version = $version; Origin = $Origin }
    } catch { Write-Verbose "Engine candidate unavailable: $Root" }
}

function Get-LowSpecLauncherEngines {
    $manifest = Join-Path $env:ProgramData 'Epic\UnrealEngineLauncher\LauncherInstalled.dat'
    if (Test-Path -LiteralPath $manifest -PathType Leaf) {
        try {
            $installed = Get-Content -LiteralPath $manifest -Raw | ConvertFrom-Json
            foreach ($entry in $installed.InstallationList) {
                if ($entry.AppName -match '^UE_\d+\.\d+') {
                    Get-LowSpecEngine $entry.InstallLocation 'Epic Launcher manifest'
                }
            }
        } catch { Write-Warning "Could not read engine installations: $manifest" }
    }
    foreach ($view in @([Microsoft.Win32.RegistryView]::Registry64, [Microsoft.Win32.RegistryView]::Registry32)) {
        $base = $null
        $key = $null
        try {
            $base = [Microsoft.Win32.RegistryKey]::OpenBaseKey([Microsoft.Win32.RegistryHive]::LocalMachine, $view)
            $key = $base.OpenSubKey('SOFTWARE\EpicGames\Unreal Engine')
            if ($null -ne $key) {
                foreach ($name in $key.GetSubKeyNames()) {
                    $entry = $key.OpenSubKey($name)
                    try {
                        if ($null -ne $entry) {
                            Get-LowSpecEngine ([string]$entry.GetValue('InstalledDirectory')) "Launcher registry $name"
                        }
                    } finally { if ($null -ne $entry) { $entry.Dispose() } }
                }
            }
        } finally {
            if ($null -ne $key) { $key.Dispose() }
            if ($null -ne $base) { $base.Dispose() }
        }
    }
}

function Find-LowSpecAssociatedEngine([string]$Association, [IO.DirectoryInfo]$ProjectDirectory) {
    if ([string]::IsNullOrWhiteSpace($Association)) {
        $cursor = $ProjectDirectory
        while ($null -ne $cursor) {
            $engine = Get-LowSpecEngine $cursor.FullName 'Source engine containing project'
            if ($engine) { return $engine }
            $cursor = $cursor.Parent
        }
        return
    }
    $key = [Microsoft.Win32.Registry]::CurrentUser.OpenSubKey('SOFTWARE\Epic Games\Unreal Engine\Builds')
    try {
        if ($null -ne $key) {
            foreach ($name in $key.GetValueNames()) {
                if ($name.Trim('{}') -eq $Association.Trim('{}')) {
                    $engine = Get-LowSpecEngine ([string]$key.GetValue($name)) 'Registered project EngineAssociation'
                    if ($engine) { return $engine }
                }
            }
        }
    } finally { if ($null -ne $key) { $key.Dispose() } }
    if ($Association -match '^\d+\.\d+(\.\d+)?$') {
        $requested = [version]$Association
        $matches = @(Get-LowSpecLauncherEngines | Where-Object {
            $_.Version.Major -eq $requested.Major -and $_.Version.Minor -eq $requested.Minor -and
            ($requested.Build -lt 0 -or $_.Version.Build -eq $requested.Build)
        } | Sort-Object Version -Descending)
        if ($matches.Count -gt 0) { return $matches[0] }
    } elseif ($Association.Contains('\') -or $Association.Contains('/')) {
        $path = $Association
        if (-not [IO.Path]::IsPathRooted($path)) { $path = Join-Path $ProjectDirectory.FullName $path }
        return Get-LowSpecEngine $path 'Project EngineAssociation path'
    }
}

try {
    if ([string]::IsNullOrWhiteSpace($StartDirectory)) { $StartDirectory = Split-Path -Parent $PSScriptRoot }
    $project = Find-LowSpecProject $StartDirectory
    $descriptor = Get-Content -LiteralPath $project.FullName -Raw | ConvertFrom-Json
    if ([string]::IsNullOrWhiteSpace($EngineRoot)) {
        $engine = Find-LowSpecAssociatedEngine ([string]$descriptor.EngineAssociation) $project.Directory
    } else {
        $engine = Get-LowSpecEngine $EngineRoot 'Explicit EngineRoot / UE_ENGINE_ROOT'
    }
    if (-not $engine) {
        throw 'Associated Windows engine not found. Pass -EngineRoot "C:\Program Files\Epic Games\UE_5.8" or set UE_ENGINE_ROOT.'
    }
    $optimizer = Join-Path $PSScriptRoot 'optimize_low_spec.py'
    if (-not (Test-Path -LiteralPath $optimizer -PathType Leaf)) { throw "Missing optimizer: $optimizer" }
    # Reuse the engine's isolated interpreter. No system Python/plugin setup needed.
    $python = Join-Path $engine.Root 'Engine\Binaries\ThirdParty\Python3\Win64\python.exe'
    if (-not (Test-Path -LiteralPath $python -PathType Leaf)) {
        throw "Engine-bundled Python missing: $python. Install the engine's Python components."
    }
    if ($SampleMap) { $Map = '/VRSimulate/Maps/L_ParcelSorting' }

    $launchArguments = @($project.FullName, $Map, '-NoSplash', '-NoHMD')
    if (-not $KeepSourceControl) { $launchArguments += '-SCCProvider=None' }
    if (-not $EnableLiveCoding) { $launchArguments += '-LiveCoding=False' }
    if ($Mute) { $launchArguments += '-NoSound' }
    # One argument per config category, with comma-separated section/key pairs.
    # Command-line overrides take effect before maps and asset tabs are loaded.
    $loading = '[/Script/UnrealEd.EditorLoadingSavingSettings]'
    $liveCoding = '[/Script/LiveCoding.LiveCodingSettings]'
    $projectOverrides = @(
        "${loading}:RestoreOpenAssetTabsOnRestart=NeverRestore",
        "${loading}:LoadLevelAtStartup=ProjectDefault",
        "${loading}:bMonitorContentDirectories=False"
    )
    if (-not $EnableLiveCoding) {
        $projectOverrides += @(
            "${liveCoding}:bEnabled=False",
            "${liveCoding}:Startup=Manual",
            "${liveCoding}:bPreloadEngineModules=False",
            "${liveCoding}:bPreloadEnginePluginModules=False",
            "${liveCoding}:bPreloadProjectModules=False",
            "${liveCoding}:bPreloadProjectPluginModules=False"
        )
    }
    $launchArguments += '-ini:EditorPerProjectUserSettings:' + ($projectOverrides -join ',')
    $performance = '[/Script/UnrealEd.EditorPerformanceSettings]'
    $editorOverrides = @(
        "${performance}:bThrottleCPUWhenNotForeground=True",
        "${performance}:bMonitorEditorPerformance=False",
        "${performance}:bEnableEditorPeformanceTool=False",
        "${performance}:bShowFrameRateAndMemory=True",
        "${performance}:bDisplayHighDPIViewports=False",
        "${performance}:bOverrideManualScreenPercentage=True",
        "${performance}:ManualScreenPercentage=$Screen",
        "${performance}:RealtimeScreenPercentageMode=Manual",
        "${performance}:NonRealtimeScreenPercentageMode=Manual",
        '[/Script/UnrealEd.ContentBrowserSettings]:RealTimeThumbnails=False'
    )
    $launchArguments += '-ini:EditorSettings:' + ($editorOverrides -join ',')
    $argumentLine = ($launchArguments | ForEach-Object { '"{0}"' -f $_ }) -join ' '

    Write-Host "Project        : $($project.FullName)"
    Write-Host "Engine         : $($engine.Root) ($($engine.Version))"
    Write-Host "Selected by    : $($engine.Origin)"
    Write-Host "Startup map    : $Map"
    Write-Host "Profile        : Low (Effects Medium) / $FPS FPS / $Screen percent / $PoolMB MiB"
    Write-Host "Shader workers : up to $ShaderWorkers requested (engine may adjust)"
    Write-Host "Python         : $python"
    Write-Host ('Launch         : "{0}" {1}' -f $engine.Editor, $argumentLine)
    if ($DetectOnly) {
        Write-Host 'Detection only. No config changes, build, or editor launch.'
        exit 0
    }

    # Refuse to edit settings underneath an already running instance.
    $editors = @(Get-CimInstance Win32_Process -Filter "Name = 'UnrealEditor.exe' OR Name = 'UnrealEditor-Cmd.exe'")
    foreach ($running in $editors) {
        if ([string]::IsNullOrWhiteSpace($running.CommandLine)) {
            throw 'A running editor command line is inaccessible. Close Unreal Editor before using this launcher.'
        }
        if ($running.CommandLine.IndexOf($project.Name, [StringComparison]::OrdinalIgnoreCase) -ge 0) {
            throw "This project is already open (PID $($running.ProcessId)). Close it before applying startup settings."
        }
    }
    Write-Host 'Applying backed-up project profile before editor startup...'
    & $python '-I' $optimizer 'apply' '--project' $project.FullName '--fps' $FPS '--screen' $Screen '--pool-mb' $PoolMB '--shader-workers' $ShaderWorkers
    if ($LASTEXITCODE -ne 0) { throw "Python optimizer failed ($LASTEXITCODE). Editor was not launched." }
    Start-Process -FilePath $engine.Editor -ArgumentList $argumentLine -WorkingDirectory $project.DirectoryName
    Write-Host 'Editor launch requested. No pre-launch C++ build was run.' -ForegroundColor Green
    exit 0
} catch {
    Write-Host $_.Exception.Message -ForegroundColor Red
    exit 1
}
