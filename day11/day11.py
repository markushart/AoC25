import numpy as np

DAY = 11

def key_to_int(char_key: str) -> int:
    # build six diget label (every ord of a capital returns a 2 digit number)
    return int("".join(str(ord(c.upper())) for c in char_key.strip()))

def int_to_key(int_key: int) -> str:
    # convert back to string key
    str_key = str(int_key)
    return "".join(chr(int(str_key[i:i+2])) for i in range(0, len(str_key), 2))

LABEL_YOU, LABEL_DAC, LABEL_FFT, LABEL_SRV, LABEL_OUT = "you", "dac", "fft", "svr", "out"

LABEL_YOU_INT = key_to_int(LABEL_YOU) 
LABEL_DAC_INT = key_to_int(LABEL_DAC) 
LABEL_FFT_INT = key_to_int(LABEL_FFT) 
LABEL_SRV_INT = key_to_int(LABEL_SRV) 
LABEL_OUT_INT = key_to_int(LABEL_OUT)


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


def traverse_connections(conn: dict[int, list[int]], 
                         curr_key: int = LABEL_YOU_INT, 
                         target_key: int = LABEL_OUT_INT,
                         keys_to_traverse: list[int] = [],
                         traversal_cache: dict[int, int] = dict()) -> int:

    if curr_key in keys_to_traverse:
        keys_to_traverse.remove(curr_key)

    n_conn = 0 
    if curr_key in traversal_cache:
        # check cache
        n_conn = traversal_cache[curr_key]
    elif curr_key == target_key:
        # break recursion
        n_conn = int(len(keys_to_traverse) == 0)
    else:
        for c in conn[curr_key]:
            n_conn += traverse_connections(conn, curr_key=c, 
                                           target_key=target_key,
                                           keys_to_traverse=keys_to_traverse.copy(),
                                           traversal_cache=traversal_cache)
    
        traversal_cache.update({curr_key: n_conn})

    return n_conn


if __name__ == "__main__":

    import sys
    import time

    print(f"===== Day {DAY} =====")

    fname = sys.argv[1] if len(sys.argv) > 1 else "input.txt"

    print(LABEL_YOU, LABEL_DAC, LABEL_FFT, LABEL_SRV, LABEL_OUT)
    print(LABEL_YOU_INT, LABEL_DAC_INT, LABEL_FFT_INT, LABEL_SRV_INT, LABEL_OUT_INT)

    # Measure file reading performance
    start_read = time.perf_counter()
    conn = read_input(fname)
    time_read = (time.perf_counter() - start_read) * 1000
    print(f"Input reading: {time_read:.3f}ms")

    print(f"\n===== Part 1 =====")

    try:
        # determine the number of paths through the grid
        start_p1 = time.perf_counter()
        n_conn = traverse_connections(conn)
        time_p1 = (time.perf_counter() - start_p1) * 1000
        print(f"Number of connections {n_conn}")
        print(f"Time: {time_p1:.3f}ms")
    except KeyError as e:
        print(f"KeyError: {e} not found in connections. Check input file for missing keys.")

    print(f"\n===== Part 2 =====")
    # count number of paths containing fft and dac
    start_p2 = time.perf_counter()
    n_conn = traverse_connections(conn,
                                  curr_key=LABEL_SRV_INT,
                                  keys_to_traverse=[LABEL_FFT_INT, LABEL_DAC_INT])
    time_p2 = (time.perf_counter() - start_p2) * 1000
    print(f"Number of connections {n_conn}")
    print(f"Time: {time_p2:.3f}ms")
