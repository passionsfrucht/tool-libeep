__all__ = [
    "pyeep",
    "CntReader",
    "CntWriter",
    "peek",
]

from . import pyeep
from .cnt_reader import CntReader
from .cnt_writer import CntWriter
from .peek import peek


def main():
    import sys

    filename = sys.argv[1]
    peek(filename)
