
def read_input_ranges(fname: str) -> list[tuple[int, int]]:

    with open(fname) as f:
        lines = f.readlines()

    ranges = []
    for line in lines:
        for range in line.strip().split(','):
            parts = range.strip().split('-')
            ranges.append((int(parts[0]), int(parts[1])))

    return ranges


def get_repeating_num_in_range(start: int, end: int, num_sl_max: int = None) -> list[int]:
    repeating_nums = []

    # filter for 'made up' ids:
    """
    You can find the invalid IDs by looking for any ID which is made 
    only of some sequence of digits repeated either twice (Part1) or at least twice (Part2). 
    So, 55 (5 twice), 6464 (64 twice), and 123123 (123 twice) would all be invalid IDs.
    """

    for num in range(start, end + 1):

        s = str(num)
        l = len(s)

        if l < 2:
            continue

        # get max sub string length, either by user or half length of number
        num_sl_max_int = l if num_sl_max is None else num_sl_max

        # iterate over number of sub strings
        for nss in range(2, num_sl_max_int + 1):

            # sub string length
            sl = l // nss

            # skip if sub string length does not divide full length
            if sl * nss != l:
                continue

            # create sub string list
            sp = list(s[i:i+sl] for i in range(0, l, sl))
            if all(x == s[:sl] for x in sp):
                repeating_nums.append(num)
                break

    return repeating_nums


def sum_repeating_nums_in_ranges(ranges: list[tuple[int, int]], num_sl_max: int = None) -> int:

    return sum(n for s, e in ranges for n in get_repeating_num_in_range(s, e, num_sl_max=num_sl_max))


if __name__ == "__main__":
    print("===== Day 2 =====")
    ranges = read_input_ranges("input.txt")

    print("===== Part 1 =====")
    print(
        f"sum of repeating nums: {sum_repeating_nums_in_ranges(ranges, num_sl_max=2)}")

    print("===== Part 2 =====")
    print(f"sum of repeating nums: {sum_repeating_nums_in_ranges(ranges)}")
