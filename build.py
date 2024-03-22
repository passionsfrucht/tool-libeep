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
    for library_file in (build_dir /  "python/libeep/v3/").glob("pyeep*"):
        library_file.replace(libeep_dir / library_file.name)
