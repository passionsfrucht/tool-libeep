from pathlib import Path

import libeep
import numpy as np
import pytest


@pytest.fixture()
def tmp_cnt(tmp_path):
    d = tmp_path / "sub"
    d.mkdir()
    fname = str(Path(d) / "test.cnt")
    rate = 1023
    channel_count = 8
    channels = [(f"Ch{i}", "None", "uV") for i in range(1, channel_count + 1, 1)]
    c = libeep.CntWriter(fname, rate, channels)
    samples = np.repeat(np.arange(0, 100), 8).flatten().tolist()
    c.add_samples(samples)
    c.add_trigger(5, "hello")
    c.close()
    yield fname, rate, channel_count


def test_creation(tmp_cnt):
    fname, rate, channel_count = tmp_cnt
    assert Path(fname).exists()


def test_channels(tmp_cnt):
    fname, rate, channel_count = tmp_cnt
    c = libeep.CntReader(fname)
    assert c.get_channel_count() == channel_count
    for i in range(c.get_channel_count()):
        channel_label, channel_reference, channel_unit = c.get_channel_info(i)
        assert channel_label == f"Ch{i+1}"
        assert channel_reference == "None"
        assert channel_unit == "uV"


def test_sampling_rate(tmp_cnt):
    fname, rate, channel_count = tmp_cnt
    c = libeep.CntReader(fname)

    assert c.get_sample_frequency() == rate


def test_add_samples(tmp_cnt):
    fname, rate, channel_count = tmp_cnt
    c = libeep.CntReader(fname)
    data = c.get_samples(0, c.get_sample_count())
    assert all([x == 0.0 for x in data[0]])
    assert all([x == 99.0 for x in data[-1]])


def test_add_trigger(tmp_cnt):
    fname, rate, channel_count = tmp_cnt
    c = libeep.CntReader(fname)
    assert c.get_trigger_count() == 1
    for i in range(c.get_trigger_count()):
        m, s, *_ = c.get_trigger(i)
        print(m, s)
        assert m == "hello"
        assert s == 5
