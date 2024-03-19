#include <stdio.h>
#include <string.h>
#include "mpi.h"

void caesar_cipher(char *string, int choice, int length, int my_rank) {
    int shift = 3;
    if (choice == 2) {
        // Decryption
        shift = -shift;
    }
    for (int i = 0; i < length; i++) {
        if (string[i] >= 'a' && string[i] <= 'z') {
            string[i] = 'a' + (string[i] - 'a' + shift + 26) % 26;
            printf("Process %d: %c -> %c\n", my_rank, string[i] - shift, string[i]);
        } else if (string[i] >= 'A' && string[i] <= 'Z') {
            string[i] = 'A' + (string[i] - 'A' + shift + 26) % 26;
            printf("Process %d: %c -> %c\n", my_rank, string[i] - shift, string[i]);
        }
    }
}

int main(int argc, char *argv[]) {
    int my_rank;
    int p;
    int tag = 0;
    int choice;
    char input_string[100];
    MPI_Status status;
    int start_indx, end_indx;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    if (my_rank == 0) {
        // Master process
        printf("Choose an option:\n1. Encrypt\n2. Decrypt\n");
        fflush(stdout);
        scanf("%d", &choice);

        if (choice != 1 && choice != 2) {
            printf("Invalid choice\n");
            fflush(stdout);
            MPI_Finalize();
            return 0;
        }
        if (choice == 1) {
            printf("Enter text: ");
            fflush(stdout);
            scanf(" %s", input_string);
        } else if (choice == 2) {
            // read the input from a file
        }
        int length = strlen(input_string);
        int piece_sz = length / p;
        int remainder = length % p;

        // devide the input string to all processes
        start_indx = 0;
        for (int dest = 1; dest < p; dest++) {
            // The current process will receive an additional part of work to handle the remainder.
            end_indx = start_indx + piece_sz + (1 <= remainder ? 1 : 0);
            remainder--;
            MPI_Send(&start_indx, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
            MPI_Send(&end_indx, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
            MPI_Send(&input_string[start_indx], end_indx - start_indx, MPI_CHAR, dest, tag, MPI_COMM_WORLD);
            start_indx = end_indx;
        }

        // Process master's cipher
        end_indx = start_indx + piece_sz + (0 < remainder ? 1 : 0);
        caesar_cipher(&input_string[start_indx], choice, end_indx - start_indx, my_rank);

        // Receive processed parts from slave processes
        int remainderr = length % p;

        start_indx = 0;
        for (int src = 1; src < p; src++) {
            end_indx = start_indx + piece_sz + (1 <= remainderr ? 1 : 0); // Receive the remainder from all processes
            remainderr--;
            MPI_Recv(&input_string[start_indx], end_indx - start_indx, MPI_CHAR, src, tag, MPI_COMM_WORLD, &status);
            start_indx = end_indx;
        }

        // Print the processed string
        printf("Processed string: %s\n", input_string);
        fflush(stdout);
    } else {
        // Slave processes
        MPI_Recv(&start_indx, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
        MPI_Recv(&end_indx, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
        
        // in a parallel program, each process operates independently. 
        // Using separate buffers allows each  process to work on its portion of the data without confusion from other processes.
        // Receive the portion of the string to process
        char input_part[end_indx - start_indx];
        MPI_Recv(input_part, end_indx - start_indx, MPI_CHAR, 0, tag, MPI_COMM_WORLD, &status);

        // Process the received part
        caesar_cipher(input_part, choice, end_indx - start_indx, my_rank);

        // Send back the processed part to the master
        MPI_Send(input_part, end_indx - start_indx, MPI_CHAR, 0, tag, MPI_COMM_WORLD);
    }
    MPI_Finalize();
    return 0;
}
