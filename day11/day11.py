import numpy as np

DAY = 11

LABEL_YOU, LABEL_OUT = "you", "out"
LABEL_YOU_INT, LABEL_OUT_INT = 0, 999999


def key_to_int(char_key: str) -> int:
    ck = char_key.strip()
    if ck == LABEL_YOU:
        return LABEL_YOU_INT
    elif ck == LABEL_OUT:
        return LABEL_OUT_INT
    else:
        # build six diget label (every ord of a capital returns a 2 digit number)
        return int("".join(str(ord(c.upper())) for c in ck))


def read_input(file_path: str = "input.txt") -> dict[int, list[int]]:
    with open(file_path, 'r') as f:
        d = dict()
        for l in f:
            char_key, char_connections = l.split(':')
            # split into key and its connections
            d.update({key_to_int(char_key): [
                key_to_int(c) for c in char_connections.split()
            ]})

        return d


def traverse_connections(conn: dict[int, list[int]], curr_key: int = LABEL_YOU_INT, target_key: int = LABEL_OUT_INT):

    if curr_key == target_key:
        # break recursion
        return 1
    else:
        n_conn = 0 
        for c in conn[curr_key]:
            n_conn += traverse_connections(conn, curr_key=c, target_key=target_key)
        return n_conn


if __name__ == "__main__":

    import sys
    import time

    print(f"===== Day {DAY} =====")

    fname = sys.argv[1] if len(sys.argv) > 1 else "input.txt"

    # Measure file reading performance
    start_read = time.perf_counter()
    conn = read_input(fname)
    time_read = (time.perf_counter() - start_read) * 1000
    print(f"Input reading: {time_read:.3f}ms")

    print(f"\n===== Part 1 =====")

    # determine the number of paths through the grid
    start_p1 = time.perf_counter()
    n_conn = traverse_connections(conn)
    time_p1 = (time.perf_counter() - start_p1) * 1000
    print(f"Number of connections {n_conn}")
    print(f"Time: {time_p1:.3f}ms")
    print(f"\n===== Part 2 =====")

    #
    start_p2 = time.perf_counter()

    time_p2 = (time.perf_counter() - start_p2) * 1000

    print(f"Time: {time_p2:.3f}ms")
