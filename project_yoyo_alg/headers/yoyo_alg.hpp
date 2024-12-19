#pragma once

#include <list>
#include <mpi.h>

#include "constants.hpp"
#include "Edge.hpp"

class Node;

extern void log(const std::string &msg);

void yoyo(Node &node, const MPI_Comm &commGraph);

void do_vote(const std::map<int, std::list<int> > &receivedIds, int minValue, Node &node, const MPI_Comm &comm);

void do_vote_no(const std::map<int, std::list<int> > &receivedIds, Node &node, const MPI_Comm &comm);

void vote(Vote v, std::list<int> nodes, Node &node, const MPI_Comm &comm);

void setup_phase(Node &node, const MPI_Comm &comm, std::stringstream &out);

void process_source(Node &node, int &leader, const MPI_Comm &comm, std::stringstream &out);

void process_internal(Node &node, const MPI_Comm &comm, std::stringstream &out);

void process_sink(Node &node, const MPI_Comm &comm);

void process_leader(const Node &node, int &leader, const MPI_Comm &comm, std::stringstream &out);

std::map<int, std::list<int> > *receive_ids(const Node &node, const MPI_Comm &comm);

std::pair<int, int> receive_and_process_votes(Node &node, const MPI_Comm &comm, std::stringstream &out);

void send_message(const void *buf, int count, MsgTag msgTag, std::map<int, Edge> *edges,
                  EdgeDirection direction, EdgeState state,
                  const MPI_Comm &comm, std::stringstream &out);

int get_min_from_messages(const std::map<int, std::list<int> > &receivedIds);
