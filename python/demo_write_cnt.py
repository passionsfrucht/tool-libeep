#!/usr/bin/python

import libeep

channels = [('Cz', '', 'uV'), ('Cz', '', 'uV')]
cnt = libeep.write_cnt('python.cnt', 512, channels, 0)
# write 4 samples at once
cnt.add_samples([0,0,1,0,2,0,3,0])

# loop while writing single sample
for n in range(1024):
  cnt.add_samples([13,n])
