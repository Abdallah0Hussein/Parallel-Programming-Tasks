#include <stdio.h>
#include <string.h>
#include "mpi.h"

int isPrime(int num) {
    if (num <= 1)
        return 0; // Not prime
    for (int i = 2; i * i <= num; i++) {
        if (num % i == 0)
            return 0; // Not prime
    }
    return 1; // Prime
}

int main(int argc, char *argv[]) {
    int my_rank;
    int p;
    int tag = 0;
    int x,y,r;
    char message[100];
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    if (my_rank == 0) {
        // Master process
         printf("Enter lower bound: ");
        scanf("%d", &x);

        printf("Enter upper bound: ");
        scanf("%d", &y);

        // Calculate subrange size
        r = (y - x) / (p-1); 
        // Broadcast x and r to each slave process
        for (int dest = 1; dest < p; dest++)
        {
            MPI_Send(&x, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
            MPI_Send(&r, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
        }
        int total_count = 0,tmp_count = 0;
        // Receive sub count from each slave process
        for (int src = 1; src < p; src++)
        {
            MPI_Recv(&tmp_count, 1, MPI_INT, src, tag, MPI_COMM_WORLD, &status);
            total_count+=tmp_count;
        }
        // Print total count of primes between x and y
        printf("Total number of prime numbers is: %d\n", total_count);        
    } else {
        // Slave processes
        // Receive x and r
        MPI_Recv(&x, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
        MPI_Recv(&r, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
        
        // Calculate the lower bound a and upper bound b according to its rank.
        int start = x + (my_rank - 1) * r;
        int end = start + r - 1;
        if (my_rank >= p - 1) {
            // Adjust the end for the last process
            // the value is from the equation >> r = (y - x) / (p-1) <<
            // which means end = y;
            end = r * (p-1) + x;
        }

        int proccess_count = 0;
        // Count primes in subrange
        for (int i = start; i <= end; i++) {
            if (isPrime(i))
                proccess_count++;
        }
        // Print the partial count
        printf("Total number of prime numbers in P%d is: %d\n", my_rank, proccess_count);


        // Send this partial count to the master process
        MPI_Send(&proccess_count, 1, MPI_INT, 0, tag, MPI_COMM_WORLD);
    }
    MPI_Finalize();
    return 0;
}
