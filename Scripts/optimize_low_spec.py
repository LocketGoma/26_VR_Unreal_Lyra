"""Portable, reversible low-spec profile for an Unreal project (Python 3.9+).

Editor: Tools > Execute Python Script > this file (applies to the open project).
CLI: python optimize_low_spec.py preview|apply|restore [--project PATH]
No external packages are required. Importing this module does not apply settings.
"""

import argparse
import codecs
from datetime import datetime, timezone
import difflib
import json
import os
from pathlib import Path
import re
import sys
import tempfile

try:
    import unreal
except ImportError:
    unreal = None


GROUPS = (
    "ViewDistanceQuality", "AntiAliasingQuality", "ShadowQuality",
    "GlobalIlluminationQuality", "ReflectionQuality", "PostProcessQuality",
    "TextureQuality", "FoliageQuality", "ShadingQuality",
    "LandscapeQuality",
)
BACKUP_DIR = Path("Saved/LowSpecOptimizer")
EFFECTS_QUALITY = 1  # Medium; do not inherit Low/Epic from an older profile.
RENDERER = {
    "r.RayTracing": "False",
    "r.Lumen.HardwareRayTracing": "False",
    "r.DynamicGlobalIlluminationMethod": "0",
    "r.ReflectionMethod": "0",
    "r.Shadow.Virtual.Enable": "0",
    "r.AntiAliasingMethod": "0",
    "r.DefaultFeature.MotionBlur": "False",
    "r.DefaultFeature.Bloom": "False",
    "r.DefaultFeature.AmbientOcclusion": "False",
}
LOADING = {
    "RestoreOpenAssetTabsOnRestart": "NeverRestore",
    "bMonitorContentDirectories": "False",
}
MATERIAL = {
    "bRealtimeMaterialViewport": "False",
    "bRealtimeExpressionViewport": "False",
    "bLivePreviewUpdate": "False",
    "bAlwaysRefreshAllPreviews": "False",
}


def log(message):
    if unreal is not None:
        unreal.log("[LowSpec] " + str(message))
    else:
        print(message)


def find_project(project=None):
    if project:
        start = Path(project).expanduser().resolve()
    elif unreal is not None:
        start = Path(unreal.Paths.convert_relative_path_to_full(
            unreal.Paths.get_project_file_path())).resolve()
    else:
        start = Path(__file__).resolve().parent
    if start.is_file():
        if start.suffix.lower() != ".uproject":
            raise ValueError("--project must be a .uproject file or its directory.")
        return start
    for folder in (start, *start.parents):
        projects = list(folder.glob("*.uproject"))
        if len(projects) > 1:
            raise ValueError("Multiple .uproject files; specify --project explicitly.")
        if projects:
            return projects[0].resolve()
    raise ValueError("No .uproject found. Specify --project PATH.")


def platform_name():
    return {"win32": "Windows", "darwin": "Mac", "linux": "Linux"}.get(sys.platform, "Linux")


def original_scalar(project, candidates, sections, key, fallback=None):
    """Read the first baseline, not an old optimizer's already-reduced values."""
    root = project.parent
    backup = root / BACKUP_DIR
    manifest_path = backup / "manifest.json"
    manifest = json.loads(manifest_path.read_bytes()) if manifest_path.exists() else None
    for name in candidates:
        if manifest is not None:
            if not manifest["files"].get(name, False):
                continue
            data = (backup / "original" / name).read_bytes()
        else:
            data = read_optional(root / name)
        if data is None:
            continue
        encoding = "utf-16" if data.startswith((codecs.BOM_UTF16_LE, codecs.BOM_UTF16_BE)) else "utf-8-sig"
        section = ""
        found = {}
        for line in data.decode(encoding).splitlines():
            header = re.match(r"^\s*\[([^]]+)\]", line)
            if header:
                section = header.group(1).casefold()
            left, separator, right = line.partition("=")
            if separator and left.strip().casefold() == key.casefold():
                found[section] = right.split(";", 1)[0].strip()
        for name in sections:
            if name.casefold() in found:
                return found[name.casefold()]
    return fallback


