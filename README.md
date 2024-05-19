# Simulated Annealing Cell Placement Tool

## Introduction
Simulated annealing is an algorithm used in the placement stage to find an approximate solution to optimization problems, particularly effective in large and complex search spaces. Inspired by the thermal annealing process, where heated atoms move freely and gradually slow down as they cool, forming stable structures. This concept is applied in a greedy algorithm to allow bigger steps at higher temperatures, reducing step size as temperature decreases. This report reviews the implementation of a simple simulated annealing tool for cell placement, including results evaluation.

## Algorithm
Initially, cells are placed randomly within a grid. The simulated annealing process begins by defining initial and final temperatures, along with a cooling rate. The process continues until the current temperature is less than the final temperature. During each temperature phase, a defined number of moves (iterations) is executed based on the number of cells. In each iteration, two random cells are picked and swapped, and the new cost (wire length) is calculated. If the cost decreases, the swap is accepted; otherwise, a probability calculation determines if the swap should be rejected. This process repeats until the final temperature is reached, concluding with the output of the final grid configuration and the total wire length.

## Implementation

### Data Structure and Definitions
- `string test_case_file = "tests/t1.txt";` Path to the test case data file.
- `double cooling_rate = 0.95;` Cooling rate of the algorithm.
- `unordered_map<int, vector<int>> cellNets;` Maps each cell to its nets for tracking connections.
- `vector<vector<int>> grid, nets;` Grid placements and net connections.
- `vector<pair<int, int>> cells;` Coordinates (x, y) of each cell.
- `vector<int> nets_wire_lengths;` Wire lengths for each net.
- `vector<int> new_wire_lengths;` Temporary storage for new wire lengths during optimization.
- `int numCells, numNets, numRows, numCols;` Grid and netlist dimensions.
- `vector<pair<int, int>> max_net_coordinates, min_net_coordinates;` Tracks net boundaries for HPWL calculations.

### Parsing the Netlist File
Function `readNetlist` reads netlist files with cells, nets, and grid dimensions:
- **Input Format**:
  - First line: number of cells, nets, rows, and columns.
  - Subsequent lines: each line represents a net and lists connected cells.
- **Parameters**:
  - `const string &filename`: Path to the file.
  - `int &numCells, &numNets, &numRows, &numCols`: References to store parsed data.
  - `vector<vector<int>> &nets`: Stores net-cell connections.
  - `unordered_map<int, vector<int>> &cellNets`: Maps cells to their nets.

### Random Initial Placement
Function `initial_placement` places cells randomly within the grid based on available indices, ensuring random initial configurations.

### Calculating Cost/Wire Length
Function calculates wire lengths or cost post-swap to decide on grid updates. Optimization reduces iterations by checking if a swap affects the cost directly.

### Simulated Annealing Algorithm
Executes the annealing process through:
- **Initialization**: Set initial conditions.
- **Temperature Reduction**: Follow the cooling schedule.
- **Iteration**: Execute moves per temperature level.
- **Evaluation**: Assess cost changes per move.
- **Acceptance**: Accept or reject changes based on cost and probability.
- **Termination**: End when temperature is below a threshold.

This iterative process ensures efficient space exploration, gradually settling to a solution as the temperature reduces.
