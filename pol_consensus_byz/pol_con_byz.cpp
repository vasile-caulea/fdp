#include <iostream>
#include <mpi.h>
#include <cstdlib>
#include <time.h>

using namespace std;
#define FAULTY_NODE 1

enum Action
{
    ATTACK,
    DEFENSE,
    RETREAT
};

const char *actionToString(int action);
void broadcastMessage(int currentP, int n, const void *buf, int count);
void broadcastFaultyMessage(int currentP, int n);

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int pRank, n;

    MPI_Comm_rank(MPI_COMM_WORLD, &pRank);
    MPI_Comm_size(MPI_COMM_WORLD, &n);

    srand(time(NULL));

    MPI_Status status;
    Action implicitAction = ATTACK;

    int noDefects = 1;
    int decision[5] = {DEFENSE, ATTACK, RETREAT, DEFENSE, DEFENSE};
    int votes[3];
    int buff;

    cout << "node " << pRank << ". initial decision: " << actionToString(decision[pRank]) << "\n";

    for (int k = 0; k < noDefects; ++k)
    {
        if (pRank != FAULTY_NODE)
        {
            broadcastMessage(pRank, n, &decision[pRank], 1);
        }
        else
        {
            broadcastFaultyMessage(pRank, n);
        }

        for (int i = 0; i < n - 1; ++i)
        {
            MPI_Recv(&buff, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            decision[status.MPI_SOURCE] = buff;
            votes[buff]++;
        }

        int max = votes[0], maxAp = 0, maj = 0;
        for (int i = 1; i < 3; ++i)
        {
            if (max < votes[i])
            {
                max = votes[i];
                maxAp = 0;
                maj = i;
            }
            else if (max == votes[i])
            {
                maxAp++;
                break;
            }
        }

        if (maxAp != 0)
        {
            maj = implicitAction;
        }
        int mult = votes[maj];

        if (pRank == k)
        {
            if (pRank != FAULTY_NODE)
            {
                broadcastMessage(pRank, n, &maj, 1);
            }
            else
            {
                broadcastFaultyMessage(pRank, n);
            }
        }
        else
        {
            // receive king_majority
            MPI_Recv(&buff, 1, MPI_INT, k, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        }

        if (mult > (n / 2 + noDefects))
        {
            decision[pRank] = maj;
        }
        else
        {
            decision[pRank] = buff;
        }
    }

    cout << "Node " << pRank << " decision: " << actionToString(decision[pRank]) << "\n";

    MPI_Finalize();
}

const char *actionToString(int action)
{
    if (action == ATTACK)
        return "attack";
    else if (action == DEFENSE)
        return "defense";
    else if (action == RETREAT)
        return "retreat";
    return "";
}

void broadcastMessage(int pRank, int n, const void *buf, int count)
{
    for (int i = 0; i < n; ++i)
    {
        if (i != pRank)
        {
            MPI_Send(buf, count, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
    }
}

void broadcastFaultyMessage(int pRank, int n)
{
    int faultyD;
    for (int i = 0; i < n; ++i)
    {
        if (i != pRank)
        {
            faultyD = rand() % 3;
            MPI_Send(&faultyD, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
    }
}
