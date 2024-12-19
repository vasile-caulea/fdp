#include "../headers/yoyo_alg.hpp"
#include "../headers/constants.hpp"
#include "../headers/Node.hpp"

#include <algorithm>
#include <list>
#include <map>
#include <sstream>

using namespace std;

void yoyo(Node &node, const MPI_Comm &commGraph) {
    int leader = -1;
    stringstream out;

    setup_phase(node, commGraph, out);

    int iteration = 0;
    while (node.is_active()) {
        iteration++;
        node.update_node_type();

        out << "Iteration " << iteration << "\n";
        out << "Node " << node.get_pid() << " round " << iteration << " node type " << node.get_node_type() << endl;
        node.get_node_state(out);

        if (const NodeType nodeType = node.get_node_type(); nodeType == SOURCE) {
            process_source(node, leader, commGraph, out);
        } else if (nodeType == INTERNAL) {
            process_internal(node, commGraph, out);
        } else if (nodeType == SINK) {
            process_sink(node, commGraph);
        }
        out << "\nInteration " << iteration << " finished. Node is active = " << node.is_active() << "\n";
        log(out.str());
        out.str("");
    }
    process_leader(node, leader, commGraph, out);
    log(out.str());
}

void do_vote(const map<int, list<int> > &receivedIds, const int minValue, Node &node, const MPI_Comm &comm) {
    for (const auto &[id, nodes]: receivedIds) {
        vote(id == minValue ? YES : NO, nodes, node, comm);
    }
}

void do_vote_no(const map<int, list<int> > &receivedIds, Node &node, const MPI_Comm &comm) {
    for (const auto &[id, nodes]: receivedIds) {
        vote(NO, nodes, node, comm);
    }
}

void vote(const Vote v, list<int> nodes, Node &node, const MPI_Comm &comm) {
    int buff[VOTE_BUFF_SIZE];
    buff[INDEX_VOTE_RESULT] = v;

    auto it = nodes.begin();
    if (nodes.size() == node.get_num_of_in_edges() && node.get_num_of_out_edges() == 0) {
        // if the node is a leaf (out edges == 0, only in edges), one edge will be pruned but the link will be kept
        // the other edges will be pruned
        buff[INDEX_PRUNE_RESULT] = PRUNE_WITH_LINK;
        node.prune_in_edge(*it); // because the messages are received on the in-edges
        node.set_is_active(false); // because all the nodes will be pruned (PRUNE and PRUNE_WITH_LINK)
    } else {
        buff[INDEX_PRUNE_RESULT] = NO_PRUNE;
        if (v == NO) {
            node.reverse_edge_direction(*it);
        }
    }

    MPI_Send(&buff, VOTE_BUFF_SIZE, MPI_INT, *it, VOTE, comm);
    ++it;

    // send PRUNE to the others
    buff[INDEX_PRUNE_RESULT] = PRUNE;
    for (; it != nodes.end(); ++it) {
        MPI_Send(&buff, VOTE_BUFF_SIZE, MPI_INT, *it, VOTE, comm);
        // if the vote is NO, it makes no sense to reverse the edge, because we prune the edge
        node.prune_in_edge(*it);
    }
}

void setup_phase(Node &node, const MPI_Comm &comm, stringstream &out) {
    MPI_Status status;
    int idN;
    const int id = node.get_id();
    send_message(&id, 1, SETUP, node.get_neighbors(), NO_DIRECTION, ACTIVE, comm, out);

    for (int i = 0; i < node.get_num_of_neighbors(); ++i) {
        MPI_Recv(&idN, 1, MPI_INT, MPI_ANY_SOURCE, SETUP, comm, &status);
        node.set_neighbor_direction(status.MPI_SOURCE, idN);
    }
}

void process_source(Node &node, int &leader, const MPI_Comm &comm, stringstream &out) {
    const int id = node.get_id();

    // yo- phase
    send_message(&id, 1, MSG, node.get_neighbors(), OUT, ACTIVE, comm, out);

    // -yo phase
    receive_and_process_votes(node, comm, out);

    if (node.is_leader()) {
        node.set_is_active(false);
        leader = node.get_pid();
        send_message(&leader, 1, LEADER, node.get_neighbors(), OUT, PRUNED_WITH_LINK, comm, out);
    }
}

