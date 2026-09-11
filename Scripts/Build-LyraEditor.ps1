#requires -Version 5.1
[CmdletBinding()]
param(
    [string]$StartDirectory,
    [string]$EngineRoot = $env:UE_ENGINE_ROOT,
    [switch]$DetectOnly,
    [switch]$NoLaunch
)

$ErrorActionPreference = 'Stop'

function Find-Project([string]$Directory) {
    $cursor = Get-Item -LiteralPath $Directory
    if (-not $cursor.PSIsContainer) { throw 'StartDirectory must be a directory.' }
    while ($null -ne $cursor) {
        $projects = @(Get-ChildItem -LiteralPath $cursor.FullName -Filter '*.uproject' -File)
        if ($projects.Count -gt 1) {
            throw "Multiple projects in $($cursor.FullName). Keep one .uproject in the nearest project directory."
        }
        if ($projects.Count -eq 1) { return $projects[0] }
        $cursor = $cursor.Parent
    }
    throw "No .uproject found in $Directory or its parents."
}

function Get-Engine([string]$Root, [string]$Origin) {
    if ([string]::IsNullOrWhiteSpace($Root)) { return }
    try {
        $fullRoot = [IO.Path]::GetFullPath([Environment]::ExpandEnvironmentVariables($Root))
        if (-not (Test-Path -LiteralPath (Join-Path $fullRoot 'Engine\Build\BatchFiles\Build.bat') -PathType Leaf)) { return }
        $versionFile = Join-Path $fullRoot 'Engine\Build\Build.version'
        $data = Get-Content -LiteralPath $versionFile -Raw | ConvertFrom-Json
        $version = [version]::new([int]$data.MajorVersion, [int]$data.MinorVersion, [int]$data.PatchVersion)
        return [pscustomobject]@{ Root = $fullRoot; Version = $version; Origin = $Origin }
    } catch {
        Write-Verbose "Engine candidate unavailable: $Root"
    }
}

function Get-LauncherEngines {
    # Launcher manifest and both registry views support custom installation directories.
    $manifest = Join-Path $env:ProgramData 'Epic\UnrealEngineLauncher\LauncherInstalled.dat'
    if (Test-Path -LiteralPath $manifest) {
        try {
            $installed = Get-Content -LiteralPath $manifest -Raw | ConvertFrom-Json
            foreach ($entry in $installed.InstallationList) {
                if ($entry.AppName -match '^UE_\d+\.\d+') {
                    Get-Engine $entry.InstallLocation "Launcher $($entry.AppName)"
                }
            }
        } catch { Write-Warning "Launcher manifest could not be read: $manifest" }
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
                        if ($null -ne $entry) { Get-Engine ([string]$entry.GetValue('InstalledDirectory')) "Launcher registry $name" }
                    } finally { if ($null -ne $entry) { $entry.Dispose() } }
                }
            }
        } finally {
            if ($null -ne $key) { $key.Dispose() }
            if ($null -ne $base) { $base.Dispose() }
        }
    }
}

