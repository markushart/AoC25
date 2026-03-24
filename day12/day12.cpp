#include <stdio.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <chrono>
#include <bitset>
#include <array>

#include <boost/range/combine.hpp>

using namespace std;

constexpr size_t NUMBER_OF_BITS = 2500;
using grid_t = bitset<NUMBER_OF_BITS>;
using grid_list_t = vector<grid_t>;

constexpr size_t X_AXIS = 0, Y_AXIS = 1;

struct Grid
{
  size_t m, // number of rows
      n;    // number of columns
  grid_t omap;

  Grid(const size_t m, const size_t n) : m{m}, n{n}
  {
    // in real world we woule need to check if m*n < NUMBER_OF_BITS
  }

  size_t get_m() const noexcept { return m; }

  size_t get_n() const noexcept { return n; }

  size_t grid_size() const noexcept { return m * n; }

  inline array<size_t, 2> idx_1d_to_2d(const size_t idx) const noexcept
  {
    return {
        idx / n, // i
        idx % n  // j
    };
  }

  inline size_t idx_2d_to_1d(const size_t i, const size_t j) const noexcept
  {
    return n * i + j;
  }

  inline bool get_bit(const size_t i, const size_t j) const noexcept
  {
    return omap[idx_2d_to_1d(i, j)];
  }

  inline void set_bit(const size_t i, const size_t j, const bool b) noexcept
  {
    omap[idx_2d_to_1d(i, j)] = b;
  }

  inline bool get_bit_at(const size_t i, const size_t j) const
  {
    check_bounds(i, j);
    return get_bit(i, j);
  }

  inline void set_bit_at(const size_t i, const size_t j, const bool b)
  {
    check_bounds(i, j);
    set_bit(i, j, b);
  }

  void mirror(const size_t axis = X_AXIS) noexcept
  {
    switch (axis)
    {
    case X_AXIS:
      for (size_t i = 0; i < m / 2; ++i)
      {
        for (size_t j = 0; j < n; ++j)
        {
          const size_t opposite_i = m - 1 - i;

          const bool upper_val = get_bit(i, j),
                     lower_val = get_bit(opposite_i, j);

          set_bit(i, j, lower_val);
          set_bit(opposite_i, j, upper_val);
        }
      }
      break;
    case Y_AXIS:
      for (size_t i = 0; i < m; ++i)
      {
        for (size_t j = 0; j < n / 2; ++j)
        {
          const size_t opposite_j = n - 1 - j;

          const bool left_val = get_bit(i, j),
                     right_val = get_bit(i, opposite_j);

          set_bit(i, j, right_val);
          set_bit(i, opposite_j, left_val);
        }
      }
      break;
    default:
      break;
    }
  }

  void rotate_right()
  {
    Grid rotated(n, m);

    for (size_t i = 0; i < m; ++i)
      for (size_t j = 0; j < n; ++j)
        rotated.set_bit(j, m - i - 1, get_bit(i, j));

    std::swap(*this, rotated);
  }

  bool operator==(const Grid &other) const noexcept
  {
    // compare every element
    return omap == other.omap;
  }

  bool operator!=(const Grid &other) const noexcept
  {
    // compare every element
    return omap != other.omap;
  }

  inline void check_bounds(const size_t i, const size_t j) const
  {
    // check if i or j out of bounds
    if (i >= m || j >= n)
    {
      throw out_of_range(
          "Grid Error: Index out of bounds! "
          "Tried to access (" +
          to_string(i) + ", " + to_string(j) +
          "), but grid is only " + to_string(m) + "x" + to_string(n));
    }
  }

  size_t sum() const
  {
    size_t s = 0;
    for (auto i = 0; i < m; ++i)
      for (auto j = 0; j < m; ++j)
        s += get_bit(i, j);
    return s;
  }

  static Grid from_str_vec(const vector<string> &str_gift)
  {

    size_t m = str_gift.size(),
           n = 0;

    // get max string width
    for (const auto &v : str_gift)
      if (n < v.size())
        n = v.size();

    Grid g(m, n);

    auto i = 0;
    for (const auto &v : str_gift)
    {
      auto j = 0;
      for (const auto &c : v)
      {
        // set to true if hash
        g.set_bit(i, j, c == '#');
        ++j;
      }
      ++i;
    }
    return g;
  }
};

