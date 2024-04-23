#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>
#include <time.h>
char input_directory[100];
char file_names[100][100];

void create_files(int num_files) {
    for (int i = 1; i <= num_files; i++) {
        FILE *file;
        char file_name[100]; // Adjust the size as necessary
        sprintf(file_name, "%sfile_%d.txt", input_directory, i);
        // printf("%s\n",file_name);
        file = fopen(file_name, "w");
        if (file == NULL) {
            fprintf(stderr, "Error creating file %s\n", file_name);
            return; // Exit function if file creation fails
        }
        for (int j = 0; j < 100; j++) {
            int random_number = rand() % 100 + 1;
            fprintf(file, "%d\n", random_number);
        }
        fclose(file);
        // Store file name in file_names array
        strcpy(file_names[i - 1], file_name); // Using strcpy to copy file name
    }
}
int count_occurrences_in_file(const char *filename, int target) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file %s\n", filename);
        return -1;
    }
    int count = 0;
    int num;
    while (fscanf(file, "%d", &num) == 1) {
        if (num == target) {
            count++;
        }
    }

    fclose(file);
    return count;
}

int main(int argc, char *argv[]) {
    int my_rank,p;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    char recv_file_names[100/p][100]; // Buffer for receiving scattered file names
    int x;

    if (my_rank == 0) { // Master Process
        printf("Enter the directory for the files: ");
        scanf("%s", input_directory);
        srand(time(NULL));
        create_files(100);
        printf("Enter number x: ");
        scanf("%d", &x);
        
    }
    MPI_Bcast(&x, 1, MPI_INT, 0, MPI_COMM_WORLD);
    // Scatter file names array to all processes
    MPI_Scatter(file_names, (100/p)*100, MPI_CHAR, recv_file_names, (100/p)*100, MPI_CHAR, 0, MPI_COMM_WORLD);
    // printf("Opened fileee %s\n", recv_file_names); 
    // Count occurrences of x in each file and accumulate the total
    int total_occurrences = 0;
    int total_occurrences_local = 0; // Use a separate variable for local occurrences
    // >>>>>> Handle remaining files <<<<<
    for (int i = 0; i < 100/p; i++) {
        int occurrences = count_occurrences_in_file(recv_file_names[i], x);
        total_occurrences_local += occurrences;
    }  
    // Send the total number of occurrences to the master process
    MPI_Reduce(&total_occurrences_local, &total_occurrences, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (my_rank == 0) { // Master Process
        int tmp_occurrences = 0;
        for (int i = (100/p)*p; i < 100; i++){
            int occurrences = count_occurrences_in_file(file_names[i], x);
            tmp_occurrences += occurrences;
        }
        printf("P%d: Total number of occurrences = %d\n", my_rank, total_occurrences_local+tmp_occurrences);
        printf("Total number of occurrences in all 100 files = %d\n", total_occurrences+tmp_occurrences);
    }
    else{
        printf("P%d: Total number of occurrences = %d\n", my_rank, total_occurrences_local);

    }
    MPI_Finalize();
    return 0;
}
