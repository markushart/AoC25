#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <numeric>
#include <bitset>
#include <unordered_map>

using namespace std;
using idx_t = size_t;
using iVec = vector<idx_t>;
using joltage_t = int;
using joltVec = vector<joltage_t>;

// The manual describes one machine per line.
// Each line contains a single indicator light diagram in [square brackets],
// one or more button wiring schematics in (parentheses),
// and joltage requirements in {curly braces}.
enum DataIndicators
{
    LIGHT_DIAGRAM_BEGIN = '[',
    LIGHT_DIAGRAM_END = ']',
    BUTTON_WIRING_BEGIN = '(',
    BUTTON_WIRING_END = ')',
    JOLTAGE_REQU_BEGIN = '{',
    JOLTAGE_REQU_END = '}',
    LIST_SEP = ','
};

// To start a machine, its indicator lights must match those shown in the diagram, where . means off and # means on.
// The machine has the number of indicator lights shown, but its indicator lights are all initially off.
// So, an indicator light diagram like [.##.] means that the machine has four indicator lights which are initially off
// and that the goal is to simultaneously configure the first light to be off,
// the second light to be on, the third to be on, and the fourth to be off.
constexpr const size_t MAX_LIGHTS = 16;
using LightVec = bitset<MAX_LIGHTS>;

constexpr const char LIGHT_OFF = '.',
                     LIGHT_ON = '#';

LightVec to_LightVec(const string &light_str)
{
    LightVec b(0);

    for (auto i = 0; i < light_str.size() && i < b.size(); ++i)
    {
        switch (light_str[i])
        {
        case LIGHT_OFF:
            // b[light_str.size() - i - 1] = false;
            b[i] = false;
            break;
        case LIGHT_ON:
            // b[light_str.size() - i - 1] = true;
            b[i] = true;
            break;
        default:
            ostringstream oss;
            oss << "Invalid argument: '{}'" << light_str;
            throw invalid_argument(oss.str());
        }
    }
    return b;
}

size_t read_str_between_indicators(const string &line, string &substr, const DataIndicators &start, const DataIndicators &end, const size_t &start_pos = 0)
{
    substr.clear();

    // search start and end
    const auto s = line.find(start, start_pos);
    if (s >= line.length())
    {
        // cout << "Could not find start sign " << start << " in " << line << " starting from " << start_pos << " \n";
        return line.length();
    }

    const auto e = line.find(end, s);
    if (e >= line.length() || s >= e)
    {
        // cout << "Could not find end sign " << end << " in " << line << " starting from " << s << " \n";
        return line.length();
    }

    // extract
    substr = line.substr(s + 1, e - s - 1);

    return e;
}

template <typename T>
int str_to_vec(vector<T> &buf, const string &line, const char sep = DataIndicators::LIST_SEP)
{
    buf.clear();

    for (size_t s = 0; s < line.length();)
    {

        // skip if s is seperator
        if (line[s] == sep)
            ++s;

        auto e = line.find(sep, s);
        if (e > line.length())
            e = line.length();

        buf.push_back(static_cast<T>(stol(line.substr(s, e - s))));
        s = e;
    }

    return buf.size();
}

int read_input(const string &fname, vector<LightVec> &lights, vector<vector<iVec>> &buts, vector<joltVec> &jolt)
{
    ifstream rfile;

    rfile.open(fname);

    if (!rfile.is_open())
    {
        cout << "please provide filename\n";
        return -1;
    }

    string line,
        substr;
    while (getline(rfile, line))
    {
        // add new line to buffers
        lights.push_back({});
        buts.push_back({});
        jolt.push_back({});

        // extract light status
        auto e = read_str_between_indicators(line, substr, DataIndicators::LIGHT_DIAGRAM_BEGIN, DataIndicators::LIGHT_DIAGRAM_END);
        if (e >= line.length())
            return -1;
        lights.back() = to_LightVec(substr);

        // extract button wirings
        auto e_but = e;
        while (true)
        {
            e_but = read_str_between_indicators(line, substr, DataIndicators::BUTTON_WIRING_BEGIN, DataIndicators::BUTTON_WIRING_END, e_but);
            if (e_but >= line.length())
                break;

            iVec butbuf;
            str_to_vec(butbuf, substr);
            buts.back().push_back(butbuf);
        }

        // extract joltage
        e = read_str_between_indicators(line, substr, DataIndicators::JOLTAGE_REQU_BEGIN, DataIndicators::JOLTAGE_REQU_END, e);
        if (e >= line.length())
            return -1;
        str_to_vec(jolt.back(), substr);
    }

    return 0;
}

