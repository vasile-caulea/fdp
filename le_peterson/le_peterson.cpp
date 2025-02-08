#include <mpi.h>
#include <stdio.h>

#define NEW_ID_TAG 0
#define LEADER_TAG 1
#define BUFF_SIZE 2

using namespace std;

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int p_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &p_rank);
    MPI_Status status;
    int left_neighbors[] = {4, 5, 0, 2, 1, 3};
    int left_neighbor = left_neighbors[p_rank];

    int first_id = -1, second_id = -1;
    int buff[BUFF_SIZE] = {0};
    bool is_relay = false;

    buff[0] = p_rank;
    MPI_Send(buff, BUFF_SIZE, MPI_INT, left_neighbor, NEW_ID_TAG, MPI_COMM_WORLD);

    bool is_finished = false;
    int max_id = p_rank;
    int leader;
    while (!is_finished)
    {
        MPI_Recv(buff, BUFF_SIZE, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        switch (status.MPI_TAG)
        {
        case NEW_ID_TAG:
        {
            if (is_relay)
            {
                MPI_Send(buff, BUFF_SIZE, MPI_INT, left_neighbor, NEW_ID_TAG, MPI_COMM_WORLD);
            }
            else if (first_id == -1)
            {
                first_id = buff[0];
                if (max_id == first_id)
                {
                    buff[0] = max_id;
                    buff[1] = p_rank;
                    MPI_Send(buff, BUFF_SIZE, MPI_INT, left_neighbor, LEADER_TAG, MPI_COMM_WORLD);
                } else {
                    MPI_Send(buff, BUFF_SIZE, MPI_INT, left_neighbor, NEW_ID_TAG, MPI_COMM_WORLD);
                }
            }
            else
            {
                second_id = buff[0];
                if (first_id > max_id && first_id > second_id)
                {
                    max_id = first_id;
                    buff[0] = max_id;
                    buff[1] = p_rank;
                    MPI_Send(buff, BUFF_SIZE, MPI_INT, left_neighbor, NEW_ID_TAG, MPI_COMM_WORLD);
                    first_id = -1;
                    second_id = -1;
                }
                else
                {
                    is_relay = true;
                }
            }
        }
        break;
        case LEADER_TAG:
        {
            leader = buff[0];
            if (buff[1] != p_rank)
            {
                MPI_Send(buff, BUFF_SIZE, MPI_INT, left_neighbor, LEADER_TAG, MPI_COMM_WORLD);
            }
            is_finished = true;
        }
        break;
        }
    }

    cout << "Node " << p_rank << ", the leader is " << leader << "\n";
    MPI_Finalize();
}
