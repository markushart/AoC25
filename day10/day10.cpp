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
    using chrono::duration;
    using chrono::duration_cast;
    using chrono::high_resolution_clock;
    using chrono::milliseconds;
    using chrono::time_point;
    using chrono::system_clock;

// build a clock to measure performance time
struct PerfClock {

    time_point<system_clock> t_start;

    void start() {
        t_start = system_clock::now();
    }

    milliseconds get_lap() {
        auto t2 = system_clock::now();
        return duration_cast<milliseconds>(t2 - t_start);
    }

};

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

size_t get_joltage_button_press(vector<joltVec> &joltage, const vector<vector<iVec>> &buts)
{
    size_t n_push = 0;

    return n_push;
}

int main(int argc, char *argv[])
{
    PerfClock clock;
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

        clock.start();
    if (read_input(fname, lights, buttons, joltage))
    {
        cout << "cannot read file " << fname << "\n";
        return -1;
    }

    cout << "read " << lights.size() << " lines in " << clock.get_lap().count() << "(ms)\n";

    // ============== PART 1 ==============

    cout << "=============== PART 1 ===============\n";
    clock.start();
    auto min_push = switch_buttons(lights, buttons);
    cout << "min. amount of button pushes: " << min_push << " discovered in " << clock.get_lap().count() << "(ms)\n";

    // ============== PART 2 ==============

    cout << "=============== PART 2 ===============\n";
    clock.start();
    min_push = get_joltage_button_press(joltage, buttons);
    cout << "min. amount of button pushes: " << min_push << " discovered in " << clock.get_lap().count() << "(ms)\n";


    return 0;
}