void sort_by_number_of_lights(vector<LightVec> &lights, vector<vector<iVec>> &buts, vector<joltVec> &jolt)
{
    // build index
    vector<size_t> argi(lights.size());
    iota(argi.begin(), argi.end(), 0);

    // sort by number of lights to switch on
    sort(argi.begin(), argi.end(), [&lights](const auto &a, const auto &b)
         { return lights[a].size() < lights[b].size(); });

    vector<LightVec> temp_light(argi.size());
    vector<vector<iVec>> temp_buts(argi.size());
    vector<joltVec> temp_jolt(argi.size());

    for (size_t i = 0; i < argi.size(); ++i)
    {
        const auto &ia = argi[i];
        temp_light[i] = lights[ia];
        temp_buts[i] = buts[ia];
        temp_jolt[i] = jolt[ia];
    }

    lights = move(temp_light);
    buts = move(temp_buts);
    jolt = move(temp_jolt);
}

LightVec light_indices_to_bitset(const iVec &i_vec)
{
    // [.##.] (3) (1,3) (2) (2,3) (0,2) (0,1) {3,5,4,7}
    //  .##. -> 0110
    //   (3) -> 1000
    //   XOR -> 1110
    // (0,2) -> 0101
    //   XOR -> 1011
    //   (3) -> 1000
    //   XOR -> 0011
    // ---------------
    //  .##. -> 0110
    // (0,2) -> 0101
    //   XOR -> 0011

    LightVec b(0);

    for (const auto &i : i_vec)
    {
        if (i < b.size())
        {
            b[i] = true;
        }
    }

    // cout << b << "\n";

    return b;
}

template <typename T>
bool next_combination(vector<T> &v, const size_t n)
{
    // swap in element in vector past N which is bigger than position x
    // return true while next combination exists, return false if range from begin to end is
    // sorted descending
    // n = 1
    // -----------
    // 0 1 2 3 4 5
    // 1 0 2 3 4 5
    // 2 0 1 3 4 5
    // 3 0 1 2 4 5
    // 4 0 1 2 3 5
    // 5 0 1 2 3 4
    // n = 2
    // -----------
    // 0 1 2 3 4 5
    // 0 2 1 3 4 5
    // 0 3 1 2 4 5
    // 0 4 1 2 3 5
    // 0 5 1 2 3 4
    // -----------
    // 1 0 2 3 4 5
    // 1 2 0 3 4 5
    // 1 3 0 2 4 5
    // 1 4 0 2 3 5
    // 1 5 0 2 3 4
    // ...
    // -----------
    // 5 0 1 2 3 4
    // 5 1 0 2 3 4
    // 5 2 0 1 3 4
    // 5 3 0 1 2 4
    // 5 4 0 1 2 3

    for (auto i = n; i > 0; --i)
    {
        // value to check for swap
        auto it1 = v.begin() + i - 1;
        // value to swap with
        auto it2 = lower_bound(v.begin() + i, v.end(), *it1);

        if (it2 != v.end())
        {
            iter_swap(it1, it2);
            break;
        }
    }

    // check if the first n values are bigger than their neighbors
    bool finished = true;
    for (auto i = 0; i < n; ++i)
        for (auto j = i + 1; j < v.size(); ++j)
            if (v[i] < v[j])
            {
                finished = false;
                break;
            }
    return finished;
}

size_t switch_buttons(const LightVec lights, const LightVec &target, const vector<LightVec> &bitmasks, unordered_map<LightVec, size_t> &ls_cache, const size_t current_best, const size_t d = 1)
{
    if (bitmasks.size() <= 1)
    {
        // TODO: check if last light realy solves
        return d;
    }
    else if (current_best < d)
    {
        // check if we stepped over current best
        return d;
    }

    vector<LightVec> bm;
    bm.reserve(bitmasks.size() - 1);
    auto new_d = d + bitmasks.size();
    for (const auto &b : bitmasks)
    {
        auto new_light = lights ^ b;

        if (new_light == target)
        {
            ls_cache.insert(make_pair(new_light, d));
            return d;
        }
        else if (ls_cache.count(new_light) > 0)
        {
            // check if there is a better combination to reach this light state
            // or update
            if (ls_cache[new_light] <= d)
                continue;
            else
                ls_cache[new_light] = d;
        }
        else
        {
            ls_cache.insert(make_pair(new_light, d));
        }

        // copy and remove current b
        bm.clear();
        for (const auto &_b : bitmasks)
            if (_b != b)
                bm.push_back(_b);

        auto _d = switch_buttons(new_light, target, bm, ls_cache, new_d, d + 1);

        if (_d < new_d)
            new_d = _d;
    }

    return new_d;
}

