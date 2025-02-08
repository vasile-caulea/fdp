#include <mpi.h>
#include <stdio.h>

using namespace std;

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    MPI_Status status;

    time_t hc;
    time(&hc);

    int d = 10;
    int u = 6;
    double *diff = new double[world_size]{0};
    srand(time(0) + world_rank * time(0));

    for (int i = 0; i < world_size; ++i)
    {
        if (i != world_rank)
        {
            long msg = (long)hc;
            MPI_Send(&msg, 1, MPI_LONG, i, 0, MPI_COMM_WORLD);
        }
    }
    long t;
    double w, max_w = 0;
    for (int i = 0; i < world_size - 1; ++i)
    {
        MPI_Recv(&t, 1, MPI_LONG, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        w = u + (rand() % (d - u + 1));
        // cout << "P" << world_rank << ": " << w << "\n";
        diff[status.MPI_SOURCE] = t + d - (u / 2) - (hc + w);

        if (w > max_w)
        {
            max_w = w;
        }
    }

    double adjust = 0;
    for (int i = 0; i < world_size; ++i)
    {
        adjust += diff[i];
    }
    adjust = adjust / world_size;
    double result = hc + adjust + max_w;
    // cout << "Node: " << world_rank << ". HC: " << hc << ". Result: " << result << "\n";
    printf("Node %d. HC: %ld. Result %.2lF\n", world_rank, hc, result);
    delete[] diff;

    MPI_Finalize();
}