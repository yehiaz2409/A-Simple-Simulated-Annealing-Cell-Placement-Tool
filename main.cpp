//#include <bits/stdc++.h>
#include <fstream>
#include <iostream>
#include <random>
#include <chrono>
using namespace std;

unordered_map<int, vector<int> >
    cellNets; // contains what nets each cell is a part of
vector<vector<int> > grid, nets;
vector<pair<int, int> > cells; // contains x,y coordinates
vector<int> nets_wire_lengths;
string test_case_file = "tests/t3.txt";
int numCells, numNets, numRows, numCols;
vector<bool> net_change;

void readNetlist(const string &filename, int &numCells, int &numNets,
                 int &numRows, int &numCols, vector<vector<int> > &nets,
                 unordered_map<int, vector<int> > &cellNets) {
  ifstream file(filename);
  if (!file.is_open()) {
    std::cout << "Error opening file." << endl;
    return;
  }

  // Read the first line: numCells, numNets, numRows, numCols
  file >> numCells >> numNets >> numRows >> numCols;
  nets.resize(numNets);

  // Read each subsequent line for nets
  for (int i = 0; i < numNets; i++) {
    int numComponents = 0;
    file >> numComponents;

    for (int j = 0; j < numComponents; j++) {
      int cell;
      file >> cell;
      nets[i].push_back(cell);
      cellNets[cell].push_back(
          i); // see if cell was alr encountered, if not, map will add a new key
    }
    nets[i].resize(numComponents);
  }

  file.close();
  // test

  // cout << "Cells: " << numCells << endl;
  // cout << "Nets: " << numNets << endl;
  // cout << "Rows: " << numRows << endl;
  // cout << "Columns: " << numCols << endl;

  // for (const auto &x : nets) {
  //   cout << "Net with " << x.size() << " components: ";
  //   for (int comp : x) {
  //     cout << comp << " ";
  //   }
  //   cout << endl;
  // }
}

void initial_placement() { // placing cells randomly in the grid
  vector<pair<int, int> > random_indices;
  for (int r = 0; r < numRows; r++) {
    for (int c = 0; c < numCols; c++)
      random_indices.emplace_back(r, c);
  }

  unsigned int seed = 81932; // fixed seed for testing
  shuffle(random_indices.begin(), random_indices.end(),
          default_random_engine(seed));

  for (int i = 0; i < numCells; i++) {
    cells.push_back(random_indices[i]);
    grid[random_indices[i].first][random_indices[i].second] =
        i; // made with the observation that cells' ids go from 0 to numcells-1
  }
}

int calculateWireLengths(
    int net) { // will be used in simulated annealing to calculate
  // the new cost/perimeter each time a swap happens
  int min_x = INT_MAX, min_y = INT_MAX, max_x = INT_MIN, max_y = INT_MIN;
  for (auto cell : nets[net]) {
    min_x = min(min_x, cells[cell].first);
    min_y = min(min_y, cells[cell].second);
    max_x = max(max_x, cells[cell].first);
    max_y = max(max_y, cells[cell].second);
  }
  int length = (max_x - min_x) + (max_y - min_y);
  nets_wire_lengths[net] = length;
  return length;
}

void print_grid() {
  for (int i = 0; i < numRows; i++) {
    for (int j = 0; j < numCols; j++) {
      if (grid[i][j] == -1)
        cout << "-- ";
      else if (grid[i][j] >= 0 && grid[i][j] <= 9)
        cout << "0" << grid[i][j] << " ";
      else
        cout << grid[i][j] << " ";
    }
    cout << endl;
  }
  cout << endl;
}

void print_binary() {
  for (int i = 0; i < numRows; i++) {
    for (int j = 0; j < numCols; j++) {
      if (grid[i][j] == -1)
        cout << 0 << " ";
      else
        cout << 1 << " ";
    }
    cout << endl;
  }
  cout << endl;
}

