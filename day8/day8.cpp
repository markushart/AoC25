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
#include <unordered_map>
#include <chrono>

using namespace std;

using et = long;

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

    NNQuery(const vector<T> &p, size_t n_nearest, vector<Node<T>> *const nodes) : p{p}, n_nearest{n_nearest}, nodes{nodes}
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

template <typename T, typename K>
struct SortedDistancePairs
{
    // contains the points of the distance matrix
    vector<array<K, 2>> dp;
    vector<K> index;

    void fill(const vector<vector<T>> &dist)
    {
        dp.clear();

        index.resize(dist.size());
        iota(index.begin(), index.end(), 0);

        // we do not need to store diagonal and upper half of distance matrix
        dp.reserve(dist.size() * (dist.size() - 1) / 2);
        for (K i = 0; i < dist.size() - 1; ++i)
            for (K j = i + 1; j < dist.size(); ++j)
                dp.push_back({i, j});

        // for (const auto &d : dp)
        //     cout << "dist[" << d[0] << "][" << d[1] << "]" << dist[d[0]][d[1]] << "\n";

        sort(dist);
    }

    void sort(const vector<vector<T>> &dist)
    {
        std::sort(dp.begin(), dp.end(), [&](const auto &a, const auto &b)
                  { return dist[a[0]][a[1]] < dist[b[0]][b[1]]; });
    }

    pair<K, K> get_pair(const size_t i) const
    {
        return make_pair(dp[i][0], dp[i][1]);
    }

    size_t size() const { return dp.size(); }

    bool empty() const { return dp.empty(); }

