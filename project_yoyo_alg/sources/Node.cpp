#include "../headers/Node.hpp"

#include <iostream>


Node::Node(const int pid, const int id, const int num_of_neighbors, const int *neighbors_arr) {
    this->pid = pid;
    this->id = id;
    isActive = true;
    numOfLogicalInEdges = 0;
    numOfLogicalOutEdges = 0;
    nodeType = NO_TYPE;
    numOfLogicalNeighbors = num_of_neighbors;
    neighborsArr = new int[numOfLogicalNeighbors];
    neighbors = new std::map<int, Edge>();
    for (int i = 0; i < numOfLogicalNeighbors; i++) {
        neighborsArr[i] = neighbors_arr[i];
        neighbors->emplace(neighbors_arr[i], Edge{NO_DIRECTION, ACTIVE, UNDEFINED});
    }
}

Node::~Node() {
    delete neighbors;
    delete[] neighborsArr;
}

int Node::get_pid() const {
    return pid;
}

int Node::get_id() const {
    return id;
}

int Node::get_num_of_in_edges() const {
    return numOfLogicalInEdges;
}

int Node::get_num_of_out_edges() const {
    return numOfLogicalOutEdges;
}

int Node::get_num_of_neighbors() const {
    return numOfLogicalNeighbors;
}

int *Node::get_neighbors_arr() const {
    return neighborsArr;
}

std::map<int, Edge> *Node::get_neighbors() const {
    return neighbors;
}

NodeType Node::get_node_type() const {
    return nodeType;
}

void Node::set_pid(const int pid) {
    this->pid = pid;
}

void Node::set_id(const int id) {
    this->id = id;
}

void Node::set_num_of_in_edges(const int num_of_in_edges) {
    numOfLogicalInEdges = num_of_in_edges;
}

void Node::set_num_of_out_edges(const int num_of_out_edges) {
    numOfLogicalOutEdges = num_of_out_edges;
}

bool Node::is_active() const {
    return isActive;
}

void Node::set_is_active(const bool is_active) {
    isActive = is_active;
}

void Node::update_node_type() {
    if (numOfLogicalInEdges == numOfLogicalNeighbors) {
        nodeType = SINK;
    } else if (numOfLogicalOutEdges == numOfLogicalNeighbors) {
        nodeType = SOURCE;
    } else {
        nodeType = INTERNAL;
    }
}

void Node::set_neighbor_direction(const int neighborPID, const int neighborID) {
    Edge &edge = neighbors->at(neighborPID);
    edge.neighbourId = neighborID;
    if (id < edge.neighbourId) {
        edge.direction = OUT;
        ++numOfLogicalOutEdges;
    } else {
        edge.direction = IN;
        ++numOfLogicalInEdges;
    }
}

void Node::reverse_edge_direction(const int neighborPID) {
    if (Edge &edge = neighbors->at(neighborPID); edge.direction == OUT) {
        edge.direction = IN;
        ++numOfLogicalInEdges;
        --numOfLogicalOutEdges;
    } else {
        edge.direction = OUT;
        ++numOfLogicalOutEdges;
        --numOfLogicalInEdges;
    }
}

void Node::prune_in_edge(const int neighborPID) {
    Edge &edge = neighbors->at(neighborPID);
    edge.state = PRUNED;
    --numOfLogicalInEdges;
    --numOfLogicalNeighbors;
}

void Node::prune_out_edge(const int neighborPID, const PruneAction pruneAction) {
    Edge &edge = neighbors->at(neighborPID);
    if (pruneAction == PRUNE) {
        edge.state = PRUNED;
    } else if (pruneAction == PRUNE_WITH_LINK) {
        edge.state = PRUNED_WITH_LINK;
    }
    --numOfLogicalOutEdges;
    --numOfLogicalNeighbors;
}

bool Node::is_leader() const {
    int numOfPrunedEdges = 0;
    for (const auto &[key, value]: *neighbors) {
        if (value.state == PRUNED || value.state == PRUNED_WITH_LINK) {
            ++numOfPrunedEdges;
        }
    }
    std::cout << "Node " << pid << " has " << numOfPrunedEdges << " pruned edges and " << numOfLogicalNeighbors <<
            " neighbors"
            << std::endl;
    return numOfPrunedEdges == neighbors->size();
}

void Node::get_node_state(std::stringstream &out) {
    out << "\nNode state: pid: " << pid << " id: " << id << " edges:\n";
    for (const auto &[key, value]: *neighbors) {
        out << "node: " << key << " id: " << value.neighbourId << ", direction: " << ((value.direction == IN)
                ? "in"
                : "out") << ", state: " << value.state << std::endl;
    }
    out << "Num of in nodes: " << numOfLogicalInEdges << std::endl;
    out << "Num of out nodes: " << numOfLogicalOutEdges << std::endl;
    out << "Num of neighbors: " << numOfLogicalNeighbors << std::endl;
    out << "\n";
}
