#!/usr/bin/env python3
import os
from pathlib import Path

import libeep


def main():
    for filename in Path(".").glob("*.cnt"):
        test_reading(filename)
        test_reading(os.fspath(Path(filename).absolute()))


def test_reading(file: str):
    try:
        print(f"Testing {file}")
        c = libeep.CntReader(file)
        print(f"Channel count: {c.get_channel_count()}")
        print(f"Sampling rate: {c.get_sample_frequency()}")
        print(f"Sample count: {c.get_sample_count()}")
        print(f"Trigger count: {c.get_trigger_count()}")
        print(f"Trigger: {c.get_trigger(0)}")
        print(f"Channel info: {c.get_channel_info(0)}")
        print(f"Samples: {c.get_samples(0, 10)}")
        print("!!! PASSED !!!")
    except Exception as e:
        print(f"!!! FAILED !!! {e}")


if __name__ == "__main__":
    main()
