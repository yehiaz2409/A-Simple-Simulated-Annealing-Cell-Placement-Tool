#include <bits/stdc++.h>
#include <iostream>
#include <fstream>
using namespace std;

string test_case_file = "tests/d0.txt";


struct Net {
    int numComponents;
    std::vector<int> components;
};

void readNetlist(const std::string &filename, int &numCells, int &numNets, int &numRows, int &numCols, vector<Net> &nets) {
   ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "Error opening file." << endl;
        return;
    }

    // Read the first line: numCells, numNets, numRows, numCols
    file >> numCells >> numNets >> numRows >> numCols;

    // Read each subsequent line for nets
    for (int i = 0; i < numNets; i++) {
        Net net;
        file >> net.numComponents;
        net.components.resize(net.numComponents);

        for (int j = 0; j < net.numComponents; j++) {
            file >> net.components[j];
        }

        nets.push_back(net);
    }

    file.close();
}


int main(){
    int numCells;
    int numNets;
    int numRows;
    int numCols;
    vector<Net> nets;

    readNetlist(test_case_file, numCells, numNets, numRows, numCols, nets);

    cout << "Cells: " << numCells << endl;
    cout << "Nets: " << numNets << endl;
    cout << "Rows: " << numRows << endl;
    cout << "Columns: " << numCols << endl;

    for (const auto &net : nets) {
        cout << "Net with " << net.numComponents << " components: ";
        for (int comp : net.components) {
            cout << comp << " ";
        }
        cout << endl;
    }

    return 0;
    
}