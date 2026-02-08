#!/usr/bin/python3
import sys
def spiral(n):
    grid = [[' ' for _ in range(n)] for _ in range(n)]
    top = 0
    bottom = n - 1
    left = 0
    right = n - 1

    while top <= bottom and left <= right:
        start_col = left if top == 0 else left - 1
        for c in range(start_col, right + 1):
            grid[top][c] = 'P'
        for r in range(top + 1, bottom + 1):
            grid[r][right] = 'P'
        if top < bottom:
            for c in range(right - 1, left - 1, -1):
                grid[bottom][c] = 'P'
        if left < right:
            for r in range(bottom - 1, top + 1, -1):
                grid[r][left] = 'P'
        top += 2
        bottom -= 2
        left += 2
        right -= 2
    grid[top][right] = 'L'

    for row in grid:
        print(''.join(row))
maxy = int(sys.argv[1])
maxx = int(sys.argv[2])
max = min(maxx, maxy)
spiral(max)
