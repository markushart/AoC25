import numpy as np
import optlang as ol


def read_input(fname: str = "input.txt"):

    lights, buttons, joltages = [], [], []

    with open(fname, 'r') as f:
        for l in f:
            light, l = l.split('[')[-1].split(']')
            light = [int(i)
                      for i in light.replace('.', '0').replace('#', '1')]
            lights.append(light) 

            butt, l = l.split('{')
            butt = [[int(i) for i in b.strip().removesuffix(')').split(',')] for b in (
                b for b in butt.strip().removeprefix('(').split('('))]
            buttons.append(butt)

            joltage = [int(i) for i in l.split('}')[0].split(',')]
            joltages.append(joltage)

    return lights, buttons, joltages

def buttons_to_arrays(buttons: list[list[int]], j_size: int) -> np.ndarray:

    a = np.zeros((j_size, len(buttons)), dtype=int)
    for i, button in enumerate(buttons):
        for bi in button:
            a[bi, i] = 1

    return a

def solve_lp_to_min(buttons: list[int], joltages: list[int]) -> list[int]:
    s, im = [], 1
    for b, j in zip(buttons, joltages):
        vj = np.array(j)
        A = buttons_to_arrays(b, vj.size)
        x = np.array([ol.Variable("x" + str(i), 
                                  lb=0,
                                  type="integer") for i in range(1, 1 + len(b))])

        obj = ol.Objective(np.ones((x.size,)).dot(x), direction='min')

        c = np.array([ol.Constraint(row, lb=bound, ub=bound) for row, bound in zip(A.dot(x), vj)])

        model = ol.Model(name=f"Model {im}")

        model.objective = obj

        model.add(c)

        if model.optimize() == "optimal":
            s.append([int(v.primal) for vname, v in model.variables.iteritems()])
        else:
            print(f"no solution found for {model.name}")
            s.append([])

        im += 1

    return s


if __name__ == "__main__":

    print("day 10")
    import sys

    fname = "input.txt"
    if len(sys.argv) > 1:
        fname = sys.argv[1]

    lights, buttons, joltages = read_input(fname)


    print("PART 2")

    s = solve_lp_to_min(buttons, joltages) 
    min_push = sum(i for b in s for i in b)
    print(f"min. button pushes to get joltage: {min_push}")