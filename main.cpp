//#include <bits/stdc++.h>
#include <fstream>
#include <iostream>
#include <random>
#include <vector>
#include <chrono>
#include <utility>
#include <ctime>
using namespace std;

string test_case_file = "tests/t1.txt";
string grid_file = "grid.txt";
double cooling_rate = 0.95;
random_device rd;
mt19937 gen(rd());
uniform_real_distribution<> dis(0.0, 1.0);
unordered_map<int, vector<int> > cellNets; // contains what nets each cell is a part of
vector<vector<int> > grid, nets;
vector<pair<int, int> > cells; // contains x,y coordinates of cells
vector<int> nets_wire_lengths;
vector<int> new_wire_lengths;
int numCells, numNets, numRows, numCols;
vector<pair<int, int> > max_net_coordinates, min_net_coordinates;
vector<pair<int, int> > new_max_coordinates, new_min_coordinates;
bool update;

void readNetlist(const string &filename, int &numCells, int &numNets, int &numRows, int &numCols, vector<vector<int> > &nets, unordered_map<int, vector<int> > &cellNets) {
  ifstream file(filename);
  if (!file.is_open()) {
    cout << "Error opening file." << endl;
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
}

void initial_placement() { // placing cells randomly in the grid
  vector<pair<int, int> > random_indices;
  for (int r = 0; r < numRows; r++) {
    for (int c = 0; c < numCols; c++)
      random_indices.emplace_back(r, c);
  }
  //srand(static_cast<unsigned int>(std::time(nullptr)));
  mt19937 generator(static_cast<unsigned int>(time(nullptr)));
  unsigned int seed = 12345;
  //rand(); // fixed seed for testing
  shuffle(random_indices.begin(), random_indices.end(), default_random_engine(seed));

  for (int i = 0; i < numCells; i++) {
    cells.push_back(random_indices[i]);
    grid[random_indices[i].first][random_indices[i].second] = i; // made with the observation that cells' ids go from 0 to numcells-1
  }
}

int calculateWireLengths(int net, int cell, vector<int>& wire_lengths, vector<pair<int, int> >& max_coord, vector<pair<int, int> >& min_coord) { // will be used in simulated annealing to calculate
  // the new cost/perimeter each time a swap happens
  if(update == true){ 
    // to optimize, we could save the max and min coordinates of each cell in each net and if the cell
    //being relocated that might change the cost is inside the pre-existing bounds and wont affect the cost
    // then to save the time, we dont re-calculate it
    if((cells[cell].first < max_coord[net].first) && (cells[cell].first > min_coord[net].first) && (cells[cell].second < max_coord[net].second) && (cells[cell].second > min_coord[net].second))
    return wire_lengths[net];
  }
  int min_x = INT_MAX, min_y = INT_MAX, max_x = INT_MIN, max_y = INT_MIN;
  for (auto cell : nets[net]) {
    min_x = min(min_x, cells[cell].first);
    min_y = min(min_y, cells[cell].second);
    max_x = max(max_x, cells[cell].first);
    max_y = max(max_y, cells[cell].second);
  }
  max_coord[net] = make_pair(max_x, max_y);
  min_coord[net] = make_pair(max_x, max_y);

  int length = (max_x - min_x) + (max_y - min_y);
  wire_lengths[net] =  length;
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
}

void outputGridToFile() {
    ofstream outputFile(grid_file, ios_base::app); // append mode
    if (!outputFile.is_open()) {
        std::cerr << "Failed to open the file!" << std::endl;
        return;
    }
    for (const auto& row : grid) {
        for (const auto& cell : row) {
            outputFile << cell << " ";
        }
        outputFile << "\n";
    }
    outputFile << "\n";
    outputFile.close();
}

int count_grid = 1;
void simulated_annealing(double &cost, double initial_temp, double final_temp) {
  // ofstream outputFile("tempVStwl.txt");
  //   if (!outputFile.is_open()) {
  //       cerr << "Failed to open the file!\n";
  //       return;
  //   }
  double temp = initial_temp;
  int moves;
  while (temp > final_temp) {
    moves = 20 * numCells;
    while (moves--) {
      // Get first random cell and make sure it is not an empty cell
      //srand(static_cast<unsigned int>(time(nullptr)));
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
      int y_val = grid[row_y][col_y]; // can be -1
      if (x != x_val) exit(-1);
      grid[row_x][col_x] = y_val;
      grid[row_y][col_y] = x_val;
      int new_cost = cost;
      new_max_coordinates = max_net_coordinates;
      new_min_coordinates = min_net_coordinates;
      new_wire_lengths = nets_wire_lengths;
      update = true; // we are about to check if we can recalc

      if (y_val != -1) { //update x and y's cell positions if it is a cell
        cells[x_val].first = row_y;
        cells[x_val].second = col_y;
        cells[y_val].first = row_x;
        cells[y_val].second = col_x;
        for (auto net : cellNets[y_val]) { 
          //checking new cost
          new_wire_lengths[net] = calculateWireLengths(net, y_val, new_wire_lengths, new_max_coordinates, new_min_coordinates);
          new_cost += new_wire_lengths[net] - nets_wire_lengths[net]; //update the cost if it is to change
        }
      } else { //update only cell x position
        cells[x_val].first = row_y;
        cells[x_val].second = col_y;
      }
      for (auto net : cellNets[x_val]){
       new_wire_lengths[net] = calculateWireLengths(net, x_val, new_wire_lengths, new_max_coordinates, new_min_coordinates);
       new_cost += new_wire_lengths[net] - nets_wire_lengths[net];
      }
      update = false;
      // calculate the change in WL (ΔL) due to the swap
      double change = new_cost - cost;
      if (change < 0) { // accept the swapping
        cost = new_cost;
        nets_wire_lengths = new_wire_lengths;
        max_net_coordinates = new_max_coordinates;
        min_net_coordinates = new_min_coordinates;
        if ((count_grid % 10000) == 0) outputGridToFile();
        count_grid ++;
      } else {
        double prob = (1 - exp((-1 * (change)) / temp)); // reject the swapping with the calculated probability
        double uniform_prob = dis(gen); 
        if (uniform_prob > prob) {
          cost = new_cost;
          nets_wire_lengths = new_wire_lengths;
          max_net_coordinates = new_max_coordinates;
          min_net_coordinates = new_min_coordinates;
          if ((count_grid % 10000) == 0) outputGridToFile();
          count_grid ++;
        } else { // reverse the swapping
          grid[row_x][col_x] = x_val;
          grid[row_y][col_y] = y_val;
          cells[x_val].first = row_x;
          cells[x_val].second = col_x;
          //swap(cells[x_val], cells[y_val]);
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
    //outputFile << temp << " " << cost << endl;
    temp *= cooling_rate; // update the temperature
  }
}

int main() {

  
  double cost = 0;
  readNetlist(test_case_file, numCells, numNets, numRows, numCols, nets, cellNets);

  grid.resize(numRows, vector<int>(numCols, -1));
  nets_wire_lengths.resize(numNets, 0);
  new_wire_lengths.resize(numNets, 0);
  max_net_coordinates.resize(numNets, make_pair(INT_MIN, INT_MIN));
  min_net_coordinates.resize(numNets, make_pair(INT_MAX, INT_MAX));

  initial_placement(); // giving initial placement to the cells in the grid
  
  //cout << "Initial Grid (binary): " << endl;
  //print_binary(); // initial placement is printed in binary
  //cout << "Initial Grid (actual): " << endl;
  //print_grid();
  
  update = false;
  for (int i = 0; i < numNets; i++) cost += calculateWireLengths(i, -1, nets_wire_lengths, max_net_coordinates, min_net_coordinates);
  //cout << "Initial cost: " << cost << endl;

  double initial_temp = 500 * cost;
  double final_temp = ((5 * 1e-6) * cost) / numNets;
  double curr_temp = initial_temp;

  //cout << "Initial Temp: " << initial_temp << " Current Temp: " << curr_temp << " Final Temp: " << final_temp << endl;
  auto start = chrono::high_resolution_clock::now();
  simulated_annealing(cost, initial_temp, final_temp);
  
  auto end = chrono::high_resolution_clock::now();
  auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);

  cout << "Final Grid: " << endl;
  print_grid();
  cout << "Final Binary Grid: " << endl;
  print_binary();
  cout << "Total wire length = " << cost << endl;
  cout << "Where:\n";
  cout << "• -- : Empty site\n";
  cout << "• DD : The site has the component number DD\n\n";
  cout << "Total time taken: " << duration.count() << " milliseconds\n";
  outputGridToFile();
  return 0;
}