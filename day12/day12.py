import numpy as np

DAY = 12


def read_input(fname: str):
    gift_shapes, gift_grid, gifts_per_grid = [], [], []

    with open(fname, 'r') as f:
        lines = [l.removesuffix("\n") for l in f.readlines()]

    # line index of gift shape index
    ii = [i for i, l in filter(lambda il: il[1].isdigit(),
                               map(lambda il: (il[0], il[1].split(':')[0]), enumerate(lines)))]

    j = 0
    for i in ii:
        gift_shapes.append([])
        j = 1
        while lines[i + j]:
            l = lines[i+j]
            gift_shapes[-1].append([int(i) for i in l.replace('#', '1').replace('.','0')])
            j += 1
    
    gift_shapes = [np.array(gs, dtype=bool) for gs in gift_shapes]

    # get grids
    gl_start = ii[-1] + j + 1 # grid line start

    for l in lines[gl_start:]:
        gsz, gpg = l.split(':')

        gift_grid.append(tuple([int(i) for i in gsz.split('x')]))
        gifts_per_grid.append(np.array([int(i) for i in gpg.strip().split()]))

    return gift_shapes, gift_grid, gifts_per_grid


if __name__ == "__main__":

    import sys

    print(f"DAY {DAY}")

    fname = "input.txt"

    if len(sys.argv) > 1:
        fname = sys.argv[1]

    shapes, grids, gpg = read_input(fname=fname)

    print("==================== PART 1 ====================")

    print("==================== PART 2 ====================")
