How to submit changes:
 *) create a patch of your changes to the latest trunk in the Subversion tree.
 *) send the patch in an e-mail to rsmies@ant-neuro.com. don't forget to include a description of your change in the body of the e-mail.

How to enable the MATLAB importers:
 *) make sure you have a working compiler toolchain(MacOSX users may need to install XCode from the app-store)
 *) assuming you have MATLAB installed in '/opt/MATLAB' and you want your plugin to be installed in '/opt/ant-eep'(you are free to change these off-course), you can compile and install the plugin using: MATLAB=/opt/MATLAB ./configure --enable-matlab --prefix=/opt/ant-eep --with-matlab=/opt/MATLAB && make install
 *) You may need to run 'make install' as super user, or with the sudo command on some platforms
 *) start MATLAB, make sure the (install dir)/share/matlab path is added to MATLAB
 *) run [my_data] = read_eep_cnt('/path/to/my_data.cnt', 1, 1024) to read the first 1024 samples

## Linux

```
make clean python
pip install -e .
```

## Windows

from the project root, run in bash

```{bash}
mkdir build
cmake -S . -B build
make -C build
```

You will find a file called `pyeep.so` in `build/python/v3`. This is a python extension to load eego files.

Create and install a python package by running

```{bash}
mkdir libeep
cp build/python/v3/pyeep.so libeep
cp python/__init__.py libeep
pip install -e .
```

## Object-oriented-interface for python

After running `pip install -e .`, you will then be able to import eep and use its object oriented interface.

```{python}
import libeep
import matplotlib.pyplot as plt

fname = "example.cnt"
cnt = libeep.cnt_file(fname)
print(f"Within the file {fname} are")
print(f"{cnt.get_channel_count()} channels and ")
print(f"{cnt.get_sample_count()} samples and")
print(f"{cnt.get_trigger_count()} events")

# load 1s before and after the second event
fs = cnt.get_sample_frequency()
marker, tstamp, *info = cnt.get_trigger(1)
data = cnt.get_samples(tstamp-fs, tstamp+fs)
# plot the data
plt.plot(data)
```



### Low-level interface

Because it is a python extension, `pyeep.so` can also be simply imported with `import pyeep`. This does only offer a low-level interface. You can find an example use case [here](python/demo_read_cnt.py) or in the following:

```{python}
import pyeep
fname = "example.cnt"
fh = pyeep.read(fname)  # get the file handle

sampling_rate = pyeep.get_sample_frequency(fh)

chan_count = pyeep.get_channel_count(fh)
channel_labels = [pyeep.get_channel_label(fh, chan) for chan in range(chan_count)]

sample_count = pyeep.get_sample_count(fh) # get how many samples are there
data = pyeep.get_samples(fh, 0, sample_count)  # load them all


trigger_count = pyeep.get_trigger_count(fh)
markers = [pyeep.get_trigger(fh, trigger) for trigger in range(trigger_count)]

```
