import numpy as np

DAY = 7

EMPTYSPACE, TACHSTART, TACHBEAM, TACHSPLIT = '.', 'S', '|', '^'
EMPTYSPACE_I, TACHSTART_I, TACHBEAM_I, TACHSPLIT_I = 0, 1, 2, 3

TACHMAP = {
    EMPTYSPACE: EMPTYSPACE_I,
    TACHSTART: TACHSTART_I,
    TACHBEAM: TACHBEAM_I,
    TACHSPLIT: TACHSPLIT_I,
}

TACHMAP_R = {i: k for k, i in TACHMAP.items()}


def tachion_map_from_array(a: np.ndarray) -> list[str]:
    return ["".join(TACHMAP_R[c] for c in l) for l in a]


def array_from_tachion_map(m: list[str]) -> np.ndarray:
    return np.array([[TACHMAP[c] for c in l if c != '\n'] for l in m])


def print_tachion_map(a: np.ndarray = None, m: list[str] = None):
    if a is not None:
        tmap = tachion_map_from_array(a)
    else:
        tmap = m

    for l in tmap:
        print(l)


def read_input(file_path: str = "input.txt") -> np.ndarray:
    with open(file_path, 'r') as f:
        return array_from_tachion_map(f.readlines())


''' PART 1'''

def tachion_ray_trace(m: np.ndarray, multiverse: bool=False) -> tuple[int, np.ndarray]:

    split_count = np.zeros_like(m)
    
    # recursive approach to find all paths through the grid
    def tachion_ray_trace_rec(m: np.ndarray, x: int, y: int, multiverse: bool) -> tuple[list[tuple[int, int]], int]:
        spl = 0
        coords = []

        if m[y, x] != TACHSTART_I:
            # do not overwrite 'S'
            m[y, x] = TACHBEAM_I

        if y >= m.shape[0] - 1:
            # end
            return [(x, y)], 0
        elif m[y + 1, x] == TACHSPLIT_I:
            # splitter
            if split_count[y, x]:
                [], split_count[y, x] 
            else:
                c1, c2, spl1, spl2 = [], [], 0, 0
                c1, spl1 = tachion_ray_trace_rec(m, x - 1, y + 1, multiverse)
                c2, spl2 = tachion_ray_trace_rec(m, x + 1, y + 1, multiverse)
                spl = spl1 + spl2 + 1
                
                split_count[y, x] = spl

                coords = c1 + c2
        else:
            # simple step forward
            coords, spl = tachion_ray_trace_rec(m, x, y + 1, multiverse)
        return [(x, y)] + coords, spl

    # search start
    x_start = int(np.argwhere(m[0] == TACHSTART_I)[0, 0])

    _, splits = tachion_ray_trace_rec(m, x_start, 0, multiverse)

    print(split_count)

    return splits


''' PART 2'''

def count_possible_tachion_worlds(m: np.ndarray) -> int:
    # get possible worlds per level
    wpl = 0

    return 0


if __name__ == "__main__":

    import sys
    import time

    print(f"===== Day {DAY} =====")

    fname = sys.argv[1] if len(sys.argv) > 1 else "input.txt"

    # Measure file reading performance
    start_read = time.perf_counter()
    tmap = read_input(fname)
    time_read = (time.perf_counter() - start_read) * 1000
    print_tachion_map(a=tmap)
    print(f"Input reading: {time_read:.3f}ms")

    print(f"\n===== Part 1 =====")

    # determine the path the tachion beam will take throgh the map
    start_p1 = time.perf_counter()
    tmap1 = tmap.copy()
    result = tachion_ray_trace(tmap1,)
    time_p1 = (time.perf_counter() - start_p1) * 1000
    print_tachion_map(tmap1)
    print(f"Tachion beam is split into {result} beams")
    print(f"Time: {time_p1:.3f}ms")

    print(f"\n===== Part 2 =====")

    # determine the path the tachion beam will take throgh the map
    start_p2 = time.perf_counter()
    tmap2 = tmap.copy()
    result = tachion_ray_trace(tmap2, multiverse=True)
    print(f"Tachion beam is split into {result} beams")
    time_p2 = (time.perf_counter() - start_p2) * 1000
    print(f"Time: {time_p2:.3f}ms")
