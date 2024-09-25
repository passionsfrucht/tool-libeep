###############################################################################
from pathlib import Path
from typing import Any

from libeep import pyeep


class CntReader:
    """A cnt file, ready to be used for reading

    args
    ----
    fname:str
        the path to the filename

    """

    def __del__(self):
        self.close()

    def __init__(self, fname: str):
        self._fname = str(Path(fname).expanduser().absolute())
        self._handle = None

    def __enter__(self):
        self.open()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()

    def open(self):
        "open the file for queries"
        if self._handle is not None:
            self.close()
        handle = pyeep.read(self._fname)
        if handle == -1:
            self._handle = None
            raise Exception("Received an invalid libeep file handle")
        else:
            self._handle = handle
            # print(f"Opening {self._fname} as {self._handle}")
            return self

    def close(self):
        "close and release the file"
        if self._handle is not None:
            pyeep.close(self._handle)
            self._handle = None
        # print(f"Closing {self._handle} for {self._fname}")

    def get_channel_count(self) -> int:
        "get the number of channels stored in the file"
        with self as f:
            channel_count = pyeep.get_channel_count(f._handle)
        return channel_count

    def get_channel_info(self, index: int) -> tuple[str, str, str]:
        """get information about a specific channel

        args
        ----
        index:int
            the index number for the channel

        returns
        -------
        Tuple[str, str, str]:
            the information where tuples contains three strings:
            channel label, channel reference and unit, i,e,:
            ('Cz', 'ref', 'uV')
        """
        with self as f:
            info = (
                pyeep.get_channel_label(f._handle, index),
                pyeep.get_channel_reference(f._handle, index),
                pyeep.get_channel_unit(f._handle, index),
            )
        return info

    def get_sample_frequency(self) -> int:
        "returns the sample frequency of the data as int"
        with self as f:
            fs = pyeep.get_sample_frequency(f._handle)
        return fs

    def get_sample_count(self):
        "returns the number of samples in the file as int"
        with self as f:
            sample_count = pyeep.get_sample_count(f._handle)
        return sample_count

    def get_samples(self, fro: int, to: int) -> list[list[float]]:
        """load a range of samples from the file

        args
        ----
        fro:int
          the first sample to load
        to:int
          the last sample to load

        returns
        ------
        data: List[List[float]]
            a list with length samples containing a list of values for each channel

        Example
        -------

        data = cnt.get_samples(0,1)
        # return the first sample for all channels

        """
        data = []
        sample_count = self.get_sample_count()
        with self as f:
            steps = to - fro
            if steps == 0:
                raise IndexError("No samples selected")
            if steps > sample_count:
                raise IndexError("Not enough samples available")
            for step in range(0, steps):
                sample = pyeep.get_samples(f._handle, fro + step, fro + step + 1)
                data.append(sample)
        return data

    def get_trigger_count(self) -> int:
        "return the number of triggers in the file as int"
        with self as f:
            trigger_count = pyeep.get_trigger_count(f._handle)
        return trigger_count

    def get_trigger(self, index: int) -> tuple[str, int, int, Any, Any, Any]:
        """get information for a specific trigger

        args
        ----
        index:int
            the index number of the trigger of interest

        returns
        info: Tuple[str, int, int, Any, Any, Any]
            information about this trigger in tehe following format
            (markertype, sample_index, markervalue, Any, Any, Any)
        """
        tc = self.get_trigger_count()
        if index < 0:
            raise IndexError(
                f"{index} is smaller zero. Only positive indices are allowed"
            )

        if index > tc or tc == 0:
            raise IndexError(f"{index} larger than trigger count of {tc}")
        with self as f:
            info = pyeep.get_trigger(f._handle, index)
        return info
