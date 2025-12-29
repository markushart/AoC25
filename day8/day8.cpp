#include <stdio.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <list>
#include <ranges>
#include <limits>
#include <numeric>
#include <algorithm>
#include <array>

using namespace std;

using et = long;

const size_t NO_GROUP = 0;

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
struct Node
{

    vector<T> p{0, 0, 0};

    size_t left = numeric_limits<size_t>::max(),
           right = numeric_limits<size_t>::max();
};

template <typename T>
struct NNQuery
{
    vector<T> p{0, 0, 0};
    size_t n_nearest = 1;
    vector<size_t> nearest;
    vector<Node<T>> *nodes = nullptr;

    NNQuery(const vector<T> &p, size_t n_nearest, vector<Node<T>> *nodes) : p{p}, n_nearest{n_nearest}, nodes{nodes}
    {
        this->nearest.reserve(this->n_nearest + 1);
    }

    bool full() const
    {
        return nearest.size() >= n_nearest;
    }

    bool empty() const
    {
        return nearest.empty();
    }

    void insert(size_t candidate)
    {
        if (nodes == nullptr)
            return;
        if (candidate >= nodes->size())
            return;

        if (empty())
            nearest.push_back(candidate);
        else
        {
            // insert candidates sorted by distance to target point
            auto i = lower_bound(nearest.begin(), nearest.end(), candidate, [&](auto &x1, auto &x2)
                                 { return straight_line_dist_squared(p, (*nodes)[x1].p) <
                                          straight_line_dist_squared(p, (*nodes)[x2].p); });

            if (i < nearest.end())
            {
                nearest.insert(i, candidate);
                if (full())
                    nearest.resize(n_nearest);
            }
        }
    }

    array<size_t, 2> get_next_child(const size_t r, const size_t k) const
    {

        if (nodes == nullptr)
            return {numeric_limits<T>::max(), numeric_limits<T>::max()};
        if (r >= nodes->size())
            return {nodes->size(), nodes->size()};

        const auto &root = (*nodes)[r];

        // find next branch
        if (p[k % p.size()] < root.p[k % p.size()])
            return {root.left, root.right};
        else
            return {root.left, root.right};
    }

    bool should_traverse_other_branch(const size_t r, const size_t k)
    {
        if (nodes == nullptr)
            return false;
        else if (r >= nodes->size())
            return false;
        else if (!full())
            return true;
        else
        {
            // we need to check if other half of tree needs to be traversed
            const auto d = (*nodes)[r].p[k % p.size()] - p[k % p.size()];
            for (auto &i : nearest)
            {
                auto rad_sq = straight_line_dist_squared(p, (*nodes)[i].p);

                if (d * d < rad_sq)
                {
                    return true;
                };
            }
            return false;
        }
    }

    void search_nearest_node(size_t r = 0, size_t k = 0)
    {
        if (nodes == nullptr)
            return;
        else if (r >= nodes->size())
            // reached a leaf
            return;

        insert(r);

        auto branches = get_next_child(r, k);

        // check if r nearer than any of the nearest
        search_nearest_node(branches[0], k + 1);

        if (branches[1] < nodes->size())
        {

            // check if other is closer
            if (should_traverse_other_branch(r, k))
            {
                search_nearest_node(branches[1], k + 1);
            }
        }
    }

    size_t get_nearest_idx(const size_t n) const
    {
        return nearest.at(n);
    }

    vector<T> &get_nearest_point(const size_t n) const
    {
        return (*nodes)[get_nearest_idx(n)].p;
    }
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

struct SearchResult
{
    size_t idx = numeric_limits<size_t>::max();
    bool left = false;
};

template <typename T>
size_t insert_kd_tree(list<vector<T>> &p,
                      vector<Node<T>> &n,
                      size_t k)
{

    const size_t d = k % p.front().size();

    // sort points by current dimension
    p.sort([&d](const auto &x1, const auto &x2)
           { return x1[d] < x2[d]; });

    // split into left and right list
    list<vector<T>> rl;
    rl.splice(rl.begin(),
              p,
              next(p.begin(), p.size() / 2),
              p.end());

    // right list first element is point to insert
    n.push_back({rl.front()});
    const auto ni = n.size() - 1; // save index of new node
    rl.pop_front();

    if (!p.empty())
    {
        n[ni].left = insert_kd_tree(p, n, k + 1);
    }
    if (!rl.empty())
    {
        n[ni].right = insert_kd_tree(rl, n, k + 1);
    }

    return ni;
}

template <typename T>
void build_distance_tree(const vector<vector<T>> &p, vector<Node<T>> &n)
{
    if (!p.size())
        return;

    list<vector<T>> pl;
    for (const auto &x : p)
        pl.push_back(x);

    n.reserve(p.size());

    insert_kd_tree(pl, n, 0);
}

int main(int argc, char *argv[])
{

    string fname = "input.txt";
    vector<vector<et>> nums;
    vector<Node<et>> nodes;
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

    // auto in = search_nearest_node(, nodes);
    NNQuery query(vector<et>({162, 817, 812}), 10, &nodes);
    query.search_nearest_node();
    for (auto i = 0; i < query.nearest.size(); ++i)
    {
        const auto &np = query.get_nearest_point(i);
        cout << "nearest point is: (" << np[0] << ", " << np[1] << ", " << np[2] << "); ";
        cout << "distance: " << straight_line_dist_squared(query.p, np) << "\n";
    }
    return 0;
}