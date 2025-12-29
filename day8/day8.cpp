#include <stdio.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <ranges>
#include <limits>

using namespace std;

using et = int;

const size_t NO_GROUP = 0;

struct Node
{

    vector<et> p{0, 0, 0};

    size_t left = numeric_limits<size_t>::max(),
           right = numeric_limits<size_t>::max();
};

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
T straight_line_dist_squared(const vector<T> &v)
{
    T s = 0;

    for (const auto &x : v)
        s += x * x;

    return s;
}

template <typename T>
T straight_line_dist_squared(const vector<T> &v1, const vector<T> &v2)
{
    vector<T> diff;
    vec_diff(v1, v2, diff);
    return straight_line_dist_squared(diff);
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
                d[i][j] = straight_line_dist_squared(n[i], n[j]);
            }
        }
    }
}

template <typename T>
size_t closest_node(const vector<T> &p, const vector<Node> &n, size_t a, size_t b, T min_r = 1)
{
    if (a >= n.size() && b >= n.size())
        return n.size();
    else if (a >= n.size())
        return b;
    else if (b >= n.size())
        return a;
    else
    {
        auto da = straight_line_dist_squared(n[a].p, p),
             db = straight_line_dist_squared(n[b].p, p);

        if (da < db)
        {
            return a;
        }
        else
        {
            return b;
        }
    }
}

struct SearchResult
{
    size_t idx = numeric_limits<size_t>::max();
    bool left = false;
};

template <typename T>
SearchResult insert_kd_tree(const vector<T> &p, const vector<Node> &n, size_t r, size_t k)
{
    if (r >= n.size())
    {
        return {n.size(), false}; // you would need to insert node at end
    }

    const Node &root = n.at(r);
    size_t _k = k % 3,
           nb = 0; // next branch

    // find node where we could insert
    bool l = p[_k] < root.p[_k];
    if (l)
    {
        nb = root.left;
    }
    else
    {
        nb = root.right;
    }

    auto sr = insert_kd_tree(p, n, nb, k + 1);
    if (sr.idx >= n.size())
        return {r, l};
    else
        return sr;
}

template <typename T>
void insert_kd_tree(const vector<T> &p, vector<Node> &n)
{
    if (!n.empty())
    {
        auto sr = insert_kd_tree(p, n, 0, 0);
        if (sr.left)
        {
            n[sr.idx].left = n.size();
        }
        else
        {
            n[sr.idx].right = n.size();
        }
    }
    n.push_back({vector<T>{p[0], p[1], p[2]}});
}

template <typename T>
size_t search_nearest_node(const vector<T> &p, const vector<Node> &n, size_t r = 0, size_t k = 0, T min_r = 1)
{
    if (r >= n.size())
    {
        return n.size(); // you would need to insert node at end
    }

    const Node &root = n.at(r);
    size_t _k = k % 3,
           nb = 0, // next branch
        ob = 0;    // other branch

    // find node where we could insert
    if (p[_k] < root.p[_k])
    {
        nb = root.left;
        ob = root.right;
    }
    else
    {
        nb = root.right;
        ob = root.left;
    }

    auto _i = closest_node(p, n, r, search_nearest_node(p, n, nb, k + 1));

    if (ob < n.size())
    {
        // we need to check if other half of tree needs to be traversed
        auto rad_sq = straight_line_dist_squared(p, n[_i].p);

        T dist = root.p[_k] - p[_k];

        if ((dist * dist) < rad_sq)
        {
            // check if other is closer
            _i = closest_node(p, n, _i, search_nearest_node(p, n, ob, k + 1));
        };
    }

    return _i;
}

template <typename T>
void build_distance_tree(const vector<vector<T>> &p, vector<Node> &n)
{
    if (!p.size())
        return;

    n.reserve(p.size());

    for (auto &_p : p)
    {
        insert_kd_tree(_p, n);
    }
}

int main(int argc, char *argv[])
{

    string fname = "input.txt";
    vector<vector<et>> nums;
    vector<Node> nodes;
    // vector<vector<et>> dist;

    if (argc >= 2)
    {
        fname = argv[1];
        cout << "read file " << fname << endl;
    }

    read_input(fname, nums);

    if (!nums.size())
    {
        return -1;
    }

    // fill_distance_matrix(nums, dist);

    /* ========== PART 1 ========== */

    // search smallest distanze where i != j
    // size_t id = 0, jd = dist.size();
    // for (auto i = 0; i < dist.size(); ++i)
    // {
    //     for (auto j = 0; j < dist.size(); ++j)
    //     {
    //         if (i != j && dist[i][j] < dist[id][jd])
    //         {
    //             id = i;
    //             jd = j;
    //         }
    //     }
    // }

    // cout << "smallest element: d[" << id << "][" << jd << "] = " << dist[id][jd] << "\n";

    build_distance_tree(nums, nodes);

    cout << "inserted " << nodes.size() << "/" << nums.size() << " nodes" << "\n";

    auto in = search_nearest_node(vector({162, 817, 812}), nodes);
    cout << "nearest point is: (" << nodes[in].p[0] << ", " << nodes[in].p[1] << ", " << nodes[in].p[2] << ")\n";

    return 0;
}