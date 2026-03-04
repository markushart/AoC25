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
#include <unordered_set>
#include <chrono>
#include <queue>

using namespace std;

using et = long long;

template <typename T>
T straight_line_dist_squared(const vector<T> &v1, const vector<T> &v2)
{
    T s = 0;
    for (size_t i = 0; i < v1.size(); ++i) {
        T d = v1[i] - v2[i];
        s += d * d;
    }
    return s;
}

using candidate = std::pair<et, size_t>;

template <typename T>
struct Node
{
    constexpr static const size_t END = numeric_limits<size_t>::max();

    vector<T> p{0, 0, 0};
    size_t id = END;

    size_t left = END,
           right = END;
};

template <typename T>
struct NNQuery
{
    struct comp
    {
        bool operator()(const candidate l, const candidate r) const { return l.first < r.first; }
    } custom_less;

    vector<T> p{0, 0, 0};
    size_t n_nearest = 1;
    priority_queue<candidate, vector<candidate>, comp> nearest;
    vector<Node<T>> *nodes = nullptr;

    vector<size_t> final_results;

    NNQuery(size_t n_nearest, vector<Node<T>> *const nodes) : n_nearest{n_nearest}, nodes{nodes}
    {
    }

    void set_p(const vector<T> &p)
    {
        this->p = p;
    }

    bool full() const
    {
        return nearest.size() >= n_nearest;
    }

    bool empty() const
    {
        return nearest.empty();
    }

    void insert(const size_t candidate)
    {
        if (nodes == nullptr)
            return;
        if (candidate >= nodes->size())
            return;

        T dist_sq = straight_line_dist_squared(p, (*nodes)[candidate].p);

        nearest.push({dist_sq, candidate});
        if (nearest.size() > n_nearest)
            nearest.pop();
    }

    array<size_t, 2> get_next_child(const size_t r, const size_t k) const
    {
        if (nodes == nullptr)
            return {Node<T>::END, Node<T>::END};
        if (r >= nodes->size())
            return {nodes->size(), nodes->size()};

        const auto &root = (*nodes)[r];

        if (p[k % p.size()] < root.p[k % p.size()])
            return {root.left, root.right};
        else
            return {root.right, root.left};
    }

    bool should_traverse_other_branch(const size_t r, const size_t k)
    {
        if (nodes == nullptr || r >= nodes->size())
            return false;

        if (!full())
            return true;

        const auto d = (*nodes)[r].p[k % p.size()] - p[k % p.size()];
        return d * d < nearest.top().first;
    }

    void search_nearest_node(size_t r = 0, size_t k = 0)
    {
        if (nodes == nullptr || r >= nodes->size())
            return;

        insert(r);

        const auto branches = get_next_child(r, k);

        search_nearest_node(branches[0], k + 1);

        if (branches[1] < nodes->size())
        {
            if (should_traverse_other_branch(r, k))
            {
                search_nearest_node(branches[1], k + 1);
            }
        }

        if (k == 0)
            finalize_results();
    }

    void finalize_results()
    {
        final_results.clear();
        final_results.reserve(nearest.size());

        while (!nearest.empty())
        {
            final_results.push_back(nearest.top().second);
            nearest.pop();
        }

        reverse(final_results.begin(), final_results.end());
    }

    size_t get_nearest_idx(const size_t n) const
    {
        return final_results.at(n);
    }

    vector<T> &get_nearest_point(const size_t n) const
    {
        return (*nodes)[get_nearest_idx(n)].p;
    }
};

template <typename T>
void read_input(const string &fname, vector<vector<T>> &nums)
{
    ifstream rfile(fname);
    string line;

    if (rfile.is_open())
    {
        while (getline(rfile, line))
        {
            if (line.empty()) continue;
            nums.push_back(vector<T>());
            size_t sep_pos_start = 0, sep_pos_end = 0;

            while (sep_pos_end < line.length())
            {
                sep_pos_end = line.find(',', sep_pos_start);
                if (sep_pos_end == string::npos) sep_pos_end = line.length();

                if (sep_pos_start < sep_pos_end)
                {
                    nums.back().push_back(stoll(line.substr(sep_pos_start, sep_pos_end - sep_pos_start)));
                }
                sep_pos_start = sep_pos_end + 1;
            }
        }
    }
    cout << "Read nums; Size <" << nums.size() << ", " << nums.at(0).size() << ">" << endl;
}

template <typename T, typename K>
struct KNNPairs
{
    struct Edge {
        T dist_sq;
        K u, v;
        bool operator>(const Edge& other) const { return dist_sq > other.dist_sq; }
    };
    priority_queue<Edge, vector<Edge>, greater<Edge>> pq;

