#include <mpi.h>
#include <iostream>

using namespace std;

#define TAG_PROBE 1
#define TAG_REPLY 2
#define TAG_LEADER 3

#define MSG_SIZE 3

#define RECEIVED_FROM_BOTH_NEIGHBORS(x) ((x == 2))

enum Indexes
{
    ID_PROCESS,
    CURRENT_PHASE,
    DISTANCE
};

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    MPI_Status status;
    int pRank;
    MPI_Comm_rank(MPI_COMM_WORLD, &pRank);

    int leftNeighbors[] = {4, 2, 5, 6, 3, 0, 1}; // left neighbors for each node (eg. left neighbor of 0 is 4)
    int rightNeighbors[] = {5, 6, 1, 4, 0, 2, 3};

    int leftNeighbor = leftNeighbors[pRank];
    int rightNeighbor = rightNeighbors[pRank];

    int msg[MSG_SIZE];
    msg[Indexes::ID_PROCESS] = pRank;
    msg[Indexes::CURRENT_PHASE] = 0;
    msg[Indexes::DISTANCE] = 1;

    MPI_Send(msg, MSG_SIZE, MPI_INT, leftNeighbor, TAG_PROBE, MPI_COMM_WORLD);
    MPI_Send(msg, MSG_SIZE, MPI_INT, rightNeighbor, TAG_PROBE, MPI_COMM_WORLD);

    int msgSource, msgTag, receivedID, d, currentPhase;
    int alreadyReceivedCounter = 0;

    while (true)
    {
        MPI_Recv(msg, MSG_SIZE, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        msgSource = status.MPI_SOURCE;
        msgTag = status.MPI_TAG;
        receivedID = msg[Indexes::ID_PROCESS];
        currentPhase = msg[Indexes::CURRENT_PHASE];
        d = msg[2];

        if (TAG_PROBE == msgTag)
        {
            if (receivedID == pRank)
            {
                MPI_Send(msg, MSG_SIZE, MPI_INT, leftNeighbor, TAG_LEADER, MPI_COMM_WORLD);
            }
            // if the received id value is bigger than the current process, (it might be the leader node)
            else if (receivedID > pRank)
            {
                // if the distance at step k was not fully traversed, then send the message to the next node
                if (d < (1 << currentPhase))
                {
                    msg[Indexes::DISTANCE] = d + 1;
                    MPI_Send(msg, MSG_SIZE, MPI_INT, (msgSource == leftNeighbor) ? rightNeighbor : leftNeighbor, TAG_PROBE, MPI_COMM_WORLD);
                }
                else // if (d >= (1 << k)) - the distance was traversed, send back the message to the source
                {
                    MPI_Send(msg, MSG_SIZE, MPI_INT, msgSource, TAG_REPLY, MPI_COMM_WORLD);
                }
            }
            // the message is swallowed
        }
        else if (msgTag == TAG_REPLY)
        {
            if (receivedID != pRank)
            {
                MPI_Send(msg, MSG_SIZE, MPI_INT, (msgSource == leftNeighbor) ? rightNeighbor : leftNeighbor, TAG_REPLY, MPI_COMM_WORLD);
            }
            else
            {
                alreadyReceivedCounter++;
                // if the Process received confirmation that it is the winner of phase k
                if (RECEIVED_FROM_BOTH_NEIGHBORS(alreadyReceivedCounter))
                {
                    msg[1] = currentPhase + 1;
                    msg[2] = 1;
                    MPI_Send(msg, MSG_SIZE, MPI_INT, leftNeighbor, TAG_PROBE, MPI_COMM_WORLD);
                    MPI_Send(msg, MSG_SIZE, MPI_INT, rightNeighbor, TAG_PROBE, MPI_COMM_WORLD);
                    alreadyReceivedCounter = 0; // reset the counter
                }
            }
        }
        else if (TAG_LEADER == msgTag) // (msgSource == rightNeighbor && ) 
        {
            if (receivedID != pRank)
            {
                MPI_Send(msg, MSG_SIZE, MPI_INT, leftNeighbor, TAG_LEADER, MPI_COMM_WORLD);
            }
            cout << "Node " << pRank << " finished. ";
            break;
        }
    }

    cout << "Node " << pRank << " leader: " << receivedID << "\n";
    cout.flush();
    MPI_Finalize();
}