    size_t index_size() const { return index.size(); }
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

template <typename T, typename K>
pair<K, K> get_closest_pair(const vector<vector<T>> &dist, const T min_dist)
{
    auto p = make_pair<K, K>(static_cast<K>(dist.size() > 0), 0);

    auto new_min_dist = numeric_limits<T>::max();
    for (auto i = 0; i < dist.size(); ++i)
        for (auto j = 0; j < dist[i].size(); ++j)
            if (dist[i][j] < new_min_dist && min_dist < dist[i][j])
            {
                new_min_dist = dist[i][j];
                p = make_pair(i, j);
            }
    return p;
}

template <typename K>
typename list<vector<K>>::iterator get_point_in_group(list<vector<K>> &groups, const K p)
{
    const auto q = [&p](const vector<K> &g)
    {
        auto it = lower_bound(g.begin(), g.end(), p);
        if (it != g.end())
            if (*it == p)
                return true;
        return false;
    };

    return find_if(groups.begin(), groups.end(), q);
}

template <typename K>
int join_groups(typename list<vector<K>>::iterator g1, typename list<vector<K>>::iterator g2, list<vector<K>> &g)
{
    if (g1 == g.end() || g2 == g.end())
    {
        // something went wrong
        return -1;
    }
    else if (g1 != g2)
    {
        // found both points, merge smaller group into bigger group
        if (g1->size() < g2->size())
            swap(g1, g2);
        for (const auto &x : *g2)
            g1->insert(lower_bound(g1->begin(), g1->end(), x), x);
        g.erase(g2);
    }
    return 0;
}

template <typename T, typename K>
void group_points(const SortedDistancePairs<T, K> &dist, list<vector<K>> &groups, const size_t ndist)
{

    if (dist.empty() || ndist > dist.size())
        // max number of distances possible reached
        return;

    // init groups where every group contains one point
    groups.clear();
    for (const auto &i : dist.index)
        groups.push_back({i});

    for (size_t bi = 0; bi < ndist; ++bi)
    {

        const auto &p = dist.get_pair(bi);

        // search for points in existing groups
        if (join_groups(get_point_in_group(groups, p.first),
                        get_point_in_group(groups, p.second),
                        groups) != 0)
        {
            cout << "Error joining groups\n";
            return;
        }
    }

    groups.sort([](const auto &a, const auto &b)
                { return a.size() > b.size(); });
}

template <typename T, typename K>
pair<K, K> group_points_to_n_groups(const SortedDistancePairs<T, K> &dist, list<vector<K>> &groups, const size_t ngroups)
{

    if (dist.empty() || ngroups > dist.size())
        return make_pair<K, K>(0, 0);

    // init groups where every group contains one point
    groups.clear();
    for (const auto &i : dist.index)
        groups.push_back({i});

    auto p = dist.get_pair(0);
    for (size_t bi = 0; bi < dist.size(); ++bi)
    {

        // get next closest distance
        p = dist.get_pair(bi);

        // search for points in existing groups
        if (join_groups(get_point_in_group(groups, p.first),
                        get_point_in_group(groups, p.second),
                        groups) != 0)
        {
            cout << "Error joining groups\n";
            return make_pair<K, K>(0, 0);
        }

        if (groups.size() <= ngroups)
            break;
    }

    groups.sort([](const auto &a, const auto &b)
                { return a.size() > b.size(); });
    return p;
}

int main(int argc, char *argv[])
{

    using std::chrono::duration;
    using std::chrono::duration_cast;
    using std::chrono::high_resolution_clock;
    using std::chrono::milliseconds;

    string fname = "input.txt";
    vector<vector<et>> nums;
    vector<Node<et>> nodes;
    vector<vector<et>> dist;
    SortedDistancePairs<et, size_t> sdp;

    if (argc >= 2)
    {
        fname = argv[1];
        cout << "read file " << fname << endl;
    }

    size_t nbiggest = 3;
    if (argc >= 3)
    {
        nbiggest = stol(argv[2]);
    }

    read_input(fname, nums);

    if (!nums.size())
    {
        return -1;
    }

    size_t ndist = nums.size();
    if (argc >= 4)
    {
        ndist = stol(argv[3]);
        if (ndist > nums.size())
            ndist = nums.size();
    }

    auto t1 = high_resolution_clock::now();
    fill_distance_matrix(nums, dist);
    sdp.fill(dist);
    auto t2 = high_resolution_clock::now();
    auto ms_read = duration_cast<milliseconds>(t2 - t1);
    cout << "time for reading and distance matrix: " << ms_read.count() << "(ms)\n";

    /* ========== PART 1 ========== */

    // build kd-tree (not used in this example, maybe use later for optimization)
    build_distance_tree(nums, nodes);
    cout << "inserted " << nodes.size() << "/" << nums.size() << " nodes" << "\n";

    // group the points
    list<vector<size_t>> groups;

    t1 = high_resolution_clock::now();
    group_points(sdp, groups, ndist);
    t2 = high_resolution_clock::now();
    auto ms_group1 = duration_cast<milliseconds>(t2 - t1);
    cout << "time for grouping (Part 1): " << ms_group1.count() << "(ms)\n";

    // eval biggest groups
    if (nbiggest >= groups.size())
        nbiggest = groups.size();

    cout << "get (" << nbiggest << "/" << groups.size() << ") biggest groups\n";

    // multiply biggest group sizes
    size_t bgp = 1;
    for (auto it = groups.begin(); it != next(groups.begin(), nbiggest); ++it)
        bgp *= it->size();

    cout << "Product of sizes of " << nbiggest << " biggest groups: " << bgp << "\n";

    /* ========== PART 2 ========== */
    t1 = high_resolution_clock::now();
    const size_t ngroups = 1;
    auto last_joined = group_points_to_n_groups(sdp, groups, ngroups);
    // answer to part 2: product of last joined points x axis
    const auto answer2 = nums[last_joined.first][0] * nums[last_joined.second][0];
    t2 = high_resolution_clock::now();
    auto ms_group2 = duration_cast<milliseconds>(t2 - t1);
    cout << "time for grouping (Part 2): " << ms_group2.count() << "(ms)\n";

    if (groups.size() != ngroups)
    {
        cout << "Error: could not group into 2 groups\n";
        return -1;
    }
    else if (last_joined.first == 0 && last_joined.second == 0)
    {
        cout << "Error: last joined point pair is invalid\n";
        return -1;
    }

    cout << "Last joined point pair:\n";
    cout << last_joined.first << " {" << nums[last_joined.first][0] << ", " << nums[last_joined.first][1] << ", " << nums[last_joined.first][2] << "}\n";
    cout << last_joined.second << " {" << nums[last_joined.second][0] << ", " << nums[last_joined.second][1] << ", " << nums[last_joined.second][2] << "}\n";
    cout << "Answer to part 2: " << answer2 << "\n";

    return 0;
}