void process_internal(Node &node, const MPI_Comm &comm, stringstream &out) {
    const map<int, list<int> > *receivedIds = receive_ids(node, comm);
    const int minValue = get_min_from_messages(*receivedIds);
    send_message(&minValue, 1, MSG, node.get_neighbors(), OUT, ACTIVE, comm, out);

    // -yo phase
    auto [yesVotes, noVotes] = receive_and_process_votes(node, comm, out);
    if (noVotes > 0) {
        do_vote_no(*receivedIds, node, comm);
    } else {
        do_vote(*receivedIds, minValue, node, comm);
    }
    delete receivedIds;
}

void process_sink(Node &node, const MPI_Comm &comm) {
    const map<int, list<int> > *receivedIds = receive_ids(node, comm);
    const int minValue = get_min_from_messages(*receivedIds);
    do_vote(*receivedIds, minValue, node, comm);
    delete receivedIds;
}

void process_leader(const Node &node, int &leader, const MPI_Comm &comm, stringstream &out) {
    MPI_Status status;
    out << "\nWaiting for leader message\n";
    if (node.get_pid() != leader) {
        MPI_Recv(&leader, 1, MPI_INT, MPI_ANY_SOURCE, LEADER, comm, &status);
        send_message(&leader, 1, LEADER, node.get_neighbors(), OUT, PRUNED_WITH_LINK, comm, out);
    }
    out << "Node " << node.get_pid() << " leader " << leader << endl;
}

map<int, list<int> > *receive_ids(const Node &node, const MPI_Comm &comm) {
    MPI_Status status;
    auto *receivedIds = new map<int, list<int> >();

    int idN;
    for (int i = 0; i < node.get_num_of_in_edges(); ++i) {
        MPI_Recv(&idN, 1, MPI_INT, MPI_ANY_SOURCE, MSG, comm, &status);
        (*receivedIds)[idN].push_back(status.MPI_SOURCE);
    }
    return receivedIds;
}

pair<int, int> receive_and_process_votes(Node &node, const MPI_Comm &comm, stringstream &out) {
    MPI_Status status;
    int yesVotes = 0, noVotes = 0;
    const int outNeighbors = node.get_num_of_out_edges();
    out << "\nNode pid: " << node.get_pid() << ". Out neighbors " << outNeighbors << ". Received votes:\n";
    for (int i = 0; i < outNeighbors; ++i) {
        int voteBuffer[VOTE_BUFF_SIZE];
        MPI_Recv(voteBuffer, VOTE_BUFF_SIZE, MPI_INT, MPI_ANY_SOURCE, VOTE, comm, &status);
        out << "node " << status.MPI_SOURCE << " vote: ";

        if (voteBuffer[INDEX_VOTE_RESULT] == NO) {
            if (voteBuffer[INDEX_PRUNE_RESULT] == NO_PRUNE) {
                node.reverse_edge_direction(status.MPI_SOURCE);
            }
            noVotes++;
            out << "NO";
        } else {
            yesVotes++;
            out << "YES";
        }
        out << " prune: " << (voteBuffer[INDEX_PRUNE_RESULT] == PRUNE
                                  ? "P"
                                  : (voteBuffer[INDEX_PRUNE_RESULT] == PRUNE_WITH_LINK)
                                        ? "P_W_L"
                                        : "NO PRUNE");
        out << "\n";

        if (voteBuffer[INDEX_PRUNE_RESULT] != NO_PRUNE) {
            node.prune_out_edge(status.MPI_SOURCE, static_cast<PruneAction>(voteBuffer[INDEX_PRUNE_RESULT]));
        }
    }
    out << "Finished processing votes\n";
    return make_pair(yesVotes, noVotes);
}

int get_min_from_messages(const map<int, list<int> > &receivedIds) {
    return min_element(receivedIds.begin(), receivedIds.end(),
                       [](const pair<int, list<int> > &a,
                          const pair<int, list<int> > &b) {
                           return a.first < b.first;
                       })->first;
}

void send_message(const void *buf, const int count, const MsgTag msgTag, map<int, Edge> *edges,
                  const EdgeDirection direction, const EdgeState state,
                  const MPI_Comm &comm, stringstream &out) {
    int rank;
    MPI_Comm_rank(comm, &rank);
    out << "Node PID: " << rank << ". Send " << *(static_cast<const int *>(buf)) << " to nodes: ";
    for (const auto &[pid, edge]: *edges) {
        if (edge.direction == direction && edge.state == state) {
            MPI_Send(buf, count, MPI_INT, pid, msgTag, comm);
            out << pid << " ";
        }
    }
    out << "\n";
}
