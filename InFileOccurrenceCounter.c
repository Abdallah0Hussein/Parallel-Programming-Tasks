#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>

char input_directory[100];
char file_names [100][20];

// The master process first creates 100 files in the directory given from the user, 
// then adds the files’ name in an array.
// Fill those 100 files with 100 random numbers (from 1 to 100).
// Broadcast the number x to all the slave processes.

// Scatter the array of the files’ names among all processes (including the master).

void create_files(int num_files){
    for (int i = 1; i <= num_files; i++) {
            FILE *file;
            char file_name[100]; // Adjust the size as necessary
            printf("%s\n", input_directory);
            snprintf(file_name, sizeof(file_name), "%sfile_%d.txt", input_directory, i);
            file = fopen(file_name, "w");
            sprintf(file_names[i-1],"file_%d.txt",i);
            for (int i = 0; i < 5; i++)
            {   
                int random_number = rand() % 5 + 1;
                fprintf(file, "%d\n", random_number);
            }
            // fclose(file);
        }
        printf("Contents of file_names array:\n");
        for (int i = 0; i < 4; i++) {
            printf("%s\n", file_names[i]);
        }

}

int main(int argc, char *argv[]) {
    int my_rank, p;
    int tag = 0;
    // char scatterd_files[100/p][20];
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    if (my_rank == 0) { // Master Process
        // Get directory path from user
        printf("Enter the directory for the files: ");
        scanf("%s", input_directory);
        create_files(4);
        // Get number x from user
        int x;
        printf("Enter number x: ");
        scanf("%d", &x);
        // Send x from Master Proccess
        MPI_Bcast(&x, 1, MPI_INT, 0, MPI_COMM_WORLD);
        printf("Process %d has received x = %d\n", my_rank, x);
        // MPI_Scatter(file_names, 100/p, MPI_CHAR, scatterd_files, 100/p, MPI_CHAR, 0, MPI_COMM_WORLD);
        // printf("Process %d received the following file names:\n", my_rank);
        // for (int i = 0; i < 100/p; i++) {
        // printf("%s\n", scatterd_files[i]);
        // }
        // reminder files
    } else { // Slave Process
        int x;
        // Receive x from master process
        MPI_Bcast(&x, 1, MPI_INT, 0, MPI_COMM_WORLD);
        printf("Process %d has received x = %d\n", my_rank, x);

    }

    MPI_Finalize();
    return 0;
}
