#include <stdio.h> 
#include <stdlib.h> 
#include <mpi.h>

// Macro to generate data 
#define generate_data(i,j) (i)+(j)*(j) 

void main(int argc, char **argv) 
{ 
    // Required variables
    int i, j, pid, np, mtag, data[100][100], row_sum[100];
    MPI_Status status; 
    MPI_Request req_s, req_r[50];  // Array of requests for multiple non-blocking receives

    // Initialize MPI
    MPI_Init(&argc, &argv); 
    MPI_Comm_rank(MPI_COMM_WORLD, &pid); // Get process rank
    MPI_Comm_size(MPI_COMM_WORLD, &np);  // Get total number of processes

    // Process with ID 0
    if(pid == 0) {
        // Generate first half of the data
        for(i=0; i<50; i++)
            for(j=0; j<100; j++) 
                data[i][j] = generate_data(i,j);

        // Sending first half to process 1
        // Placing asynchronous sends inside a for loop.
        // This will send 1 row at a time to PID 1 then return control to main PID 0 function
        // Processing will continue on main PID 0 function while sending completes
        mtag = 1;
        for(i=0; i<50; i++)
            MPI_Isend(data[i], 100, MPI_INT, 1, mtag, MPI_COMM_WORLD, &req_s);
        

        // Generate the second half of the data using the given generate function
        for(i=50; i<100; i++)
            for(j=0; j<100; j++) 
                data[i][j] = generate_data(i,j);

        // Compute row sums for the second half of the data
        for(i=50; i<100; i++){ 
            row_sum[i] = 0; 
            for(j=0; j<100; j++) 
                row_sum[i] += data[i][j]; 
        } 

        MPI_Wait(&req_s, &status); // Wait for the data send to complete

        // Receive of computed row_sums from process 1
        mtag = 2;
        MPI_Recv(row_sum, 50, MPI_INT, 1, mtag, MPI_COMM_WORLD, &status); 

        // Print all row sums
        for(i=0; i<100; i++) { 
            printf(" %d ", row_sum[i]);
            if(i%10 == 9) printf("\n"); 
        }
    }
    // Process with ID 1
    else { /*** pid == 1 ***/
        
        
        // Set up non-blocking recieves in a for loop
        mtag = 1;
        for(i=0; i<50; i++) {
            MPI_Irecv(data[i], 100, MPI_INT, 0, mtag, MPI_COMM_WORLD, &req_r[i]);
        } 

        // Set up wait and summation tasts in a for loop
        // This should handle each message as it is recieved from above series of MPI_Ircev
        for(i=0; i<50; i++) {
            MPI_Wait(&req_r[i], &status);  // Wait for the receive to complete

            row_sum[i] = 0;
            for(j=0; j<100; j++) 
                {
                row_sum[i] += data[i][j];
                }
           
            //Commented out debugging logs
            //printf("row: ");
            //printf(" %d ", i);
            //printf(" %d ", row_sum[i]);
        } 


        // Send the computed row sums to process 0
        mtag = 2;
        MPI_Send(row_sum, 50, MPI_INT, 0, mtag, MPI_COMM_WORLD);
    } 

    MPI_Finalize(); // Finalize MPI
}