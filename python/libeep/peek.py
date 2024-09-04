from .cnt_reader import CntReader


def peek(filename):
    from collections import Counter

    print(f"Peeking into {filename}:")
    with CntReader(filename) as f:
        channels = []
        for c in range(f.get_channel_count()):
            channels.append(f.get_channel_info(c)[0])
        print("Channels:", channels)
        events = []
        for t in range(f.get_trigger_count()):
            events.append(f.get_trigger(t)[0])
        print("Unique event names:", Counter(events), "total is ", len(events))
        print("Sampling Rate:", f.get_sample_frequency())
        duration = f.get_sample_count() / f.get_sample_frequency()
        print(f"Duration: {duration:10.2f}s")
