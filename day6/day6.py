import numpy as np

from multiprocessing import Pool

DAY = 6

def calc_task_sum_column(operator: str, numbers: np.ndarray) -> int:
    # calc either sum or product of column
    if operator == '+':
        return np.sum(numbers)
    elif operator == '*':
        return np.prod(numbers)
    else:
        raise ValueError("operator must be '+' or '*'")

def calc_task_sum(operators: list[str], numbers: np.ndarray, mp: bool=False) -> int:

    if len(operators) != numbers.shape[1]:
        raise ValueError("there must be one operator less than number columns")
    
    args = ((o, numbers[:, i]) for i, o in enumerate(operators))
    if mp: 
        with Pool(len(operators)) as p:
            p.map(calc_task_sum_column, args)
    else:
        return sum(calc_task_sum_column(o, n) for o, n in args)

''' PART 1'''
def read_input_rowwise(file_path: str = "input.txt") -> tuple[list[str], np.ndarray]:
    with open(file_path, 'r') as f:
        lines = f.readlines()
    # last line contains operators 
    ops = lines.pop().split()
    return np.array([[int(n) for n in l.split()] for l in lines]), ops


''' PART 2'''

def read_input_columnwise(file_path: str="input.txt") -> tuple[list[str], np.ndarray]:
    with open(file_path, 'r') as f:
        lines = f.readlines()

    # last line contains operators 
    ops = lines.pop().split()
    # swap columns and lines to easily split at space
    linew = len(lines[0]) - 1
    linesT = ['' for i in range(linew)]
    for l in lines:
        for i in range(linew):
            linesT[i] += l[i]
    nums = np.array(
        [[int(n) for n in l.split()[::-1]] for l in linesT if not l.isspace()][::1]
    ).reshape((-1, len(lines))).T
    return nums, ops

if __name__ == "__main__":

    import sys
    import time

    print(f"===== Day {DAY} =====")

    fname = sys.argv[1] if len(sys.argv) > 1 else "input.txt"

    for task in range(1,3):
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
        start_p = time.perf_counter()
        result = calc_task_sum(operators, numbers)
        time_p = (time.perf_counter() - start_p) * 1000
        print(f"Sum of cephalopod math task solutions: {result}")
        print(f"Time: {time_p:.3f}ms")

        # # determine the solution to cephalopod math using multiprocessing
        # start_p = time.perf_counter()
        # result = calc_task_sum(operators, numbers, True)
        # time_p = (time.perf_counter() - start_p) * 1000
        # print(f"Sum of cephalopod math task solutions (multiprocessing): {result}")
        # print(f"Time: {time_p:.3f}ms")
