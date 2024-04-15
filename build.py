import subprocess
from pathlib import Path
from typing import Any


def build(setup_kwargs: Any) -> None:
    build_dir = Path("./build-cmake")
    build_dir.mkdir(exist_ok=True)

    subprocess.check_call(["cmake", ".."], cwd=build_dir)
    subprocess.check_call(["cmake", "--build", ".", "--config", "Release"], cwd=build_dir)

    # copy the resulting files to the python package
    libeep_dir = Path("./python/libeep")
    python_result_dir = build_dir /  "python/libeep/v3/"
    # May contain weird windows directories. But the build on windows results in the files ending up
    # in a 'Releases' folder.
    potential_eep_files = python_result_dir.glob("**/pyeep*")
    files_with_right_suffix = [file for file in potential_eep_files if file.suffix in [".pyd", ".so"]]
    assert files_with_right_suffix, "Could not find the correct build result"
    for file in files_with_right_suffix:
        file.replace(libeep_dir / file.name)
