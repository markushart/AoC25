import sys
import numpy as np

DAY = 3

NUMPY = False

def read_input_battery_banks(file_path: str) -> list[int]:
    battery_banks = []
    with open(file_path, 'r') as f:
        for line in f:
            battery_banks.append(int(line.strip()))
    return battery_banks

def get_max_joltage(bank: int, n_batteries: int = 2) -> int:

    s = str(bank)
    if len(s) < n_batteries:
        return bank

    max_bats = [0] * n_batteries

    # get the n largest digits
    s_bat = 0 # start index for battery search
    for i_bat in range(0, n_batteries):
        # end index for battery search
        e_bat = len(s) - n_batteries + i_bat + 1
        # print(f"{s_bat}, {i_bat}, {e_bat}, {len(s)}")
        ic_max = 0
        for ic, c in enumerate(s[s_bat:e_bat]):
            if int(s[s_bat + ic_max]) < int(c):
                # index of max char
                ic_max = ic

        # add index to battery list
        max_bats[i_bat] = s_bat + ic_max
        # update start index for next battery search
        s_bat += ic_max + 1

    joltage = int("".join(s[i] for i in max_bats))
    # print(f"bank: {bank}, joltage: {joltage}")
    return joltage

def sum_max_joltage(battery_banks: list[int], n_batteries: int = 2) -> int:
    return sum(get_max_joltage(bank, n_batteries=n_batteries) for bank in battery_banks)

if __name__ == "__main__":

    print(f"--- Day {DAY} ---")

    fname = sys.argv[1] if len(sys.argv) > 1 else "input.txt"

    print(f"Using input file: {fname}")

    batbank = read_input_battery_banks(fname)

    print("===== Part 1 =====")

    if NUMPY:
        pass
    else:
        print(
            f"sum of joltage: { sum_max_joltage(batbank, n_batteries=2) }")

    print("===== Part 2 =====")

    if NUMPY:
        pass
    else:
        print(
            f"sum of joltage: { sum_max_joltage(batbank, n_batteries=12) }")