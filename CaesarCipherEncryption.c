#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mpi.h"

void caesar_cipher(char *string, int choice, int length, int my_rank) {
    int shift = 3;
    if (choice == 2) {
        // Decryption
        shift = -shift;
    }
    for (int i = 0; i < length; i++) {
        if (string[i] >= 'a' && string[i] <= 'z') {     
            string[i] = 'a' + ((((string[i] - 'a') + shift) + 26) % 26);
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
    int mode;
    char input_string[100];
    MPI_Status status;
    int start_indx, end_indx;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    if (my_rank == 0) {
        // Master process
        printf("Choose the program mode:\n1. Input from the console\n2. Input from a file\n");
        fflush(stdout);
        scanf("%d", &mode);

        if (mode != 1 && mode != 2){
            printf("Invalid choice\n");
            fflush(stdout);
            MPI_Finalize();
            return 0;
        }

        printf("Choose an option:\n1. Encrypt\n2. Decrypt\n");
        fflush(stdout);
        scanf("%d", &choice);

        if (choice != 1 && choice != 2) {
            printf("Invalid choice\n");
            fflush(stdout);
            MPI_Finalize();
            return 0;
        }
        // Console Mode
        if (mode == 1) {
            printf("Enter The text to perform: ");
            fflush(stdout);
            scanf(" %s", input_string);
        // File Mode    
        }else if (mode == 2) {
            char* file_name;
            printf("Enter The name of the text file (without extension): ");
            fflush(stdout);
            scanf("%s", file_name);
            strcat(file_name, ".txt");
            // read the input from a file
            // Open the file in read mode
            FILE *file = fopen(file_name, "r");
            if (file != NULL) {
            // Find the size of the file
            /*
                Determine the size of the file by seeking to the end of the file then using ftell(file).
                the ftell function is used to determine the current position of the file pointer within a file stream
            */
                fseek(file, 0, SEEK_END);
                long file_size = ftell(file);
                // After that, we seek back to the beginning of the file
                fseek(file, 0, SEEK_SET); 
                // Allocate memory for the string
                char *buffer = (char *)malloc(file_size + 1); // +1 for null terminator    
                if (buffer != NULL) {
                    // Read the file into the buffer
                    size_t bytes_read = fread(buffer, 1, file_size, file);
                    buffer[bytes_read] = '\0'; // Add null terminator
                    // Close the file
                    fclose(file); 
                    strcpy(input_string, buffer);  
                    // Free the allocated memory
                    free(buffer);
                } else {
                    // Memory allocation failed
                    printf("Memory allocation failed.\n");
                }
            } else {
                // File opening failed
                printf("Failed to open the file.\n");
            }
            printf("String From File: %s\n", input_string);
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
            MPI_Send(&choice, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
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

        // if (mode == 2){
        //     char* file_name;
        //     printf("Enter The name of a text file to Save Processed String into (To Exit Press 0): ");
        //     fflush(stdout);
        // PROBLEM: CANNOT INTERRUPT TO GET THE INPUT
        //     scanf("%s", file_name)
        //     strcat(file_name, ".txt");

        //     if (strcmp(file_name, "0") == 0){
        //         printf("Okay, Bye :)\n");
        //         fflush(stdout);
        //         MPI_Finalize();
        //         return 0;
        //     }

        //     FILE *file = fopen(file_name, "w");
        //     if (file != NULL) {
        //         fprintf(file, "%s", input_string);
        //         // Close the file
        //         fclose(file);
        //         printf("String successfully written to the file.\n");
        //     } else {
        //         // File opening failed
        //         printf("Failed to open the file.\n");
        //     }
        //     fflush(stdout);
        // }
    } else {
        // Slave processes
        MPI_Recv(&start_indx, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
        MPI_Recv(&end_indx, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
        
        // in a parallel program, each process operates independently. 
        // Using separate buffers allows each  process to work on its portion of the data without confusion from other processes.
        // Receive the portion of the string to process
        char input_part[end_indx - start_indx];
        MPI_Recv(&input_part, end_indx - start_indx, MPI_CHAR, 0, tag, MPI_COMM_WORLD, &status);
        
        MPI_Recv(&choice, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
        // Process the received part
        caesar_cipher(input_part, choice, end_indx - start_indx, my_rank);

        // Send back the processed part to the master
        MPI_Send(input_part, end_indx - start_indx, MPI_CHAR, 0, tag, MPI_COMM_WORLD);
    }
    MPI_Finalize();
    return 0;
}