    void fill(vector<Node<T>> &nodes, size_t k_neighbors) {
        NNQuery<T> query(k_neighbors + 1, &nodes);
        for (size_t i = 0; i < nodes.size(); ++i) {
            query.set_p(nodes[i].p);
            query.search_nearest_node();
            for (size_t j = 1; j < query.final_results.size(); ++j) {
                K neighbor_idx_in_nodes = query.get_nearest_idx(j);
                K u = (K)nodes[i].id;
                K v = (K)nodes[neighbor_idx_in_nodes].id;
                if (u < v) {
                    pq.push({straight_line_dist_squared(nodes[i].p, nodes[neighbor_idx_in_nodes].p), u, v});
                }
            }
        }
    }

    pair<K, K> pop_closest() {
        auto top = pq.top();
        pq.pop();
        return {top.u, top.v};
    }

    size_t size() const { return pq.size(); }
    bool empty() const { return pq.empty(); }
};

struct DSU {
    vector<int> parent;
    vector<int> sz;
    int num_sets;

    DSU(int n) : num_sets(n) {
        parent.resize(n);
        iota(parent.begin(), parent.end(), 0);
        sz.assign(n, 1);
    }

    int find(int i) {
        if (parent[i] == i)
            return i;
        return parent[i] = find(parent[i]);
    }

    bool unite(int i, int j) {
        int root_i = find(i);
        int root_j = find(j);
        if (root_i != root_j) {
            if (sz[root_i] < sz[root_j])
                swap(root_i, root_j);
            parent[root_j] = root_i;
            sz[root_i] += sz[root_j];
            num_sets--;
            return true;
        }
        return false;
    }
};

template <typename T, typename K>
void group_points(vector<Node<T>> &nodes, list<vector<K>> &groups, const size_t n_connections)
{
    if (nodes.empty()) return;

    KNNPairs<T, K> knn;
    knn.fill(nodes, 20); 

    int max_id = 0;
    for(const auto& n : nodes) if((int)n.id > max_id) max_id = (int)n.id;
    DSU dsu(max_id + 1);

    for (size_t i = 0; i < n_connections && !knn.empty(); ++i)
    {
        auto p = knn.pop_closest();
        dsu.unite(p.first, p.second);
    }

    groups.clear();
    unordered_map<int, vector<K>> components;
    for (const auto& n : nodes) {
        components[dsu.find(n.id)].push_back((K)n.id);
    }
    for (auto& [root, members] : components) {
        groups.push_back(members);
    }

    groups.sort([](const auto &a, const auto &b)
                { return a.size() > b.size(); });
}

template <typename T, typename K>
pair<K, K> group_points_to_n_groups(vector<Node<T>> &nodes, list<vector<K>> &groups, const size_t ngroups)
{
    if (nodes.empty()) return {0, 0};

    struct LazyEdge {
        T dist_sq;
        K u_id;
        K u_idx_in_nodes;
        int next_k;
        bool operator>(const LazyEdge& other) const { return dist_sq > other.dist_sq; }
    };
    priority_queue<LazyEdge, vector<LazyEdge>, greater<LazyEdge>> global_pq;

    // Initialize: Get the 1st nearest neighbor for every point
    for (size_t i = 0; i < nodes.size(); ++i) {
        NNQuery<T> query(2, &nodes); // k=2 to get self (0) and 1st neighbor (1)
        query.set_p(nodes[i].p);
        query.search_nearest_node();
        if (query.final_results.size() > 1) {
            K v_node_idx = query.get_nearest_idx(1);
            global_pq.push({straight_line_dist_squared(nodes[i].p, nodes[v_node_idx].p), (K)nodes[i].id, (K)i, 2});
        }
    }

    int max_id = 0;
    for(const auto& n : nodes) if((int)n.id > max_id) max_id = (int)n.id;
    DSU dsu(max_id + 1);
    
    pair<K, K> last_joined = {0, 0};

    while (!global_pq.empty() && (size_t)dsu.num_sets > ngroups)
    {
        auto top = global_pq.top();
        global_pq.pop();

        K u_id = top.u_id;
        // The current neighbor we are looking at is at query.get_nearest_idx(top.next_k - 1)
        // But we need to run the query again to find out what it was...
        // Better: store the neighbor in the LazyEdge too.
        
        // Re-query to find the actual v_id for the top.next_k - 1 neighbor
        NNQuery<T> query(top.next_k, &nodes);
        query.set_p(nodes[top.u_idx_in_nodes].p);
        query.search_nearest_node();
        
        if (query.final_results.size() >= (size_t)top.next_k) {
            K v_node_idx = query.get_nearest_idx(top.next_k - 1);
            K v_id = (K)nodes[v_node_idx].id;
            
            if (dsu.unite(u_id, v_id)) {
                last_joined = {u_id, v_id};
            }

            // Lazy Expand: Get the NEXT neighbor (top.next_k)
            if (query.final_results.size() < (size_t)(top.next_k + 1)) {
                 // Try searching deeper
                 NNQuery<T> deeper_query(top.next_k + 1, &nodes);
                 deeper_query.set_p(nodes[top.u_idx_in_nodes].p);
                 deeper_query.search_nearest_node();
                 if (deeper_query.final_results.size() >= (size_t)(top.next_k + 1)) {
                     K next_v_node_idx = deeper_query.get_nearest_idx(top.next_k);
                     global_pq.push({straight_line_dist_squared(nodes[top.u_idx_in_nodes].p, nodes[next_v_node_idx].p), 
                                     u_id, top.u_idx_in_nodes, top.next_k + 1});
                 }
            } else {
                 K next_v_node_idx = query.get_nearest_idx(top.next_k);
                 global_pq.push({straight_line_dist_squared(nodes[top.u_idx_in_nodes].p, nodes[next_v_node_idx].p), 
                                 u_id, top.u_idx_in_nodes, top.next_k + 1});
            }
        }
    }

    groups.clear();
    unordered_map<int, vector<K>> components;
    for (const auto& n : nodes) {
        components[dsu.find(n.id)].push_back((K)n.id);
    }
    for (auto& [root, members] : components) {
        groups.push_back(members);
    }

    groups.sort([](const auto &a, const auto &b)
                { return a.size() > b.size(); });
    return last_joined;
}

