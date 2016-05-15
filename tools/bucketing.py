#!/usr/bin/python

def semi(freq, offset):
  return freq * (2.0 ** (offset / 12.0))

E1 = semi(440, -29)

freq = [semi(E1, i / 2.0 - 0.25) for i in range(0, 98)]
norm_freq = [f * 512 / 10000 for f in freq]
fixed_freq = [int(f * 256 + 0.5) for f in norm_freq]

for i in range(len(fixed_freq)):
  if (i % 8 == 0):
    if (i > 0):
      print
    print '  ',
  print '0x{:04x}, '.format(fixed_freq[i]),
print

