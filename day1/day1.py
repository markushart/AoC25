

def read_input(fname: str) -> list[str]:

    with open(fname) as f:
        return f.readlines()


def rotations_to_diffs(rots: list[str]) -> list[int]:

    return [int(s.replace('R', '').replace('L', '-')) for s in rots]


def count_zero_events(ini_rot: int, rots: list[int], rot_max: int) -> int:

    # cumulated sum
    csum = [ini_rot]

    for r in rots:
        csum.append(csum[-1] + r)

    csum = [c % rot_max for c in csum]

    # if the dial is exactly zero
    exact_zero = len(list(filter(lambda x: x == 0, csum)))

    return exact_zero

def count_zero_crossing(ini_rot: int, rots: list[int], rot_max: int):

    # number of times we crossed zero
    zeros = 0

    ticks = ini_rot

    for r in rots:
        while r > 0:
            r -= 1
            ticks = (ticks + 1) % rot_max
            zeros += int(ticks == 0)
        while r < 0:
            r += 1
            ticks = (ticks - 1) % rot_max
            zeros += int(ticks == 0)

    return zeros

if __name__ == "__main__":

    # initial value is 50
    ini_rot = 50

    # max value is 100
    rot_max = 100

    # values to rotate to left (-) or right (+) from the input file
    diffs = rotations_to_diffs(rots=read_input("input.txt"))

    ez = count_zero_events(ini_rot, diffs, rot_max)
    cz = count_zero_crossing(ini_rot, diffs, rot_max)
    print(f"answer to day 1, part 1: {ez}")
    print(f"answer to day 1, part 2: {cz}")


