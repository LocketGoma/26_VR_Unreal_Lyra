"""Restore the first LowSpec backup. Close Unreal Editor before running.

Windows: py -3 Scripts/restore_low_spec.py [--project PATH]
macOS/Linux: python3 Scripts/restore_low_spec.py [--project PATH]
Requires optimize_low_spec.py in the same directory. Does not start the editor.
"""

from pathlib import Path
import runpy
import sys


def main(argv=None):
    optimizer = Path(__file__).resolve().with_name("optimize_low_spec.py")
    if not optimizer.is_file():
        print("ERROR: Missing optimizer: " + str(optimizer))
        return 1
    # Import its functions without running the optimizer's apply entry point.
    namespace = runpy.run_path(str(optimizer), run_name="low_spec_restore")
    if namespace["unreal"] is not None:
        # Unreal owns sys.argv. The restore command will explain that the editor
        # must be closed, without modifying any files or applying the profile.
        return namespace["main"](["restore"])
    arguments = sys.argv[1:] if argv is None else argv
    return namespace["main"](["restore", *arguments])


if __name__ == "__main__":
    result = main()
    if result:
        sys.exit(result)