void read_input(const string &fname,
                vector<Grid> &gifts,
                vector<Grid> &grids,
                vector<vector<size_t>> &grid_indizes,
                bool print = true)
{
  ifstream rfile(fname);
  if (!rfile)
    throw runtime_error("cannot open input file");

  string line;
  vector<string> current_gift;

  auto flush_gift = [&]()
  {
    if (!current_gift.empty())
    {
      gifts.push_back(Grid::from_str_vec(current_gift));
      current_gift.clear();
    }
  };

  while (getline(rfile, line))
  {
    if (line.empty())
      continue;

    // ---------- gift row (only '.' and '#')
    if (line.find_first_not_of("#.") == string::npos)
    {
      current_gift.push_back(line);
      continue;
    }

    // ---------- gift header "N:"
    if (isdigit(line.front()) && line.back() == ':' && line.find('x') == string::npos)
    {
      flush_gift();
      continue;
    }

    // ---------- grid definition "MxN: ..."
    auto x_pos = line.find('x');
    auto colon = line.find(':');

    if (x_pos != string::npos && colon != string::npos)
    {
      flush_gift();

      size_t m = stoul(line.substr(0, x_pos));
      size_t n = stoul(line.substr(x_pos + 1, colon - x_pos - 1));

      grids.emplace_back(m, n);

      grid_indizes.emplace_back();

      string numbers = line.substr(colon + 1);
      stringstream ss(numbers);

      size_t id;
      while (ss >> id)
        grid_indizes.back().push_back(id);

      continue;
    }

    // ---------- unknown line
    cerr << "warning: ignoring line: " << line << "\n";
  }

  flush_gift();

  if (!print)
    return;

  for (const auto &gf : gifts)
  {
    cout << "found gift shape " << gf.get_m() << "x" << gf.get_n() << "\n";
  }

  auto max_gs = 0;
  for (size_t i = 0; i < grids.size(); ++i)
  {
    const auto &gr = grids[i];
    cout << "found grid shape " << gr.get_m() << "x" << gr.get_n() << ", index: ";

    if (max_gs < gr.grid_size())
      max_gs = gr.grid_size();

    for (auto idx : grid_indizes[i])
      cout << idx << " ";
    cout << "\n";
  }

  cout << "\n"
       << "max grid size: " << max_gs << "( " << max_gs / 8 + ((max_gs % 8) > 0) << " Byte )" << "\n\n";
}

vector<vector<Grid>> create_grid_transmutations(const vector<Grid> &grids)
{

  vector<vector<Grid>> transmutations;

  // create rotation for 0°, 90°, 180°, 270° and their mutations
  for (const auto &g : grids)
  {

    transmutations.push_back({});
    auto &tb = transmutations.back();

    auto _g = g,
         mirror_g = g;
    mirror_g.mirror();

    for (auto i = 0; i < 4; ++i)
    {
      for (const auto &gr : {_g, mirror_g})
        if (find(tb.begin(), tb.end(), gr) == tb.end())
          tb.push_back(gr);
      _g.rotate_right();
      mirror_g.rotate_right();
    }
  }

  return transmutations;
}

void print_grids(const vector<vector<Grid>> &grids)
{

  vector<string> output;
  vector<vector<string>> blocks;

  if (grids.empty())
    return;

  for (const auto &gr : grids)
  {
    blocks.push_back({});
    auto &block = blocks.back();

    auto gi = 0;
    for (const auto &g : gr)
    {
      for (auto i = 0; i < g.get_m(); ++i)
      {
        if (block.size() <= i)
          block.push_back("|  ");

        auto &s = block[i];
        for (auto j = 0; j < g.get_n(); ++j)
          s += (g.get_bit(i, j) ? '#' : ' ');
        s += "  |";
        if (gi < gr.size() - 1)
          s += "  ";
      }
      ++gi;
    }
  }

  for (auto i = 0; i < blocks.size() + 1; ++i)
  {
    string lfl = "", // last or first line
        sep = "";
    if (i == 0)
      lfl = blocks.front().front();
    else if (i >= blocks.size())
      lfl = blocks.back().back();
    else if (blocks[i - 1].back().size() < blocks[i].front().size())
      lfl = blocks[i].front();
    else
      lfl = blocks[i - 1].back();

    for (const auto &c : lfl)
      if (c == '|')
        sep += "+";
      else
        sep += "-";

    output.push_back(sep);

    if (i < blocks.size())
      for (const auto &l : blocks[i])
        output.push_back(l);
  }

  for (const auto &l : output)
    cout << l << "\n";
}

vector<bool> filter_grid_riddle_by_sums(const vector<Grid> &gifts, const vector<Grid> &grids, const vector<vector<size_t>> &gifts_per_grid)
{

  vector<bool> fits;
  fits.reserve(grids.size());
  vector<size_t> grid_sums;

  transform(gifts.begin(), gifts.end(), back_inserter(grid_sums), [](const auto &g)
            { return g.sum(); });

  for (auto i = 0; i < grids.size(); ++i)
  {
    auto &grid = grids[i];
    auto &gpg = gifts_per_grid.at(i);

    vector<size_t> gift_idx(gpg.size());
    iota(gift_idx.begin(), gift_idx.end(), 0);

    fits.push_back(transform_reduce(gift_idx.begin(), gift_idx.end(), 0, [](const auto &a, const auto &b)
                                    { return a + b; }, [&gpg, &grid_sums](const auto &i)
                                    { return gpg[i] * grid_sums[i]; }) <= grid.grid_size());
  }

  return fits;
}

int main(int argc, char **argv)
{
  string fname = "input.txt";
  vector<Grid> gifts,
      grids;
  vector<vector<size_t>> gifts_per_grid;

  // read input
  if (argc > 1)
    fname = argv[1];

  read_input(fname, gifts, grids, gifts_per_grid);

  // solve riddle
  const auto transmut = create_grid_transmutations(gifts);

  print_grids(transmut);

  // ================================== PART1 ==================================
  // as it turns out on reddit, simply checking sums is enough...
  const auto fits = filter_grid_riddle_by_sums(gifts, grids, gifts_per_grid);

  cout << "Trivial test allows " << accumulate(fits.begin(), fits.end(), 0) << " to be fit into the grids\n";

  // ================================== PART2 ==================================

  return 0;
}