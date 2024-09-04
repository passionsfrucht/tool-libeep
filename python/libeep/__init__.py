__all__ = [
    "pyeep",
    "cnt_file",
    "cnt_out",
    "peek",
]

from . import pyeep
from .cnt_file import cnt_file
from .cnt_out import cnt_out
from .peek import peek


def main():
    import sys

    filename = sys.argv[1]
    peek(filename)