def runtime_cvars(fps=30, screen=50, pool_mb=256, disable_nanite=False):
    values = {"sg." + name: "0" for name in GROUPS}
    values.update({
        "sg.EffectsQuality": str(EFFECTS_QUALITY),
        "sg.ResolutionQuality": str(screen),
        "t.MaxFPS": str(fps),
        "r.VSync": "0",
        "r.VSyncEditor": "0",
        "r.ScreenPercentage": str(screen),
        "r.ScreenPercentage.Default": str(screen),
        "r.ScreenPercentage.Default.Desktop.Mode": "0",
        "r.Editor.Viewport.ScreenPercentage": str(screen),
        "r.Editor.Viewport.ScreenPercentageMode.RealTime": "0",
        "r.Editor.Viewport.ScreenPercentageMode.NonRealTime": "0",
        "r.Editor.Viewport.HighDPI": "0",
        "r.Editor.Viewport.OverridePIEScreenPercentage": "1",
        "r.DynamicRes.OperationMode": "0",
        "r.ShadowQuality": "0",
        "r.Shadow.Virtual.Enable": "0",
        "r.DynamicGlobalIlluminationMethod": "0",
        "r.ReflectionMethod": "0",
        "r.Lumen.DiffuseIndirect.Allow": "0",
        "r.Lumen.Reflections.Allow": "0",
        "r.SSR.Quality": "0",
        "r.AntiAliasingMethod": "0",
        "r.PostProcessAAQuality": "0",
        "r.MotionBlurQuality": "0",
        "r.DepthOfFieldQuality": "0",
        "r.BloomQuality": "0",
        "r.LensFlareQuality": "0",
        "r.AmbientOcclusionLevels": "0",
        "r.VolumetricFog": "0",
        "r.VolumetricCloud": "0",
        "r.SSS.Quality": "0",
        "r.RefractionQuality": "0",
        "r.MaxAnisotropy": "0",
        "r.Streaming.PoolSize": str(pool_mb),
        "r.Streaming.UseFixedPoolSize": "1",
        "r.Streaming.MipBias": "2",
        "r.Streaming.LimitPoolSizeToVRAM": "1",
        "r.Streaming.MaxEffectiveScreenSize": "720",
        "r.RenderTargetPoolMin": "100",
        "foliage.DensityScale": "0",
        "grass.DensityScale": "0",
    })
    # Explicitly reset the optional fallback mode on a later normal apply.
    values["r.Nanite"] = "0" if disable_nanite else "1"
    return values


def build_profile(project, fps=30, screen=50, pool_mb=256, shader_workers=4,
                  disable_nanite=False):
    """Return project-relative config patches; never writes engine/global files."""
    cvars = runtime_cvars(fps, screen, pool_mb, disable_nanite)
    renderer = dict(RENDERER)
    # Niagara GPU distance-field collision can need this project capability.
    # Also undo the previous profile's override when reapplying with its backup.
    renderer["r.GenerateMeshDistanceFields"] = original_scalar(
        project, ("Config/DefaultEngine.ini",),
        ("/Script/Engine.RendererSettings",), "r.GenerateMeshDistanceFields")
    # Reserve at least one logical core. UE may apply its own low-core overrides.
    unused = max(1, (os.cpu_count() or 2) - shader_workers)
    engine = {
        "/Script/Engine.RendererSettings": renderer,
        "SystemSettings": cvars,
        "SystemSettingsEditor": dict(cvars),
        "DevOptions.Shaders": {
            "ShaderCompilerCoreCountThreshold": "2147483647",
            "NumUnusedShaderCompilingThreads": str(unused),
            "NumUnusedShaderCompilingThreadsDuringGame": str(unused),
        },
    }
    quality = {"sg." + name: "0" for name in GROUPS}
    quality["sg.EffectsQuality"] = str(EFFECTS_QUALITY)
    quality["sg.ResolutionQuality"] = str(screen)
    game = {
        "bUseVSync": "False", "bUseDynamicResolution": "False",
        "FrameRateLimit": str(fps), "ResolutionSizeX": "1280",
        "ResolutionSizeY": "720", "LastUserConfirmedResolutionSizeX": "1280",
        "LastUserConfirmedResolutionSizeY": "720", "FullscreenMode": "2",
        "LastConfirmedFullscreenMode": "2", "Version": "5",
    }
    lyra = dict(game, FrameRateLimit_InMenu=str(fps),
                FrameRateLimit_OnBattery=str(fps),
                FrameRateLimit_WhenBackgrounded=str(min(fps, 10)))
    user = {
        "ScalabilityGroups": quality,
        "/Script/Engine.GameUserSettings": game,
        "/Script/LyraGame.LyraSettingsLocal": lyra,
    }
    editor = {
        "/Script/UnrealEd.EditorLoadingSavingSettings": LOADING,
        "/Script/UnrealEd.MaterialEditorOptions": MATERIAL,
    }
    profile = {
        "Config/DefaultEngine.ini": engine,
        "Config/DefaultGameUserSettings.ini": user,
        "Config/DefaultEditorPerProjectUserSettings.ini": editor,
    }
    # Update current-host generated overrides too. Other hosts consume defaults.
    host = platform_name()
    for platform in (host, host + "Editor"):
        prefix = "Saved/Config/" + platform + "/"
        profile[prefix + "GameUserSettings.ini"] = user
        if platform.endswith("Editor"):
            profile[prefix + "EditorPerProjectUserSettings.ini"] = editor
        # Only patch an existing generated Engine.ini, rather than create one.
        if (project.parent / (prefix + "Engine.ini")).exists():
            profile[prefix + "Engine.ini"] = engine
    return profile


