import numpy as np
from numpy.lib.stride_tricks import sliding_window_view
import time
from typing import Callable, Any

day = 4

EMPTY_CHAR, ROLL_CHAR, EMTPY, ROLL = '.', '@', 0, 1


def read_input_paper_rolls(file_path: str = "input.txt") -> np.ndarray:
    # replace EMPTY_CHAR and ROLL_CHAR with EMPTY and ROLL
    with open(file_path, 'r') as f:
        rolls = [
            [EMTPY if c == EMPTY_CHAR else ROLL for c in line.strip()]
            for line in f
        ]
        return np.array(rolls, dtype=int)


def roll_array_to_str(rolls: np.ndarray) -> str:
    char_map = {
        EMTPY: EMPTY_CHAR,
        ROLL: ROLL_CHAR
    }
    lines = []
    for row in rolls:
        line = ''.join([char_map[c] for c in row])
        lines.append(line)
    return '\n'.join(lines)


''' PART 1'''


def det_num_accessible_rolls(rolls: np.ndarray, adjacent_rolls_limit: int = 4) -> int:

    # pad rolls with empty slots
    rpad = np.pad(rolls, pad_width=1, mode='constant', constant_values=EMTPY)
    # rolls are accessible if at least 5 surrounding slots are empty
    w = sliding_window_view(rpad, (3, 3))
    # only keep windows where center is a roll
    w = w[w[:, :, 1, 1] == ROLL, :, :]
    # get sum of rolls in each window
    s = np.sum(w, axis=(-2, -1))
    return np.sum(s <= adjacent_rolls_limit)  # +1 because center is roll


def sum_accessible_rolls(rolls: np.ndarray, adjacent_rolls_limit: int = 4) -> int:
    return det_num_accessible_rolls(rolls, adjacent_rolls_limit=adjacent_rolls_limit)


''' PART 2'''


def det_accessible_rolls(rolls: np.ndarray, adjacent_rolls_limit: int = 4) -> np.ndarray:

    def is_accessible(window: np.ndarray) -> int:
        # center must be a roll and sum must be < adjacent_rolls_limit
        CENTER_IDX = 4
        return (window[CENTER_IDX] == ROLL) * (np.sum(window) <= adjacent_rolls_limit)

    # create a new map of accessible rolls
    rpad = np.pad(rolls, pad_width=1, mode='constant', constant_values=EMTPY)
    w = sliding_window_view(rpad, (3, 3))

    result = np.apply_along_axis(
        is_accessible,
        axis=-1,
        arr=w.reshape(*w.shape[:2], -1)
    ).reshape(w.shape[:2])

    return result


def iterate_roll_removal(rolls: np.ndarray, adjacent_rolls_limit: int = 4, show_removables: bool = False, show_remaining: bool = True, max_iterations: int = None) -> np.ndarray:

    # remaining roll stock
    rem_rol = np.copy(rolls)
    # keep on iterating as long as rolls are removed or max iterations is reached
    # -> maximum iterations is initial number of rolls
    max_it_int = max_iterations if max_iterations is not None else np.sum(
        rem_rol)
    for i in range(max_it_int):
        cur_rol = np.sum(rem_rol)
        accessible_rolls = det_accessible_rolls(
            rem_rol, adjacent_rolls_limit=adjacent_rolls_limit)

        if show_removables:
            print("-" * rem_rol.shape[1])
            print(f"Removables (Iteration: {i}):")
            print("-" * rem_rol.shape[1])
            print(roll_array_to_str(accessible_rolls))

        rem_rol[accessible_rolls == 1] = EMTPY

        if show_remaining:
            print("-" * rem_rol.shape[1])
            print(f"Remaining (Iteration: {i}):")
            print("-" * rem_rol.shape[1])
            print(roll_array_to_str(rem_rol))

        if np.sum(rem_rol) >= cur_rol:
            # no rolls removed
            break
        i += 1

    return rem_rol


def det_total_accessible_rolls(rolls: np.ndarray,
                               adjacent_rolls_limit: int = 4,
                               show_removables: bool = False,
                               show_remaining: bool = True,
                               max_iterations: int = None) -> int:
    remaining = iterate_roll_removal(rolls,
                                     adjacent_rolls_limit=adjacent_rolls_limit,
                                     show_remaining=show_remaining,
                                     show_removables=show_removables,
                                     max_iterations=max_iterations)
    return np.sum(rolls) - np.sum(remaining)


if __name__ == "__main__":

    import sys

    print("===== Day 4 =====")

    fname = sys.argv[1] if len(sys.argv) > 1 else "input.txt"

    # Measure file reading performance
    start_read = time.perf_counter()
    rolls = read_input_paper_rolls(fname)
    time_read = (time.perf_counter() - start_read) * 1000
    print(f"Input reading: {time_read:.3f}ms")
    print(f"Grid size: {rolls.shape[0]}x{rolls.shape[1]}")

    print("\n===== Part 1 =====")
    # determine the number of removable rolls in first step
    start_p1 = time.perf_counter()
    accessible_rolls1 = det_total_accessible_rolls(rolls,
                                                   adjacent_rolls_limit=4,
                                                   show_remaining=False,
                                                   show_removables=False,
                                                   max_iterations=1)
    time_p1 = (time.perf_counter() - start_p1) * 1000
    print(f"accessible rolls: {accessible_rolls1}")
    print(f"Time: {time_p1:.3f}ms")

    print("\n===== Part 2 =====")
    # determine the rolls that remain if we remove all removable rolls
    start_p2 = time.perf_counter()
    total_accessible_rolls = det_total_accessible_rolls(rolls,
                                                        adjacent_rolls_limit=4,
                                                        show_remaining=True,
                                                        show_removables=False)
    time_p2 = (time.perf_counter() - start_p2) * 1000
    print(f"total accesible rolls: {total_accessible_rolls}")
    print(f"Time: {time_p2:.3f}ms")
