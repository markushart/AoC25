import numpy as np

DAY = 9

ANYTILE, REDTILE, GREENTILE, RECTMARKER = '.', '#', 'X', 'O'

ANYSPACE_I, REDTILE_I, GREENTILE_I, RECTMARKER_I = 0, 1, 2, 3

TILEMAP = {
    ANYTILE: ANYSPACE_I,
    REDTILE: REDTILE_I,
    GREENTILE: GREENTILE_I,
    RECTMARKER: RECTMARKER_I,
}

TILEMAP_R = {i: k for k, i in TILEMAP.items()}


def tile_map_from_coords(crds: np.ndarray) -> np.ndarray:
    xmax, ymax = np.max(crds[:, 0]) + 2, np.max(crds[:, 1]) + 2

    m = np.zeros((ymax, xmax), dtype=np.uint8)
    m[crds[:, 1], crds[:, 0]] = REDTILE_I
    return m


def tile_map_from_array(a: np.ndarray) -> list[str]:
    return ["".join(TILEMAP_R[c] for c in l) for l in a]


def array_from_tile_map(m: list[str]) -> np.ndarray:
    return np.array([[TILEMAP[c] for c in l if c != '\n'] for l in m])


def print_tile_map(a: np.ndarray = None, m: list[str] = None):
    if a is not None:
        tmap = tile_map_from_array(a)
    else:
        tmap = m

    ll = len(tmap[0])
    fsize = max(len(str(i)) for i in range(ll)) + 1
    headline = f"  | {''.join(' ' * (fsize - len(str(i))) + str(i) for i in range(ll))}"
    print(headline)
    print("-" * len(headline))
    for i, l in enumerate(tmap):
        print(f"{i} | {''.join(' ' * (fsize - 1) + c for c in l)}")


def set_rectangle(m: np.ndarray, c1: tuple[int, int], c2: tuple[int, int]) -> None:
    x1, y1, x2, y2 = min(c1[0], c2[0]), min(
        c1[1], c2[1]), max(c1[0], c2[0]), max(c1[1], c2[1])
    for x in range(x1, x2 + 1):
        for y in range(y1, y2 + 1):
            m[y, x] = RECTMARKER_I


def read_input(file_path: str = "input.txt") -> np.ndarray:
    # find empty line, this will seperate id ranges and ids to check
    with open(file_path, 'r') as f:
        return np.array([[int(s) for s in l.split(',')] for l in f])


''' PART 1'''


def rect_area(c1: tuple[int, int], c2: tuple[int, int]) -> int:
    # get rectangle area
    return (abs(c2[0] - c1[0]) + 1) * (abs(c2[1] - c1[1]) + 1)


def get_biggest_tile_rect(crds: np.ndarray) -> tuple[tuple[int, int], tuple[int, int], int]:

    if crds.shape[1] < 2:
        raise ValueError("At least 2 corners must be provided")

    # corners of max area
    c1mx, c2mx = tuple(crds[0]), tuple(crds[1])

    for c1 in crds:
        for c2 in crds:
            if rect_area(c1, c2) > rect_area(c1mx, c2mx):
                c1mx, c2mx = c1, c2

    return c1mx, c2mx, rect_area(c1mx, c2mx)


''' PART 2'''


def get_bounding_box(crds: np.ndarray) -> tuple[tuple[int, int], tuple[int, int]]:
    return (min(crds[:, 0]), max(crds[:, 0])), (min(crds[:, 1]), max(crds[:, 1]))


