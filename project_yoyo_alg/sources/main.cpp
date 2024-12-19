// mpic++ -o
// mpirun -np 19 --oversubscribe openmpi_yoyo "../testing/graph1.txt"

// https://www.jetbrains.com/help/clion/openmpi.html#build-quick
// https://www.jetbrains.com/help/clion/how-to-use-wsl-development-environment-in-product.html#wsl-general

#include <mpi.h>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "../headers/Node.hpp"
#include "../headers/yoyo_alg.hpp"
using namespace std;

string logFilePath;
std::ofstream logFile;

void init_log(const int pid) {
    stringstream ss;
    ss << "./out/file_pid_" << pid;
    logFilePath = ss.str();
    std::remove(logFilePath.c_str());

    logFile.open(logFilePath, ios_base::app);
}

void log(const string &msg) {
    logFile << msg << endl;
}

void get_communication_graph_topology(char *fileName, int &pid, int &id, MPI_Comm &comm);

int main(int argc, char **argv) {
    MPI_Init(nullptr, nullptr);

    MPI_Comm gr;
    int id, pid, numOfNeighbors, *neighborsPIDs = nullptr;
    get_communication_graph_topology(argv[1], pid, id, gr);

    init_log(pid);

    MPI_Graph_neighbors_count(gr, pid, &numOfNeighbors);
    neighborsPIDs = new int[numOfNeighbors];
    MPI_Graph_neighbors(gr, pid, numOfNeighbors, neighborsPIDs);

    Node node(pid, id, numOfNeighbors, neighborsPIDs);
    delete[] neighborsPIDs;

    yoyo(node, gr);

    logFile.close();
    MPI_Finalize();
}

void get_communication_graph_topology(char *fileName, int &pid, int &id, MPI_Comm &comm) {
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);

    ifstream file;
    file.open(fileName);
    if (!file.is_open()) {
        cerr << "Could not open file " << fileName << ". Exitting ..." << endl;
        exit(EXIT_FAILURE);
    }

    string line;
    map<int, list<int> > mEdges;
    int size = 0;
    int *edges = nullptr;
    int *index = nullptr;

    while (getline(file, line)) {
        line.erase(std::remove(line.begin(), line.end(), ' '), line.end());
        if (line.empty()) {
            continue;
        }
        if (line.find("graph_nodes") != string::npos) {
            size = stoi(line.substr(line.find('-') + 1));
            cout << "\n\n" << size << " nodes" << endl;
        } else if (line.find("neighbors") != string::npos) {
            index = new int[size];
            int i = 0;
            while (i < size) {
                getline(file, line);
                line.erase(std::remove(line.begin(), line.end(), ' '), line.end());

                if (line.empty()) {
                    continue;
                }
                int node = stoi(line.substr(0, line.find('-')));
                stringstream ss(line.substr(line.find('-') + 1));
                string neighbor;
                while (getline(ss, neighbor, ',')) {
                    mEdges[node].push_back(stoi(neighbor));
                }
                index[node] = (node >= 0) ? index[node - 1] : 0;
                index[node] += mEdges[node].size();
                ++i;
            }
        } else if (line.find("ids") != string::npos) {
            int i = 0;
            while (i < size) {
                getline(file, line);
                line.erase(std::remove(line.begin(), line.end(), ' '), line.end());

                if (line.empty()) {
                    continue;
                }
                unsigned long posOfDash = line.find('-');
                if (int node = stoi(line.substr(0, posOfDash)); node == pid) {
                    id = stoi(line.substr(posOfDash + 1));
                    break;
                }
                ++i;
            }
        }
    }
    file.close();
    if (index != nullptr) {
        edges = new int[index[size - 1]];
        int i = 0;
        for (const auto &[nodePID, neighbors]: mEdges) {
            for (const int elem: neighbors) {
                edges[i] = elem;
                ++i;
            }
        }
        MPI_Graph_create(MPI_COMM_WORLD, size, index, edges, 0, &comm);
        delete[] edges;
        delete[] index;
    } else {
        cerr << "Something went wrong. Exitting... " << endl;
        delete[] edges;
        delete[] index;
        exit(EXIT_FAILURE);
    }
}