def merge_ini(data, sections):
    """Preserve comments, arrays, unrelated keys, encoding, and line endings."""
    encoding = "utf-8"
    if data.startswith(codecs.BOM_UTF16_LE) or data.startswith(codecs.BOM_UTF16_BE):
        encoding = "utf-16"
    elif data.startswith(codecs.BOM_UTF8):
        encoding = "utf-8-sig"
    source = data.decode(encoding)
    newline = "\r\n" if "\r\n" in source else "\n"
    targets = {s.casefold(): {k.casefold() for k in keys} for s, keys in sections.items()}
    # Remove managed scalars from all occurrences of a section. UE array syntax
    # (+/-/!/.) and unrelated duplicate keys must remain untouched.
    output = []
    section = ""
    for line in source.splitlines(keepends=True):
        header = re.match(r"^\s*\[([^]]+)\]\s*(?:[;#].*)?$", line.strip())
        if header:
            section = header.group(1).casefold()
        key = line.partition("=")[0].strip().casefold()
        if "=" in line and key in targets.get(section, set()):
            continue
        output.append(line)
    text = "".join(output).rstrip("\r\n")
    # Append managed scalars to the last matching section so later duplicates
    # cannot override them. Missing sections are created at the end.
    for name, keys in sections.items():
        lines = text.splitlines()
        indices = [i for i, line in enumerate(lines)
                   if line.strip().casefold() == ("[" + name + "]").casefold()]
        entries = [key + "=" + str(value) for key, value in keys.items() if value is not None]
        if indices:
            pos = indices[-1] + 1
            lines[pos:pos] = entries
        else:
            if lines and lines[-1]:
                lines.append("")
            lines.extend(["[" + name + "]", *entries])
        text = newline.join(lines)
    return (text.rstrip("\r\n") + newline).encode(encoding)


def atomic_write(path, data):
    path.parent.mkdir(parents=True, exist_ok=True)
    fd, temporary = tempfile.mkstemp(prefix=path.name + ".", dir=str(path.parent))
    try:
        with os.fdopen(fd, "wb") as stream:
            stream.write(data)
        os.replace(temporary, path)
    finally:
        if os.path.exists(temporary):
            os.unlink(temporary)


def read_optional(path):
    return path.read_bytes() if path.exists() else None


def restore_bytes(path, data):
    if data is None:
        if path.exists():
            path.unlink()
    else:
        atomic_write(path, data)


def validate_options(fps, screen, pool_mb, shader_workers):
    if not 1 <= fps <= 240 or not 10 <= screen <= 100:
        raise ValueError("fps must be 1..240; screen must be 10..100 percent.")
    if not 64 <= pool_mb <= 8192 or not 1 <= shader_workers <= 128:
        raise ValueError("pool_mb must be 64..8192; shader_workers must be 1..128.")


