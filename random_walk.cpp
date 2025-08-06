#include <iostream>
#include <cstdlib> // For atoi, rand, srand
#include <ctime>   // For time
#include <mpi.h>

void walker_process();
void controller_process();

int domain_size;
int max_steps;
int world_rank;
int world_size;

int main(int argc, char **argv)
{
    // Initialize the MPI environment
    MPI_Init(&argc, &argv);

    // Get the number of processes and the rank of this process
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    if (argc != 3)
    {
        if (world_rank == 0)
        {
            std::cerr << "Usage: mpirun -np <p> " << argv[0] << " <domain_size> <max_steps>" << std::endl;
        }
        MPI_Finalize();
        return 1;
    }

    domain_size = atoi(argv[1]);
    max_steps = atoi(argv[2]);

    if (world_rank == 0)
    {
        // Rank 0 is the controller
        controller_process();
    }
    else
    {
        // All other ranks are walkers
        walker_process();
    }

    // Finalize the MPI environment
    MPI_Finalize();
    return 0;
}

void walker_process()
{
    // Seed the random number generator.
    // Using rank ensures each walker gets a different sequence of random numbers.
    srand(time(NULL) + world_rank);

    int position = 0;
    int steps_taken;

    // Perform the random walk
    for (steps_taken = 0; steps_taken < max_steps; ++steps_taken)
    {
        // Randomly move left or right
        position += (rand() % 2 == 0) ? -1 : 1;

        // Check if the walker is out of bounds
        if (position > domain_size || position < -domain_size)
        {
            break; // Exit loop if out of bounds
        }
    }

    // Print the required "finished" message for the autograder.
    std::cout << "Rank " << world_rank << ": Walker finished in " << steps_taken << " steps." << std::endl;

    // Send the number of steps taken to the controller process (rank 0).
    int data_to_send = steps_taken;
    MPI_Send(&data_to_send, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
}

void controller_process()
{
    int num_walkers = world_size - 1;
    int received_data;
    MPI_Status status;

    std::cout << "Controller: Waiting for " << num_walkers << " walkers to finish..." << std::endl;

    // Loop to receive one message from each walker
    for (int i = 0; i < num_walkers; ++i)
    {
        // Receive a message from any walker that has finished.
        MPI_Recv(
            &received_data, // Buffer to store received data
            1,              // Max number of elements to receive
            MPI_INT,        // Data type of elements
            MPI_ANY_SOURCE, // Receive from any process
            0,              // Tag to match
            MPI_COMM_WORLD, // Communicator
            &status         // Status object to get info about the message
        );
        // You could optionally print a confirmation here:
        // std::cout << "Controller: Received confirmation from rank " << status.MPI_SOURCE << std::endl;
    }

    // After all walkers have checked in, print the final summary.
    std::cout << "Controller: All " << num_walkers << " walkers have finished." << std::endl;
}