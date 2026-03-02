__all__ = [
    "pyeep",
    "CntReader",
    "CntWriter",
    "peek",
]

from loguru import logger

from . import pyeep
from .cnt_reader import CntReader
from .cnt_writer import CntWriter
from .peek import peek

logger.disable("libeep")  # Disable logging by default


def main():
    import sys

    filename = sys.argv[1]
    peek(filename)