def get_biggest_tile_rect_green_red(crds: np.ndarray) -> tuple[tuple[int, int], tuple[int, int], int]:

    if crds.shape[1] < 2:
        raise ValueError("At least 2 corners must be provided")

    # # lexicographic sort in x and y direction
    # cxsrt = crds[np.lexsort(crds.T)]
    # cysrt = crds[np.lexsort(crds[:,::-1].T)]
    # ux = np.unique(cxsrt[:, 1])
    # uy = np.unique(cysrt[:, 0])

    # # divide the edge points into rectangles
    # # there always has to be a multiple of 2 red tiles per row or column
    # # edges are from the odd to the even index of a tile in row/column,
    # # e.g. 1 -- 2     3 -- 4     5 -- 6 ...
    # lx, ly = [], []
    # for x in ux:
    #     r = cxsrt[cxsrt[:, 0] == x]
    #     for i in range(0, r.shape[0], 2):
    #         lx.append((r[i], r[i+1]))

    def inner_point(c, bb) -> bool:
        return c[1] not in bb[1] and c[0] not in bb[0]

    def bb_point(c, bb) -> bool:
        return not inner_point(c, bb)

    def cuts_line(c: int, l: tuple[tuple[int, int], tuple[int, int]], d: int = 1) -> tuple[int, int]:
        if int(l[0][0]) != int(l[1][0]) and int(l[0][1]) != int(l[1][1]):
            raise ValueError("line is diagonal")

        # check cut vertical to direction d
        if l[0][d] < c and c < l[1][d] or \
           l[1][d] < c and c < l[0][d]:
            return (c, int(l[0][1])) if d == 0 else (int(l[0][0]), c)

        return None

    def get_orientation(l) -> int:
        if l[0][0] == l[1][0]:
            return 1
        elif l[0][1] == l[1][1]:
            return 0
        else:
            raise ValueError("line is diagonal")

    def cut_to_rects(points: np.ndarray, cut_dir: int = 1) -> list[tuple[tuple[int, int], tuple[int, int]]]:

        cut_dir_vert = 1 if cut_dir == 0 else 0

        # get first point on bounding box
        bb = get_bounding_box(points)
        sp = 0
        for i, c in enumerate(points):
            if bb_point(c, bb):
                sp = i
                break

        # get point where inner points projection cuts the outline
        subbox_lines = dict()
        for i in list(range(sp, points.shape[0])) + list(range(sp)):
            if inner_point(points[i], bb):
                # found inner point
                for j in range(1, points.shape[0]):
                    l = sorted(points[j-1:j+1], key=lambda x: x[cut_dir_vert])

                    # conversion from numpy...
                    l = ((int(l[0][0]), int(l[0][1])),
                         (int(l[1][0]), int(l[1][1])))

                    # cutting point on line
                    if get_orientation(l) == cut_dir_vert:
                        cp = cuts_line(
                            points[i, cut_dir_vert], l, d=cut_dir_vert)
                        if cp:
                            cp = (int(cp[0]), int(cp[1]))
                            if cut_dir_vert == 0:
                                lines = [(l[0],             (cp[0], l[0][1])),
                                         ((cp[0], l[0][1]),  l[1])]
                            else:
                                lines = [(l[0],             (l[0][0], cp[1])),
                                         ((l[0][0], cp[1]),  l[1])]
                        else:
                            lines = [l]

                        for l in lines:
                            ckey = l[0][cut_dir_vert]
                            if ckey not in subbox_lines.keys():
                                subbox_lines.update({ckey: set()})
                            subbox_lines[ckey].add(l)

        def make_box(l1, l2, cdv):
            if cdv == 0:
                pass
            else:
                pass

        subboxes = []
        for k in list(subbox_lines.keys()):
            lines = subbox_lines.pop(k)

            # remove lines bigger than subbox, they are already broken up
            lmin = min([l[1][cut_dir_vert] for l in lines])
            bl = [l for l in lines if l[1][cut_dir_vert] <= lmin]

            if len(bl) % 2 == 1:
                subboxes.append(make_box(l, ln, cut_dir_vert))
            else:
                for i in range(0, len(bl), 2):
                    l, ln = bl[i], bl[i+1]
                    subboxes.append(make_box(l, ln, cut_dir_vert))
                
        for s in subboxes:
            print(s)

    cut_to_rects(np.array(list(crds) + [crds[0]]))

    return (0, 0), (0, 0), 0


if __name__ == "__main__":

    import sys
    import time

    print(f"===== Day {DAY} =====")

    fname = sys.argv[1] if len(sys.argv) > 1 else "input.txt"
    print_flg = sys.argv[2] == "--print_map" if len(sys.argv) > 2 else False

    # Measure file reading performance
    start_read = time.perf_counter()
    crds = read_input(fname)
    if print_flg:
        tmap = tile_map_from_coords(crds)
        print_tile_map(tmap)
    time_read = (time.perf_counter() - start_read) * 1000
    print(f"Input reading: {time_read:.3f}ms")

    print(f"\n===== Part 1 =====")

    # determine the path the tile beam will take throgh the map
    start_p1 = time.perf_counter()
    c1, c2, area = get_biggest_tile_rect(crds)
    time_p1 = (time.perf_counter() - start_p1) * 1000
    # if print_flg:
    #     set_rectangle(tmap, c1, c2)
    #     print_tile_map(tmap)
    print(f"Time: {time_p1:.3f}ms")
    print(f"biggest tile rectangle is {area} big")
    print(f"\n===== Part 2 =====")

    # determine the path the tile beam will take throgh the map
    start_p2 = time.perf_counter()
    c1, c2, area = get_biggest_tile_rect_green_red(crds)
    time_p2 = (time.perf_counter() - start_p2) * 1000
    print(f"Biggest area using only red and green tiles: {area}")
    print(f"Time: {time_p2:.3f}ms")
