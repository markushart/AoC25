#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <numeric>
#include <bitset>

using namespace std;
using idx_t = size_t;
using iVec = vector<idx_t>;
using joltage_t = unsigned int;
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

    const auto comp_bitmask = [](const LightVec &a, const LightVec &b)
    { return a.to_ulong() < b.to_ulong(); };

    const auto comp_bitmask_r = [](const LightVec &a, const LightVec &b)
    { return a.to_ulong() > b.to_ulong(); };

    // try to push buttons to switch all lights on
    size_t num_push = 0;
    for (size_t i = 0; i < lights.size(); ++i)
    {
        const auto &ls = lights[i]; // light state
        auto &bm = bitmasks[i];     // bit map

        // bitmasks need to be sorted to iter all possible permutations
        sort(bm.begin(), bm.end(), comp_bitmask);

        // if we accumulate the bitmasks at position x,
        // the sum is an even number if the light is OFF
        // and its an odd number if the light is ON
        // effectively this means that applying the
        // same button twice just annulates the first button press
        size_t n_min = bm.size();
        vector<LightVec> bm_comp(bm.size());
        auto iterations = 0;
        do
        {
            // cout << "next iteration; n_min = " << n_min << "\n";
            // if next permutation creates the first n_min positions
            // exactly like in the previous iteration,
            // modify bm and swap in another bitmask
            bool do_masking = true;
            if (n_min < bm.size())
            {
                do_masking = false;
                for (auto j = 0; j < n_min; ++j)
                {
                    if (bm[j] != bm_comp[j])
                    {
                        // vectors are different, do masking
                        do_masking = true;
                        break;
                    }
                }

                if (!do_masking)
                    sort(bm.begin() + n_min, bm.end(), comp_bitmask_r);
            }

            if (do_masking)
            {
                LightVec l(0);
                size_t n = 0;
                for (; l != ls && n < n_min; ++n)
                    l ^= bm[n];

                // set new minimum
                if (n < n_min)
                {
                    n_min = n;
                    bm_comp.resize(n_min);
                }
            }

            // copy to compare vec
            if (n_min < bm.size())
            {
                for (auto j = 0; j < n_min; ++j)
                    bm_comp[j] = bm[j];
            }

            ++iterations;
        } while (next_permutation(bm.begin(), bm.end(), comp_bitmask));

        cout << "lights[" << i << "]: " << ls << "; n min: " << n_min << "; iterations: " << iterations << "\n";

        num_push += n_min;
    }

    return num_push;
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

    // sort_by_number_of_lights(lights, buttons, joltage);

    // ============== PART 1 ==============

    auto min_push = switch_buttons(lights, buttons);

    cout << "min. amount of button pushes: " << min_push << "\n";

    // ============== PART 2 ==============

    return 0;
}