void simulated_annealing(double &cost, double initial_temp, double final_temp) {
  double temp = initial_temp;
  int moves;
  while (temp > final_temp) {
    moves = 20 * numCells;
    while (moves--) {
      // Get first random cell and make sure it is not an empty cell
      int x = rand() % numCells;
      int row_x = cells[x].first;
      int col_x = cells[x].second;
      // Get second random cell
      int y = rand() % (numRows * numCols);
      int row_y = y / numCols;
      int col_y = y % numCols;

      // Skip if selecting the same cell
      if (row_x == row_y && col_x == col_y)
        continue;
      // Swap the cells
      int x_val = grid[row_x][col_x];
      // cout << x << " " << grid[row_x][col_x] << " " << x_val << " " << row_x
      // << " " << col_x << " " << cells[x].first << " " << cells[x].second << "
      // " << endl;

      // out << x_val << endl;
      int y_val = grid[row_y][col_y]; // can be -1
      if (x != x_val)
        exit(-1);
      grid[row_x][col_x] = y_val;
      grid[row_y][col_y] = x_val;

      for (auto net : cellNets[x_val])
        net_change[net] = true;

      if (y_val != -1) {
        cells[x_val].first = row_y;
        cells[x_val].second = col_y;
        cells[y_val].first = row_x;
        cells[y_val].second = col_x;
        for (auto net : cellNets[y_val])
          net_change[net] = true;
      }
      // swap(cells[x_val], cells[y_val]);
      else {
        cells[x_val].first = row_y;
        cells[x_val].second = col_y;
      }

      double new_cost = 0.0;
      for (int i = 0; i < numNets; i++) {
        if (net_change[i])
          new_cost = new_cost + calculateWireLengths(i);
        else
          new_cost = new_cost + nets_wire_lengths[i];
      }
      // new_cost += calculateWireLengths(
      //     i); // calculate the change in WL (ΔL) due to the swap
      double change = new_cost - cost;
      if (change < 0) { // accept the swapping
        cost = new_cost;
      } else {
        double prob =
            (1 -
             exp((-1 * (change)) /
                 temp)); // reject the swapping with the calculated probability
        if (prob < 0.5) {
          cost = new_cost;
        } else { // reverse the swapping
          grid[row_x][col_x] = x_val;
          grid[row_y][col_y] = y_val;
          cells[x_val].first = row_x;
          cells[x_val].second = col_x;
          swap(cells[x_val], cells[y_val]);
          if (y_val != -1) {
            if (y_val != -1) {
              cells[x_val].first = row_x;
              cells[x_val].second = col_x;
              cells[y_val].first = row_y;
              cells[y_val].second = col_y;
            }
          } else {
            cells[x_val].first = row_x;
            cells[x_val].second = col_x;
          }
        }
      }
    }
    temp *= 0.95; // update the temperature
  }
}

int main() {
  auto start = chrono::high_resolution_clock::now();
  double cost = 0;
  readNetlist(test_case_file, numCells, numNets, numRows, numCols, nets,
              cellNets);

  grid.resize(numRows, vector<int>(numCols, -1));
  nets_wire_lengths.resize(numNets, 0);
  net_change.resize(numNets, false);

  initial_placement(); // giving initial placement to the cells in the grid

  cout << "Initial Grid (binary): " << endl;
  print_binary(); // initial placement is printed in binary
  cout << "Initial Grid (actual): " << endl;
  print_grid();

  for (int i = 0; i < numNets; i++)
    cost += calculateWireLengths(i);
  cout << "Initial cost: " << cost << endl;

  double initial_temp = 500 * cost;
  double final_temp = ((5 * 1e-6) * cost) / numNets;
  double curr_temp = initial_temp;

  cout << "Initial Temp: " << initial_temp << " Current Temp: " << curr_temp
       << " Final Temp: " << final_temp << endl;

  simulated_annealing(cost, initial_temp, final_temp);
  auto end = chrono::high_resolution_clock::now();
  auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
  cout << "Final Grid: " << endl;
  print_grid();
  // cost = 0;
  // or (int i = 0; i < numNets; i++) cost += calculateWireLengths(i);
  cout << "Total wire length: " << cost << endl;
  cout << "Where:\n";
  cout << "• -- : Empty site\n";
  cout << "• DD : The site has the component number DD\n\n";
  cout << "Total time taken: " << duration.count() << " milliseconds\n";
  return 0;
}