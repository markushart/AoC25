#include <stdio.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <limits>
#include <numeric>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <chrono>

using namespace std;

using grid_line_t = vector<bool>;
using grid_t = vector<grid_line_t>;
using grid_list_t = vector<grid_t>;

struct Grid
{
  grid_t omap;

  Grid(size_t m, size_t n)
  {
    omap.resize(m);

    for (auto &gl : omap)
    {
      // init with false
      gl.resize(n, false);
    }
  }

  size_t get_m() const { return omap.size(); }

  size_t get_n() const
  {
    if (omap.empty())
      return 0;
    else
      return omap.front().size();
  }

  size_t grid_size() const { return get_n() * get_m(); }

  void mirror()
  {
    for (auto &row : omap)
      reverse(row.begin(), row.end());
  }

  void rotate_right()
  {
    // rotate grid 90 degrees
    size_t m = get_m();
    size_t n = get_n();

    grid_t new_map(n, grid_line_t(m));

    for (size_t i = 0; i < m; ++i)
      for (size_t j = 0; j < n; ++j)
        new_map[j][m - 1 - i] = omap[i][j];

    omap.swap(new_map);
  }

  bool operator==(const Grid &other) const
  {
    // compare every element
    return omap == other.omap;
  }

  bool operator!=(const Grid &other) const
  {
    // compare every element
    return omap != other.omap;
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
        g.omap[i][j] = c == '#';
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
       << "max grid size: " << max_gs << "( " << max_gs / 8 + ( ( max_gs % 8 ) > 0 ) << " Byte )" << "\n\n";
}

vector<vector<Grid>> create_grid_transmutations(const vector<Grid> &grids)
{

  vector<vector<Grid>> transmutations;

  // create rotation for 0°, 90°, 180°, 270° and their mutations
  for (const auto &g : grids)
  {

    transmutations.push_back({g});
    auto &tb = transmutations.back();

    auto _g = g,
         mirror_g = g;
    mirror_g.mirror();

    for (auto i = 0; i < 3; ++i)
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
          s += (g.omap[i][j] ? '#' : ' ');
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
  auto transmut = create_grid_transmutations(gifts);

  print_grids(transmut);

  return 0;
}