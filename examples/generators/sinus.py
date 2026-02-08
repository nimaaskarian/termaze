#!/usr/bin/python3
import sys
maxy = int(sys.argv[1])
maxx = int(sys.argv[2])

numwaves=maxx//4
waves=(
        numwaves*"PP L",
        numwaves*" P P",
        numwaves*" PPP",
)

for wave in waves:
    print(wave)