template <typename T>
size_t insert_kd_tree(list<pair<vector<T>, size_t>> &p,
                      vector<Node<T>> &n,
                      size_t k)
{
    if (p.empty()) return Node<T>::END;

    const size_t d = k % p.front().first.size();

    p.sort([&d](const auto &x1, const auto &x2)
           { return x1.first[d] < x2.first[d]; });

    list<pair<vector<T>, size_t>> rl;
    auto mid_it = next(p.begin(), p.size() / 2);
    rl.splice(rl.begin(), p, mid_it, p.end());

    auto mid_val = rl.front();
    rl.pop_front();

    n.push_back({mid_val.first, mid_val.second});
    const auto ni = n.size() - 1;

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

    list<pair<vector<T>, size_t>> pl;
    for (size_t i = 0; i < p.size(); ++i)
        pl.push_back({p[i], i});

    n.reserve(p.size());
    insert_kd_tree(pl, n, 0);
}

int main(int argc, char *argv[])
{
    using std::chrono::duration_cast;
    using std::chrono::high_resolution_clock;
    using std::chrono::milliseconds;

    string fname = "input.txt";
    if (argc >= 2) fname = argv[1];

    size_t nbiggest = 3;
    if (argc >= 3) nbiggest = stol(argv[2]);

    vector<vector<et>> nums;
    read_input(fname, nums);
    if (nums.empty()) return -1;

    size_t n_connections = 1000;
    if (fname.find("example") != string::npos) n_connections = 10;
    if (argc >= 4) n_connections = stol(argv[3]);

    vector<Node<et>> nodes;
    auto t1 = high_resolution_clock::now();
    build_distance_tree(nums, nodes);
    auto t2 = high_resolution_clock::now();
    cout << "built tree in " << duration_cast<milliseconds>(t2 - t1).count() << "ms\n";

    list<vector<size_t>> groups;
    t1 = high_resolution_clock::now();
    group_points(nodes, groups, n_connections);
    t2 = high_resolution_clock::now();
    cout << "Part 1 grouping: " << duration_cast<milliseconds>(t2 - t1).count() << "ms\n";

    if (nbiggest > groups.size()) nbiggest = groups.size();
    long long bgp = 1;
    auto it = groups.begin();
    for (size_t i = 0; i < nbiggest; ++i, ++it) bgp *= it->size();
    cout << "Part 1 Result: " << bgp << "\n";

    t1 = high_resolution_clock::now();
    auto last_joined = group_points_to_n_groups<et, size_t>(nodes, groups, 1);
    const auto answer2 = nums[last_joined.first][0] * nums[last_joined.second][0];
    t2 = high_resolution_clock::now();
    cout << "Part 2 grouping: " << duration_cast<milliseconds>(t2 - t1).count() << "ms\n";
    cout << "Part 2 Result: " << answer2 << "\n";

    return 0;
}