size_t switch_buttons(const vector<LightVec> &lights, const vector<vector<iVec>> &buts)
{
    vector<vector<LightVec>> bitmasks(buts.size());

    // map light switch indices to bitmasks
    for (auto i = 0; i < buts.size(); ++i)
    {
        bitmasks[i].reserve(buts[i].size());
        for (const auto &ind : buts[i])
            bitmasks[i].push_back(light_indices_to_bitset(ind));
    }

    // try to push buttons to switch all lights on
    size_t num_push = 0;
    for (size_t i = 0; i < lights.size(); ++i)
    {
        unordered_map<LightVec, size_t> ls_cache;
        num_push += switch_buttons(LightVec(0), lights[i], bitmasks[i], ls_cache, bitmasks[i].size());
    }

    return num_push;
}

// PART 2

struct back_track_result
{
    size_t best;
    bool all_failed;
};

template <typename J, typename I>
bool has_val_less_than(const vector<J> &j, const vector<I> &at, const J &val)
{
    return any_of(at.begin(), at.end(), [j, val](const auto &i)
                  { return j[i] < val; });
}

template <typename J>
bool all_eq_val(const vector<J> &j, const J &val = 0)
{
    return all_of(j.begin(), j.end(), [val](const auto &jj)
                  { return jj == val; });
}

template <typename J, typename I>
void add_at_idx(vector<J> &j, const vector<I> &at, const J &val = 1)
{
    for_each(at.begin(), at.end(), [&val, &j](const auto &i)
             { j[i] += val; });
}

back_track_result get_joltage_button_press(joltVec &jolt, const vector<iVec> &buts, size_t current_best = numeric_limits<size_t>::max(), const size_t d = 1)
{
    if (buts.size() <= 1 || d >= current_best)
        return {d, false};

    vector<iVec> b2;
    b2.reserve(buts.size() - 1);
    back_track_result result{current_best, true};

    for (const auto &b : buts)
    {
        // returns true if zero exists in joltage, else false
        if (!has_val_less_than(jolt, b, 1))
        {
            add_at_idx(jolt, b, -1);

            if (all_eq_val(jolt, 0))
                // finished
                return {d, false};

            // copy every value except b
            b2.clear();
            for_each(buts.begin(), buts.end(), [&b2, &b](const auto &bb)
                     { if (b != bb) b2.push_back(bb); });

            auto r = get_joltage_button_press(jolt, b2, result.best, d + 1);
            add_at_idx(jolt, b, 1);

            result.all_failed = r.all_failed;
            if (r.best < result.best && !r.all_failed)
                result.best = r.best;
        }
    }

    return result;
}

size_t get_joltage_button_press(vector<joltVec> &joltage, const vector<vector<iVec>> &buts)
{
    size_t n_push = 0;
    for (auto i = 0; i < joltage.size(); ++i)
    {
        n_push += get_joltage_button_press(joltage[i], buts[i]).best;
    }

    return n_push;
}

int main(int argc, char *argv[])
{
    using std::chrono::duration;
    using std::chrono::duration_cast;
    using std::chrono::high_resolution_clock;
    using std::chrono::milliseconds;

    vector<LightVec> lights;
    vector<vector<iVec>> buttons;
    vector<joltVec> joltage;
    string fname;

    // ============== READ INPUT ==============
    if (argc >= 2)
    {
        fname = argv[1];
        cout << "read file " << fname << "\n";
    }
    else
        return -1;

    if (read_input(fname, lights, buttons, joltage))
    {
        cout << "cannot read file " << fname << "\n";
        return -1;
    }

    cout << "read " << lights.size() << " lines\n";

    // ============== PART 1 ==============

    auto min_push = switch_buttons(lights, buttons);

    cout << "min. amount of button pushes: " << min_push << "\n";

    // ============== PART 2 ==============

    min_push = get_joltage_button_press(joltage, buttons);

    cout << "min. amount of button pushes: " << min_push << "\n";

    return 0;
}