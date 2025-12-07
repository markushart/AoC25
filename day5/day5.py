
import numpy as np
import time
from typing import Callable, Any

DAY = 5

def read_input(file_path: str = "input.txt") -> tuple[np.ndarray, np.ndarray]:
    # find empty line, this will seperate id ranges and ids to check
    with open(file_path, 'r') as f:
        nums = [[s.removesuffix('\n') for s in l.split('-')] for l in f]
        # empty line seperates ids and ranges
        sidx = nums.index([''])
        # return ranges, ids
        return np.array(nums[:sidx], dtype=int), \
               np.array(nums[sidx+1:], dtype=int).reshape(-1)


''' PART 1'''
def join_ranges(ranges: np.ndarray) -> np.ndarray:

    # sort by intervall start
    r = np.sort(ranges, axis=0)
    j = np.pad(r[:-1,1] >= r[1:,0], pad_width=1, constant_values=False)
    # elements of r which are kept
    d = ~np.stack((j[:-1], j[1:])).T

    return r[d].reshape((-1,2))

def count_ids_in_ranges(ids: np.ndarray, rgs: np.ndarray) -> int:
    # sort and join ranges that overlap if possible
    r = join_ranges(rgs)
    # upper limits are inclusive
    r[:,1] += 1
    # sorted index
    si = np.sort(ids)
    
    # binary search for upper and lower bound of range in ids
    return np.sum(np.searchsorted(si, r[:, 1]) - np.searchsorted(si, r[:, 0]))

''' PART 2'''
def count_all_ids_in_range(rgs: np.ndarray) -> int:
    # overlapping ids would be count doube, so join
    r = join_ranges(rgs)
    # add shape[0] because range is inclusive
    return np.sum(r[:, 1] - r[:, 0]) + r.shape[0]


if __name__ == "__main__":

    import sys

    print(f"===== Day {DAY} =====")

    # Measure file reading performance
    start_read = time.perf_counter()

    fname = sys.argv[1] if len(sys.argv) > 1 else "input.txt"
    rgs, ids = read_input(fname)

    time_read = (time.perf_counter() - start_read) * 1000
    print(f"Input reading: {time_read:.3f}ms")

    print("\n===== Part 1 =====")
    # determine the number of fresh ingredients 
    start_p1 = time.perf_counter()
    n_fresh = count_ids_in_ranges(ids, rgs)
    time_p1 = (time.perf_counter() - start_p1) * 1000
    print(f"Number of fresh ingredients: {n_fresh}")
    print(f"Time: {time_p1:.3f}ms")

    print("\n===== Part 2 =====")
    # determine the rolls that remain if we remove all removable rolls
    start_p2 = time.perf_counter()
    range_sum = count_all_ids_in_range(rgs)
    time_p2 = (time.perf_counter() - start_p2) * 1000
    print(f"Number of possible fresh ingredients: {range_sum}")
    print(f"Time: {time_p2:.3f}ms")
