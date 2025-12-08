import numpy as np

from multiprocessing import Pool

DAY = 6

def calc_task_sum_column(ops: str, numbers: np.ndarray=[]) -> int:
    # calc either sum or product of column
    if ops == '+':
        return np.sum(numbers)
    elif ops == '*':
        return np.prod(numbers)
    else:
        raise ValueError("operator must be '+' or '*'")


def calc_task_sum(operators: list[str], numbers: list[np.ndarray], mp: bool = False, procs: int = 4) -> int:

    if len(operators) != len(numbers):
        raise ValueError("there must be one operator less than number columns")

    if mp:
        opp = len(operators) // procs  # ops per process
        with Pool(procs) as p:
            return sum(p.starmap(calc_task_sum_column, zip(operators, numbers), chunksize=opp))
    else:
        return sum(calc_task_sum_column(o, n) for o, n in zip(operators, numbers))


''' PART 1'''


def read_input_rowwise(file_path: str = "input.txt") -> tuple[list[np.ndarray], list[str]]:
    with open(file_path, 'r') as f:
        lines = f.readlines()
    # last line contains operators
    ops = lines.pop().split()
    numbers = [[int(n) for n in l.split()] for l in lines]
    return list(map(list, zip(*numbers))), ops


''' PART 2'''


def read_input_columnwise(file_path: str = "input.txt") -> tuple[list[np.ndarray], list[str]]:
    with open(file_path, 'r') as f:
        lines = f.readlines()

    # last line contains operators
    ops = lines.pop().split()[::-1]
    # swap columns and lines to easily split at space
    linew = len(lines[0]) - 1
    linesT = ['' for i in range(linew)]
    for l in lines:
        for i in range(linew):
            # order is reversed
            linesT[i] += l[i]

    # search all space lines, these are column seperators
    idx_space = [
        0] + [i for i in range(len(linesT)) if linesT[i].isspace()] + [len(linesT)]
    # nums = [np.array([int(n) for n in linesT[idx_space[-1]:] if not n.isspace()][::-1])]
    nums = []
    for i in range(1, len(idx_space)):
        s, e = idx_space[i - 1], idx_space[i]
        nums.append(np.array([int(n)
                    for n in linesT[s:e] if not n.isspace()][::-1]))
    return nums[::-1], ops


if __name__ == "__main__":

    import sys
    import time

    print(f"===== Day {DAY} =====")

    fname = sys.argv[1] if len(sys.argv) > 1 else "input.txt"
    mp = sys.argv[2] == "--MP" if len(sys.argv) > 2 else False

    for task in range(1, 3):
        print(f"\n===== Part {task} =====")
        # Measure file reading performance
        start_read = time.perf_counter()

        if task == 1:
            numbers, operators = read_input_rowwise(fname)
        elif task == 2:
            numbers, operators = read_input_columnwise(fname)

        time_read = (time.perf_counter() - start_read) * 1000
        print(f"Input reading: {time_read:.3f}ms")

        # determine the solution to cephalopod math
        if not mp:
            start_p = time.perf_counter()
            result = calc_task_sum(operators, numbers)
            time_p = (time.perf_counter() - start_p) * 1000
            print(f"Sum of cephalopod math task solutions: {result}")
            print(f"Time: {time_p:.3f}ms")
        else:
            # determine the solution to cephalopod math using multiprocessing
            start_p = time.perf_counter()
            result = calc_task_sum(operators, numbers, True)
            time_p = (time.perf_counter() - start_p) * 1000
            print(
                f"Sum of cephalopod math task solutions (multiprocessing): {result}")
            print(f"Time: {time_p:.3f}ms")
