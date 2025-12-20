#include <stdio.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <ranges>

using namespace std;

using et = unsigned int;

const size_t NO_GROUP = 0;

template <typename T>
void read_input(const string &fname, vector<vector<T>> &nums)
{
    ifstream rfile;
    string line;

    rfile.open(fname);

    if (rfile.is_open())
    {
        while (getline(rfile, line))
        {
            nums.push_back(vector<T>());
            size_t sep_pos_start = 0, sep_pos_end = 0;

            while (sep_pos_end < line.length())
            {

                sep_pos_end = line.find(',', sep_pos_start + 1);

                sep_pos_end = (sep_pos_end > line.length()) ? line.length() : sep_pos_end;

                if (sep_pos_start < sep_pos_end)
                {
                    nums.back().push_back(stoi(line.substr(sep_pos_start, sep_pos_end - sep_pos_start)));
                }
                sep_pos_start = sep_pos_end + 1;
            }
        }
        rfile.close();
    }

    cout << "Read nums; Size <" << nums.size() << ", " << nums.at(0).size() << ">" << endl;
}

template <typename T>
T straight_line_dist_squared(const vector<T> &v)
{
    T s = 0;

    for (auto x : v)
        s += x * x;

    return s;
}

template <typename T>
void vec_diff(const vector<T> &a, const vector<T> &b, vector<T> &x)
{
    if (a.size() != b.size())
    {
        return;
    }

    x.resize(a.size());

    for (auto i = 0; i < a.size(); ++i)
        x[i] = a[i] - b[i];
}

template <typename T>
void fill_distance_matrix(const vector<vector<T>> &n, vector<vector<T>> &d)
{
    // build distance matrix
    d.resize(n.size());
    for (auto i = 0; i < n.size(); ++i)
    {
        d[i].resize(n.size());
        for (auto j = 0; j < n.size(); ++j)
        {
            if (i == j)
            {
                d[i][j] = 0;
            }
            else
            {
                vector<et> diff;
                vec_diff(n[i], n[j], diff);
                d[i][j] = straight_line_dist_squared(diff);
            }
        }
    }
}

int main(int argc, char *argv[])
{

    string fname = "input.txt";

    if (argc >= 2)
    {
        fname = argv[1];
        cout << "read file " << fname << endl;
    }

    vector<vector<et>> nums;
    read_input(fname, nums);

    vector<vector<et>> dist;
    fill_distance_matrix(nums, dist);

    /* ========== PART 1 ========== */

    // search smallest distanze where i != j
    size_t id = 0, jd = dist.size();
    for (auto i = 0; i < dist.size(); ++i)
    {
        for (auto j = 0; j < dist.size(); ++j)
        {
            if (i != j && dist[i][j] < dist[id][jd])
            {
                id = i;
                jd = j;
            }
        }
    }

    cout << "smallest element: d[" << id << "][" << jd << "] = " << dist[id][jd] << "\n";

    return 0;
}