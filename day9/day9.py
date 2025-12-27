import numpy as np
import matplotlib.pyplot as plt

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


def get_bounding_box(crds: np.ndarray) -> np.ndarray:
    return np.array([(min(crds[:, 0]), min(crds[:, 1])), (max(crds[:, 0]), max(crds[:, 1]))], dtype=int)


def inner_point(c, bb) -> bool:

    return c[1] not in bb[1] and c[0] not in bb[0]


def bb_point(c, bb) -> bool:

    return not inner_point(c, bb)


def is_in_rect(c1: np.ndarray, c2: np.ndarray, p: np.ndarray, include_borders: bool = False) -> bool:
    # check if p3 is within the rectangle build by corners c1 and c2
    bb = get_bounding_box(np.array([c1, c2]))
    if include_borders:
        return (bb[0, 0] <= p[0] and p[0] <= bb[1, 0]) and (bb[0, 1] <= p[1] and p[1] <= bb[1, 1])
    else:
        return (bb[0, 0] < p[0] and p[0] < bb[1, 0]) and (bb[0, 1] < p[1] and p[1] < bb[1, 1])


def get_orientation(l) -> int:
    if l[0][0] == l[1][0]:
        return 1
    elif l[0][1] == l[1][1]:
        return 0
    else:
        raise ValueError("line is diagonal")


def line_cuts_rect(p1: np.ndarray, p2: np.ndarray, l: np.ndarray) -> bool:
    bb = get_bounding_box(np.array((p1, p2)))
    bl = get_bounding_box(l)
    o = get_orientation(l)
    no = 0 if o == 1 else 1
    return (bl[0, o] < bb[0, o] and bb[0, o] < bl[1, o] or
            bl[0, o] < bb[1, o] and bb[1, o] < bl[1, o]) and \
           (bb[0, no] < bl[0, no] and bl[0, no] < bb[1, no])


def get_biggest_tile_rect_green_red(crds: np.ndarray) -> tuple[tuple[int, int], tuple[int, int], int]:

    if crds.shape[1] < 2:
        raise ValueError("At least 2 corners must be provided")

    """
    picture possible quadrants of opposing rectangle corner
    --------------------------------------
    convex corner         / concave corner
    \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/
    xxxxxxxxx|xxxxxxxxx   /          |xxxxxxxxx
    xxx 3 xxx|xxx 0 xxx   \     3    |xxx 0 xxx
    xxxxxxxxx|xxxxxxxxx   /          |xxxxxxxxx
    ---------c--------->  \ ---------c--------->
             |xxxxxxxxx   /          |        
        2    |xxx 1 xxx   \     2    |    1   
             |xxxxxxxxx   /          |        
             v            \          v
    """

    # clock         | ^->   | ->|   |  3|   | ^ 0   |
    # wise          | |1    | 2 v   | <-v   | |<-   |
    # -----------------------------------------------
    #               | (0,-1)| (1, 0)| (0, 1)|(-1, 0)|
    #               | (1, 0)| (0, 1)| (-1,0)|( 0,-1)|
    # -----------------------------------------------
    # counter-clock |3|     |   ^   |  3 0  |3 0   |
    # wise          |2v->   | ->|0  | <-^   |2|<-  |
    #               |  1    | 2  1  |   |1  | v    |
    # -----------------------------------------------
    #               | (0, 1)| (1, 0)| (0,-1)|(0, -1)|
    #               | (1, 0)| (0, 1)| (-1,0)|(-1, 0)|

    convex_corners = np.array((
        (-1, 0),
        (0, -1),
        (1,  0),
        (0,  1),
        (-1, 0),
    ))

    concave_corners = np.array((
        (0,  1),
        (1,  0),
        (0, -1),
        (-1, 0),
        (0,  1),
    ))

    r_quad_keys = {
        True: {
            0: (1, 2, 3),
            1: (0, 1, 2),
            2: (0, 1, 3),
            3: (0, 2, 3),
        },
        False: {
            0: (0,),
            1: (1,),
            2: (2,),
            3: (3,),
        }
    }

    def get_quadrant(p: np.ndarray, c: np.ndarray, rq: np.ndarray) -> list[np.ndarray]:

        # compare to concave / convex corners
        for concave, cc in zip([False, True], [convex_corners, concave_corners]):
            for j in range(cc.shape[0]-1):
                if np.all(c == cc[j:j+2]):
                    return [p + rq[k] for k in r_quad_keys[concave][j]]

        raise ValueError("Corner shape not found")

    bb = get_bounding_box(crds)

    # relative quadrants
    w, h = bb[1] - bb[0]
    r_quad = np.array((
        ((0, -h), (w, 0)),
        ((0, 0), (w, h)),
        ((-w, 0), (0, h)),
        ((-w, -h), (0, 0)),
    ))

    # asume the coordinates are listed in clockwise order
    # we build an array of edges with length 1 that store edge orientation
    rlines = np.diff(np.concat((crds[-1:], crds, crds[:1])), axis=0)
    norm = np.sum(rlines, axis=1)
    norm *= np.sign(norm)
    rlines[:, 0] //= norm
    rlines[:, 1] //= norm

    # walk along this index to get wrapped point list
    lidx = np.array(list(zip(
        list(range(crds.shape[0])),
        [crds.shape[0] - 1] + list(range(0, crds.shape[0] - 1)
    ))))

    imax, jmax = 0, 0
    for i in range(rlines.shape[0]-1):
        # get corner
        c1, p1 = rlines[i:i+2], crds[i]
        q1 = get_quadrant(p1, c1, r_quad)

        for j in range(rlines.shape[0]-1):
            # get corner
            c2, p2 = rlines[j:j+2], crds[j]

            # check if rectangle could be bigger
            if rect_area(p1, p2) <= rect_area(crds[imax], crds[jmax]):
                continue

            # check if point is within reachable quadrant of other point
            # -> if so, the point might be corner point of inner rectangle
            if not any(is_in_rect(q[0], q[1], p2, include_borders=True) for q in q1):
                continue

            # reverse check
            q2 = get_quadrant(p2, c2, r_quad)
            if not any(is_in_rect(q[0], q[1], p1, include_borders=True) for q in q2):
                continue

            # check if any other point is inside the rectangle build by c1, c2
            # -> if so, the shape is not convex and we need to search further
            if any(is_in_rect(p1, p2, p) for p in crds):
                continue

            # check if any other line cuts the rectangle
            if any(line_cuts_rect(p1, p2, np.array((crds[k1], crds[k2]))) for k1, k2 in lidx):
                continue

            imax, jmax = i, j

    return crds[imax], crds[jmax], rect_area(crds[imax], crds[jmax])


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
    if print_flg:
        set_rectangle(tmap, c1, c2)
        print_tile_map(tmap)
    print(f"Biggest area using only red and green tiles: {area}")
    print(f"corners: ({c1[0]},{c1[1]}) -- ({c2[0]},{c2[1]})")
    print(f"Time: {time_p2:.3f}ms")

    cwr = np.concatenate((crds, crds[:1]))
    bb = get_bounding_box(np.array((c1, c2)))
    fig, ax1, = plt.subplots(1, 1)
    ax1.plot(cwr[:, 0], cwr[:, 1])
    ax1.add_patch(plt.Rectangle(
        bb[0], bb[1, 0] - bb[0, 0], bb[1, 1] - bb[0, 1]))
    plt.show()
