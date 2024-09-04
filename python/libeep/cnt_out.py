###############################################################################
from pathlib import Path

from libeep import pyeep

from .cnt_file import cnt_file


class cnt_out(cnt_file):
    def __init__(self, fname: str, rate: int, channels: list[str], rf64=0):
        """A cnt-file for storing data on your harddrive

        args
        ----
        fname:str
            the path to the filename
        rate:
            sampling rate in Herz
        channels:
            list of tuples, where tuples contains three strings:
              channel label, channel reference and unit, i,e,:
              ['Cz', 'ref', 'uV')]
        rf64:
            if 0, create default 32-bit cnt data. otherwise 64 bit(for larger tan 2GB files)
        """
        fname = Path(fname).expanduser().absolute()
        if not fname.suffix == ".cnt":
            raise Exception("unsupported extension")
        if fname.exists():
            fname.unlink()
        fname.touch()

        self._fname = str(fname)
        channels_handle = pyeep.create_channel_info()
        for c in channels:
            pyeep.add_channel(channels_handle, c[0], c[1], c[2])
        self._handle = pyeep.write_cnt(self._fname, rate, channels_handle, rf64)
        if self._handle == -1:
            raise Exception("not a valid libeep handle")

        self._channel_count = len(channels)

    def add_samples(self, samples):
        return pyeep.add_samples(self._handle, samples, self._channel_count)

    def add_trigger(self, sample: int, marker: str):
        return pyeep.add_trigger(self._handle, sample, marker)
