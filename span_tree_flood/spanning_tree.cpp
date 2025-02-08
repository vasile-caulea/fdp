#include <mpi.h>
#include <iostream>
#include <queue>

using namespace std;

#define TAG_MSG 1
#define TAG_PARENT 2
#define TAG_ALREADY 3
#define NO_PARENT -1
#define ROOT_PROCESS 5

#define PROCESS_HAS_PARENT(parent) ((parent) != NO_PARENT)
#define PROCESS_HAS_ONLY_PARENT(nNeighbors) ((nNeighbors - 1) == 0)
#define PROCESS_IS_LEAF_NODE(childrenNumber) (childrenNumber == 0)

bool msg_received_from_all_neighbors(int nc, int no, int pRank, int nNeighbors);
void print_tree(int *parentVector, int size);

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    MPI_Status status;
    MPI_Comm c;

    int nNodes = 6;
    int index[] = {2, 4, 7, 8, 12, 14};
    int edges[] = {4, 5, 2, 4, 1, 3, 4, 2, 0, 1, 2, 5, 0, 4};
    MPI_Graph_create(MPI_COMM_WORLD, nNodes, index, edges, 0, &c);

    int pRank;
    MPI_Comm_rank(MPI_COMM_WORLD, &pRank);

    int nNeighbors;
    MPI_Graph_neighbors_count(c, pRank, &nNeighbors);

    int *neighbors = new int[nNeighbors];
    MPI_Graph_neighbors(c, pRank, nNeighbors, neighbors);

    int *parents = nullptr;
    int parent = NO_PARENT;
    char message;

    if (pRank == ROOT_PROCESS)
    {
        parents = new int[nNodes];
        message = 'W';
        parent = pRank;
        for (int i = 0; i < nNeighbors; ++i)
        {
            MPI_Send(&message, 1, MPI_CHAR, neighbors[i], TAG_MSG, c);
        }
    }

    int nc = 0, no = 0; // nc - number of children, no - number of other
    bool done = false;
    while (!done)
    {
        MPI_Recv(&message, 1, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, c, &status);
        int pj = status.MPI_SOURCE;
        int tag = status.MPI_TAG;

        // cout << "Process= " << pRank << " received msg from " << pj << " with tag " << tag <<  "\n";
        switch (tag)
        {
        case TAG_MSG:
        {
            if (PROCESS_HAS_PARENT(parent))
            {
                MPI_Send(&message, 1, MPI_CHAR, pj, TAG_ALREADY, c);
                break;
            }
            parent = pj;
            MPI_Send(&message, 1, MPI_CHAR, parent, TAG_PARENT, c);

            // if there is only one neighbor and that one is the parent, then exit
            if (PROCESS_HAS_ONLY_PARENT(nNeighbors))
            {
                done = true;
                break;
            }
            for (int i = 0; i < nNeighbors; ++i)
            {
                if (neighbors[i] != parent)
                {
                    MPI_Send(&message, 1, MPI_CHAR, neighbors[i], TAG_MSG, c);
                }
            }
        }
        break;
        case TAG_PARENT:
        {
            nc++;
            done = msg_received_from_all_neighbors(nc, no, pRank, nNeighbors);
        }
        break;
        case TAG_ALREADY:
        {
            no++;
            done = msg_received_from_all_neighbors(nc, no, pRank, nNeighbors);
        }
        break;
        }
    }

    MPI_Barrier(c);

    int buf[2], aux[2];
    buf[0] = pRank;
    buf[1] = parent;

    // step 2, root node collects the parent information from other processes from the spanning tree
    if (PROCESS_IS_LEAF_NODE(nc))
    {
        MPI_Send(&buf, 2, MPI_INT, parent, 0, c);
    }
    else
    {
        int nCReceived = 0;

        while (true)
        {
            MPI_Recv(&aux, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, c, &status);

            if (pRank == ROOT_PROCESS)
            {
                parents[aux[0]] = aux[1]; // the parent of child(aux[0]) is aux[1]
            }
            else
            {
                MPI_Send(&aux, 2, MPI_INT, parent, 0, c);
            }

            if (aux[1] == pRank)
            {
                nCReceived++;
            }

            if (nCReceived == nc)
            {
                break;
            }
        }
        MPI_Send(&buf, 2, MPI_INT, parent, 0, c);
    }

    if (pRank == ROOT_PROCESS)
    {
        parents[pRank] = pRank;
        print_tree(parents, nNodes);
    }

    delete[] neighbors;
    if (parents)
    {
        delete[] parents;
    }
    MPI_Finalize();
}

bool msg_received_from_all_neighbors(int nc, int no, int pRank, int nNeighbors)
{
    return (nc + no + (pRank == ROOT_PROCESS ? 0 : 1)) == nNeighbors;
}

void print_tree(int *parentVector, int size)
{
    int *counts = new int[size]{0};
    int *indexes = new int[size]{0};
    int rootP;

    for (int i = 0; i < size; ++i)
    {
        if (parentVector[i] != i)
        {
            counts[parentVector[i]] += 1;
        }
        else
        {
            rootP = i;
        }
    }

    int **children = new int *[size];
    for (int i = 0; i < size; ++i)
    {
        children[i] = nullptr;
        if (counts[i] != 0)
        {
            children[i] = new int[counts[i]];
        }
    }

    for (int i = 0; i < size; ++i)
    {
        children[parentVector[i]][indexes[parentVector[i]]] = i;
        indexes[parentVector[i]] += 1;
    }

    // printing the tree
    queue<pair<int, int>> q;
    q.push({rootP, 0});

    while (!q.empty())
    {
        pair<int, int> node = q.front();
        q.pop();
        
        for (int i = 0; i < node.second; ++i)
        {
            cout << "\t";
        }
        cout << "- " << node.first << "\n";

        for (int i = 0; i < counts[node.first]; ++i)
        {
            q.push({children[node.first][i], node.second + 1});
        }
    }

    // free the memory
    for (int i = 0; i < size; ++i)
    {
        if (children[i])
        {
            delete[] children[i];
        }
    }
    delete[] children;
    delete[] indexes;
    delete[] counts;
}