function Find-AssociatedEngine([string]$Association, [IO.DirectoryInfo]$ProjectDirectory, [object[]]$LauncherEngines) {
    if (-not [string]::IsNullOrWhiteSpace($Association)) {
        $key = [Microsoft.Win32.Registry]::CurrentUser.OpenSubKey('SOFTWARE\Epic Games\Unreal Engine\Builds')
        try {
            if ($null -ne $key) {
                foreach ($name in $key.GetValueNames()) {
                    if ($name.Trim('{}') -eq $Association.Trim('{}')) {
                        $engine = Get-Engine ([string]$key.GetValue($name)) 'Project EngineAssociation (registered build)'
                        if ($engine) { return $engine }
                    }
                }
            }
        } finally { if ($null -ne $key) { $key.Dispose() } }

        # Numeric associations identify a launcher major/minor release.
        if ($Association -match '^\d+\.\d+(\.\d+)?$') {
            $requested = [version]$Association
            $matches = @($LauncherEngines | Where-Object {
                $_.Version.Major -eq $requested.Major -and $_.Version.Minor -eq $requested.Minor -and
                ($requested.Build -lt 0 -or $_.Version.Build -eq $requested.Build)
            } | Sort-Object Version -Descending)
            if ($matches.Count -gt 0) { return $matches[0] }
        }
        if ($Association.Contains('\') -or $Association.Contains('/')) {
            $path = $Association
            if (-not [IO.Path]::IsPathRooted($path)) { $path = Join-Path $ProjectDirectory.FullName $path }
            $engine = Get-Engine $path 'Project EngineAssociation (path)'
            if ($engine) { return $engine }
        }
    } else {
        # Empty associations support projects inside a source-engine checkout.
        $cursor = $ProjectDirectory
        while ($null -ne $cursor) {
            $engine = Get-Engine $cursor.FullName 'Engine containing the project'
            if ($engine) { return $engine }
            $cursor = $cursor.Parent
        }
    }
}

try {
    if ([string]::IsNullOrWhiteSpace($StartDirectory)) {
        $StartDirectory = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
    }
    $project = Find-Project $StartDirectory
    $descriptor = Get-Content -LiteralPath $project.FullName -Raw | ConvertFrom-Json
    $feature = Join-Path $project.DirectoryName 'Plugins\GameFeatures\VRSimulate\VRSimulate.uplugin'
    if (-not (Test-Path -LiteralPath $feature -PathType Leaf)) {
        throw "VRSimulate is missing from the detected project: $($project.FullName)"
    }
    $target = 'LyraEditor'
    if (-not (Test-Path -LiteralPath (Join-Path $project.DirectoryName "Source\$target.Target.cs") -PathType Leaf)) {
        throw "LyraEditor.Target.cs is missing from $($project.DirectoryName)."
    }

    Write-Host "Project     : $($project.FullName)"
    Write-Host "Association : $($descriptor.EngineAssociation)"
    if (-not [string]::IsNullOrWhiteSpace($EngineRoot)) {
        $engine = Get-Engine $EngineRoot 'Explicit EngineRoot / UE_ENGINE_ROOT'
        if (-not $engine) { throw "Invalid engine override: $EngineRoot" }
    } else {
        $launcher = @(Get-LauncherEngines | Sort-Object Version -Descending)
        $engine = Find-AssociatedEngine ([string]$descriptor.EngineAssociation) $project.Directory $launcher
        if (-not $engine -and $launcher.Count -gt 0) {
            $engine = $launcher[0]
            Write-Warning 'Associated engine unavailable. Using the newest locally installed launcher engine.'
            Write-Warning 'Fallback does not guarantee source compatibility. EngineAssociation and Target.cs will remain unchanged.'
        }
        if (-not $engine -and -not $DetectOnly) {
            Write-Warning 'No usable registered or launcher engine was found.'
            $manualRoot = Read-Host 'Engine root folder (contains Engine; blank cancels)'
            $engine = Get-Engine ($manualRoot.Trim().Trim('"')) 'Manual engine folder'
        }
        if (-not $engine) {
            throw 'No usable associated or launcher engine found. Register the source engine with UnrealVersionSelector, install a launcher engine, or set UE_ENGINE_ROOT.'
        }
    }

    $build = Join-Path $engine.Root 'Engine\Build\BatchFiles\Build.bat'
    $editor = Join-Path $engine.Root 'Engine\Binaries\Win64\UnrealEditor.exe'
    # Unreal's Build.bat uses delayed expansion; these path characters are unsupported.
    if (($project.FullName + $engine.Root) -match '[!%"]') {
        throw 'Project and engine paths must not contain !, %, or double-quote characters.'
    }
    $buildArguments = @($target, 'Win64', 'Development', "-Project=$($project.FullName)",
        '-EnablePlugin=VRSimulate', '-WaitMutex', '-NoHotReloadFromIDE')

    Write-Host "Engine      : $($engine.Root)"
    Write-Host "Version     : $($engine.Version)"
    Write-Host "Selected by : $($engine.Origin)"
    Write-Host "Build       : $target / Win64 / Development + VRSimulate"
    Write-Host ('Command     : "{0}" {1}' -f $build, (($buildArguments | ForEach-Object { '"{0}"' -f $_ }) -join ' '))
    if ($DetectOnly) {
        Write-Host 'Detection only. No build or editor launch.'
        exit 0
    }

    Write-Host ''
    Write-Host 'Close this project in Unreal Editor before building.'
    Push-Location -LiteralPath $project.DirectoryName
    try {
        & $build @buildArguments
        $buildExit = $LASTEXITCODE
    } finally { Pop-Location }
    if ($buildExit -ne 0) {
        Write-Host "Build failed ($buildExit). Editor was not launched." -ForegroundColor Red
        exit $buildExit
    }
    Write-Host 'Editor build succeeded.' -ForegroundColor Green
    if (-not $NoLaunch) {
        if (-not (Test-Path -LiteralPath $editor -PathType Leaf)) { throw "Editor executable missing after build: $editor" }
        Start-Process -FilePath $editor -ArgumentList ('"{0}"' -f $project.FullName) -WorkingDirectory $project.DirectoryName
        Write-Host 'Editor launch requested.'
    }
    exit 0
} catch {
    Write-Host ''
    Write-Host $_.Exception.Message -ForegroundColor Red
    exit 1
}
