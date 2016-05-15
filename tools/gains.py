#!/usr/bin/python

low_boost = [10-(x/3.0) for x in range(30)]
mid_flat = [0 for x in range(40)]
high_boost = [x/3.0 for x in range(27)]

gains_db = low_boost + mid_flat + high_boost
gains = [10 ** (x/10) for x in gains_db]
gains_fixed = [int(x *16 + 0.5) for x in gains]

for i in range(len(gains_fixed)):
  print '0x{:02x}, '.format(gains_fixed[i]),
  if i % 8 == 7:
    print

