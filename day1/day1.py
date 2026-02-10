import math

def read_input(fname: str) -> list[str]:

    with open(fname) as f:
        return f.readlines()


def rotations_to_diffs(rots: list[str]) -> list[int]:

    return [int(s.replace('R', '').replace('L', '-')) for s in rots]


def count_zero_events(ini_rot: int, rots: list[int], rot_max: int) -> int:

    # cumulated sum
    csum = ini_rot 
    zeros = 0
    for r in rots:
        csum += r
        # add if dial is at zero after rotation
        zeros += int(csum % rot_max == 0)
    return zeros 


def count_zero_crossing(ini_rot: int, rots: list[int], rot_max: int):

    zeros = 0
    pos = ini_rot  # unwrapped position

    for r in rots:
        rsign = -1 if r > 0 else 1
        pnow = pos + int(pos % rot_max == 0) * rsign
        pnext = (pos + r) + int((pos + r) % rot_max == 0) * rsign
        zeros += abs(pnext // rot_max - pnow // rot_max)
        pos += r

    return zeros


if __name__ == "__main__":

    import time
    import sys

    fname = sys.argv[1] if len(sys.argv) > 1 else "input.txt"

    # initial value is 50
    ini_rot = 50

    # max value is 100
    rot_max = 100

    # values to rotate to left (-) or right (+) from the input file
    diffs = rotations_to_diffs(rots=read_input(fname))

    # add performance measurement
    start_p1 = time.perf_counter()
    ez = count_zero_events(ini_rot, diffs, rot_max)
    time_p1 = (time.perf_counter() - start_p1) * 1000
    print(f"answer to day 1, part 1: {ez} (time: {time_p1:.3f}ms)")

    start_p2 = time.perf_counter()
    cz = count_zero_crossing(ini_rot, diffs, rot_max)
    time_p2 = (time.perf_counter() - start_p2) * 1000
    print(f"answer to day 1, part 2: {cz} (time: {time_p2:.3f}ms)")
