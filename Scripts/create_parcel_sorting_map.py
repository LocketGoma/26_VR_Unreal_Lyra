"""Run manually in UE Editor after compiling the native ParcelSorting classes.

Requires the built-in Python Editor Script Plugin. Does not build code or use MCP.
Save your current level before running: new_level switches the active editor map.
Existing ParcelSorting maps are preserved.
"""

from pathlib import Path

import unreal


EXPECTED_PROJECT = Path(r"F:\Programing\Git\26_VR\LyraStarterGame\LyraStarterGame.uproject")
MAP_PATH = "/VRSimulate/Maps/L_ParcelSorting"


def verify_project():
    current = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.get_project_file_path())).resolve()
    expected = EXPECTED_PROJECT.resolve()
    if current != expected or not current.is_file():
        raise RuntimeError(f"Wrong project: {current}. Expected: {expected}. No map was changed.")
    feature = current.parent / "Plugins/GameFeatures/VRSimulate"
    if not (feature / "VRSimulate.uplugin").is_file():
        raise RuntimeError(f"VRSimulate descriptor not found under {feature}")
    if Path(__file__).resolve().parent != (feature / "Scripts").resolve():
        raise RuntimeError("Run the script stored in this project's VRSimulate/Scripts directory.")
    return feature


def create_map():
    feature = verify_project()
    map_path = MAP_PATH
    arena_class = unreal.load_class(None, "/Script/VRSimulateRuntime.ParcelSortingArena")
    game_mode_class = unreal.load_class(None, "/Script/VRSimulateRuntime.ParcelSortingGameMode")
    if not arena_class or not game_mode_class:
        raise RuntimeError("VRSimulateRuntime classes are unavailable. Build LyraEditor manually, reopen the verified project, and ensure VRSimulate is Registered or higher.")

    assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
    if assets.does_asset_exist(map_path) or (feature / "Content/Maps/L_ParcelSorting.umap").exists():
        unreal.log("ParcelSorting map already exists; keeping it unchanged: " + map_path)
        return

    levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if not levels.new_level(map_path):
        raise RuntimeError("Unable to create " + map_path)
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    if not world:
        raise RuntimeError("No editor world after creating the map.")
    world.get_world_settings().set_editor_property("default_game_mode", game_mode_class)
    arena = actors.spawn_actor_from_class(arena_class, unreal.Vector(0, 0, 0))
    if not arena:
        raise RuntimeError("Failed to place ParcelSortingArena")
    arena.set_actor_label("Parcel Sorting - Settings (geometry appears in Play)")
    start = actors.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(0, 0, 100))
    if not start:
        raise RuntimeError("Failed to place PlayerStart")
    start.set_actor_label("Parcel Sorting Player Start")
    if not levels.save_current_level():
        raise RuntimeError("Failed to save " + map_path)
    unreal.log("Created " + map_path + ". Tune Arena > Settings, then Play with one player in Standalone net mode.")


if __name__ == "__main__":
    create_map()