def apply(project=None, fps=30, screen=50, pool_mb=256, shader_workers=4,
          disable_nanite=False, dry_run=False):
    validate_options(fps, screen, pool_mb, shader_workers)
    project = find_project(project)
    if unreal is not None and project != find_project():
        raise ValueError("In-editor execution must target the currently open project.")
    root = project.parent
    patches = build_profile(project, fps, screen, pool_mb, shader_workers, disable_nanite)
    before = {name: read_optional(root / name) for name in patches}
    after = {name: merge_ini(before[name] or b"", patch) for name, patch in patches.items()}
    log("Project: " + str(project))
    log("Low (Effects Medium) / {} FPS / {}% render scale / {} MB texture pool / {} shader workers requested".format(
        fps, screen, pool_mb, shader_workers))
    log("Effects quality: " + patches["Config/DefaultEngine.ini"]["SystemSettings"]["sg.EffectsQuality"])
    for name in patches:
        if before[name] == after[name]:
            continue
        log("Change: " + name)
        if dry_run:
            # Diffs are diagnostic only; writes retain the original encoding.
            def decode(data):
                if data and data.startswith((codecs.BOM_UTF16_LE, codecs.BOM_UTF16_BE)):
                    return data.decode("utf-16")
                return (data or b"").decode("utf-8-sig")
            log("".join(difflib.unified_diff(decode(before[name]).splitlines(True),
                decode(after[name]).splitlines(True), fromfile=name, tofile=name + " (LowSpec)")))
    if dry_run:
        log("Preview only. No files or editor settings changed.")
        return
    backup = root / BACKUP_DIR
    manifest_path = backup / "manifest.json"
    old_manifest = read_optional(manifest_path)
    manifest = json.loads(old_manifest) if old_manifest else {
        "version": 1, "project": str(project), "files": {},
    }
    if manifest.get("version") != 1:
        raise ValueError("Unsupported backup format; original backup was not changed.")
    # Preserve the FIRST baseline across repeated apply calls.
    for name, data in before.items():
        if name not in manifest["files"]:
            manifest["files"][name] = data is not None
            if data is not None:
                atomic_write(backup / "original" / name, data)
    atomic_write(manifest_path, json.dumps(manifest, indent=2).encode("utf-8"))
    try:
        for name, data in after.items():
            if data != before[name]:
                atomic_write(root / name, data)
    except Exception:
        for name, data in before.items():
            restore_bytes(root / name, data)
        restore_bytes(manifest_path, old_manifest)
        raise
    log("Original settings backup: " + str(backup))
    log("Saved. Restart the editor to apply this profile. No live scalability changes were made.")
    log("Texture pool is a streaming budget, NOT a total RAM/VRAM limit.")


def restore(project=None):
    if unreal is not None:
        raise RuntimeError("Close the editor, then run: python optimize_low_spec.py restore --project PATH. "
                           "The editor's config cache could otherwise overwrite restored files.")
    project = find_project(project)
    root = project.parent
    backup = root / BACKUP_DIR
    manifest_path = backup / "manifest.json"
    if not manifest_path.exists():
        raise ValueError("No active LowSpec backup found: " + str(manifest_path))
    manifest = json.loads(manifest_path.read_bytes())
    if manifest.get("version") != 1:
        raise ValueError("Unsupported backup format.")
    allowed = set(build_profile(project))
    # Also permit generated overrides created by a previous supported host.
    for host in ("Windows", "Mac", "Linux"):
        for suffix in (host, host + "Editor"):
            for filename in ("Engine.ini", "GameUserSettings.ini", "EditorPerProjectUserSettings.ini"):
                allowed.add("Saved/Config/" + suffix + "/" + filename)
    originals = {}
    for name, existed in manifest["files"].items():
        if name not in allowed:
            raise ValueError("Unexpected backup path: " + name)
        originals[name] = (backup / "original" / name).read_bytes() if existed else None
    current = {name: read_optional(root / name) for name in originals}
    archive = backup / ("restored-" + datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%S%fZ"))
    for name, data in current.items():
        if data is not None:
            atomic_write(archive / "before-restore" / name, data)
    try:
        for name, data in originals.items():
            restore_bytes(root / name, data)
    except Exception:
        for name, data in current.items():
            restore_bytes(root / name, data)
        raise
    os.replace(manifest_path, archive / "manifest.json")
    log("Original config files restored. Pre-restore copies: " + str(archive))


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("action", nargs="?", choices=("preview", "apply", "restore"), default="preview")
    parser.add_argument("--project", help=".uproject file or project folder; otherwise auto-detected")
    parser.add_argument("--fps", type=int, default=30)
    parser.add_argument("--screen", type=int, default=50, help="render scale percent")
    parser.add_argument("--pool-mb", type=int, default=256, help="texture streaming budget in MiB")
    parser.add_argument("--shader-workers", type=int, default=4)
    parser.add_argument("--disable-nanite", action="store_true", help="use fallback meshes (may lose geometry)")
    args = parser.parse_args(argv)
    try:
        if args.action == "restore":
            restore(args.project)
        else:
            apply(args.project, args.fps, args.screen, args.pool_mb, args.shader_workers,
                  args.disable_nanite, dry_run=args.action == "preview")
    except Exception as exc:
        log("ERROR: " + str(exc))
        if unreal is not None:
            raise
        return 1
    return 0


if __name__ == "__main__":
    if unreal is not None:
        # Tools > Execute Python Script runs in the editor, whose argv belongs
        # to Unreal rather than this script. Never parse the editor's argv.
        main(["apply"])
    else:
        sys.exit(main())
