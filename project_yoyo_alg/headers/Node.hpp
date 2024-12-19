#pragma once
#include <map>
#include <sstream>

#include "constants.hpp"
#include "Edge.hpp"

class Node {
    int pid;
    int id;
    int numOfLogicalInEdges;
    int numOfLogicalOutEdges;
    int numOfLogicalNeighbors;
    int *neighborsArr;
    bool isActive;
    NodeType nodeType;
    std::map<int, Edge> *neighbors;

public:
    explicit Node(int pid, int id, int num_of_neighbors, const int *neighbors_arr);

    ~Node();

    [[nodiscard]] int get_pid() const;

    [[nodiscard]] int get_id() const;

    [[nodiscard]] bool is_active() const;

    [[nodiscard]] int get_num_of_in_edges() const;

    [[nodiscard]] int get_num_of_out_edges() const;

    [[nodiscard]] int get_num_of_neighbors() const;

    [[nodiscard]] int *get_neighbors_arr() const;

    [[nodiscard]] std::map<int, Edge> *get_neighbors() const;

    [[nodiscard]] NodeType get_node_type() const;

    void set_pid(int pid);

    void set_id(int id);

    void set_num_of_in_edges(int num_of_in_edges);

    void set_num_of_out_edges(int num_of_out_edges);

    void set_is_active(bool is_active);

    void update_node_type();

    void set_neighbor_direction(int neighborPID, int neighborID);

    void reverse_edge_direction(int neighborPID);

    void prune_in_edge(int neighborPID);

    void prune_out_edge(int neighborPID, PruneAction pruneAction);

    [[nodiscard]] bool is_leader() const;

    void get_node_state(std::stringstream &out);
};
