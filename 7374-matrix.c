#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

typedef struct {
    int row;
    int col;
    int **mat1;
    int **mat2;
    int **result;
    int cols1;
} ThreadDataElement; 

typedef struct {
    int row;
    int **mat1;
    int **mat2;
    int **result;
    int cols1;
    int cols2;
} ThreadDataRow; 

void* multiplyElement(void* arg) {
    ThreadDataElement* data = (ThreadDataElement*)arg; // Cast arg of type void to a pointer of type ThreadDataElement*.
    int row = data->row;
    int col = data->col;
    data->result[row][col] = 0; 
    for (int k = 0; k < data->cols1; k++) {
        data->result[row][col] += data->mat1[row][k] * data->mat2[k][col];
    } 
    pthread_exit(NULL); // Terminate the calling thread and return a value. NULL is the exit status.
}

void* multiplyRow(void* arg) {
    ThreadDataRow* data = (ThreadDataRow*)arg;
    int row = data->row;

    for (int j = 0; j < data->cols2; j++) { // Loop over all columns of second input matrix.
        data->result[row][j] = 0; //
        for (int k = 0; k < data->cols1; k++) {
            data->result[row][j] += data->mat1[row][k] * data->mat2[k][j];
        } // Multiply a single element of the output matrix based on the provided row and column.
    } // Compute a single row of the output matrix.

    pthread_exit(NULL);
}

void printMatrix(int **matrix, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
} 

int main(int argument_count, char *argument_values[]) { // Take input file as an argument.
    if (argument_count !=2)
    {
    printf("2 arguments required.\n");
    return 1;
    }
    FILE *file = fopen(argument_values[1], "r");

    if (file == NULL) {
        fprintf(stderr, "Error opening file.\n");
        return 1;
    }

    int rows1, cols1, rows2, cols2;

    fscanf(file, "%d %d", &rows1, &cols1); // Read dimensions of the first matrix.

    int **matrix1 = malloc(rows1 * sizeof(int *)); // Allocate memory to store rows1 pointers to integers (int *). We set up the array of pointers, but it doesn't allocate memory for the individual integer arrays yet.
    for (int i = 0; i < rows1; i++) {
        matrix1[i] = malloc(cols1 * sizeof(int)); // Allocate memory for an array of cols1 integers. Then, it stores the address of this newly allocated memory in matrix1[i].
        for (int j = 0; j < cols1; j++) {
            fscanf(file, "%d", &matrix1[i][j]);
        }
    } // Read first input matrix values.

    // Read dimensions of the second matrix.
    fscanf(file, "%d %d", &rows2, &cols2);

    int **matrix2 = malloc(rows2 * sizeof(int *)); 
    for (int i = 0; i < rows2; i++) {
        matrix2[i] = malloc(cols2 * sizeof(int));
        for (int j = 0; j < cols2; j++) {
            fscanf(file, "%d", &matrix2[i][j]);
        }
    } // Read second input matrix values.

    if (cols1 != rows2) {
        fprintf(stderr, "Multipication of Matrix is not Possible !!\n");
        return 1;
    } // Check if multiplication is possible.

    int **resultMatrixElement = malloc(rows1 * sizeof(int *)); 
    for (int i = 0; i < rows1; i++) {
        resultMatrixElement[i] = malloc(cols2 * sizeof(int)); 
    }

    int **resultMatrixRow = malloc(rows1 * sizeof(int *));  
    for (int i = 0; i < rows1; i++) {
        resultMatrixRow[i] = malloc(cols2 * sizeof(int)); 
    }

    pthread_t *threadsElement = malloc(rows1 * cols2 * sizeof(pthread_t)); // Allocate memory to store rows1 * cols2 elements of type pthread_t. This array will be used to store thread IDs for the threads responsible for element-wise matrix multiplication.
    ThreadDataElement *threadDataElement = malloc(rows1 * cols2 * sizeof(ThreadDataElement)); // Allocates memory to store rows1 * cols2 elements of type ThreadDataElement. This array will be used to store the data passed to each thread responsible for element-wise matrix multiplication. Each element in this array corresponds to a thread and contains information about the specific element in the result matrix that the thread is responsible for.

    pthread_t *threadsRow = malloc(rows1 * sizeof(pthread_t)); // Allocate memory to store rows1 elements of type pthread_t. This array will be used to store thread IDs for the threads responsible for row-wise matrix multiplication.
    ThreadDataRow *threadDataRow = malloc(rows1 * sizeof(ThreadDataRow)); // Allocate memory to store rows1 elements of type ThreadDataRow. This array will be used to store the data passed to each thread responsible for row-wise matrix multiplication. Each element in this array corresponds to a thread and contains information about the specific row in the result matrix that the thread is responsible for.

    struct timeval startElement, endElement; // Structure that represents time with microsecond precision. startElement and endElement are instances of this structure, which will be used to store the starting and ending times.
    gettimeofday(&startElement, NULL); // System call that gets the current time of day with microsecond precision and records the current time in the startElement variable.
    
    for (int i = 0; i < rows1; i++) {
        for (int j = 0; j < cols2; j++) {
            threadDataElement[i * cols2 + j].row = i;
            threadDataElement[i * cols2 + j].col = j;
            threadDataElement[i * cols2 + j].mat1 = matrix1;
            threadDataElement[i * cols2 + j].mat2 = matrix2;
            threadDataElement[i * cols2 + j].result = resultMatrixElement;
            threadDataElement[i * cols2 + j].cols1 = cols1;

            pthread_create(&threadsElement[i * cols2 + j], NULL, multiplyElement, (void*)&threadDataElement[i * cols2 + j]);
        } // [i * cols2 + j] to get current element in the array according to the element resulted from current row and column multiplication.
    }// Create threads to compute each element in the output matrix.

    for (int i = 0; i < rows1 * cols2; i++) {
        pthread_join(threadsElement[i], NULL);
    } // Block main thread to ensure that the main thread waits for the completion of all threads created for element-wise computation before moving on to the next steps in the program. Clean up resources associated with that thread, such as its stack and thread ID and ensuring that there are no memory leaks.

    gettimeofday(&endElement, NULL); // System call that gets the current time of day with microsecond precision and records the current time in the endElement variable.

    double elapsed_time_element = (endElement.tv_sec - startElement.tv_sec) + (endElement.tv_usec - startElement.tv_usec) / 1e6; // (endElement.tv_usec - startElement.tv_usec) / 1e6 calculates the difference in microseconds and converts it to seconds by dividing by 1e6 (1 million).
// Calculate the elapsed time in seconds.

    struct timeval startRow, endRow; // Structure that represents time with microsecond precision. startRow and endRow are instances of this structure, which will be used to store the starting and ending times.
    gettimeofday(&startRow, NULL); // System call that gets the current time of day with microsecond precision and records the current time in the startRow variable.

    for (int i = 0; i < rows1; i++) {
        threadDataRow[i].row = i;
        threadDataRow[i].mat1 = matrix1;
        threadDataRow[i].mat2 = matrix2;
        threadDataRow[i].result = resultMatrixRow;
        threadDataRow[i].cols1 = cols1;
        threadDataRow[i].cols2 = cols2;

        pthread_create(&threadsRow[i], NULL, multiplyRow, (void*)&threadDataRow[i]);
    }// [i] to get current element in the array according to the row resulted from current row and all columns multiplication.
// Create threads to compute each row in the output matrix.

    for (int i = 0; i < rows1; i++) {
        pthread_join(threadsRow[i], NULL);
    } // Block main thread to ensure that the main thread waits for the completion of all threads created for row-wise computation before moving on to the next steps in the program. Clean up resources associated with that thread, such as its stack and thread ID and ensuring that there are no memory leaks.

    gettimeofday(&endRow, NULL); // System call that gets the current time of day with microsecond precision and records the current time in the endRow variable.

    double elapsed_time_row = (endRow.tv_sec - startRow.tv_sec) + (endRow.tv_usec - startRow.tv_usec) / 1e6; 
// (endRow.tv_usec - startRow.tv_usec) / 1e6 calculates the difference in microseconds and converts it to seconds by dividing by 1e6 (1 million).
// Calculate the elapsed time in seconds.

    printMatrix(resultMatrixElement, rows1, cols2);

    printf("END1 %f\n", elapsed_time_element);

    printMatrix(resultMatrixRow, rows1, cols2);

    printf("END2 %f\n", elapsed_time_row);

    // Free allocated memory.
    for (int i = 0; i < rows1; i++) {
        free(matrix1[i]);
    }
    free(matrix1);

    for (int i = 0; i < rows2; i++) {
        free(matrix2[i]);
    }
    free(matrix2);

    for (int i = 0; i < rows1; i++) {
        free(resultMatrixElement[i]);
        free(resultMatrixRow[i]);
    }
    free(resultMatrixElement);
    free(resultMatrixRow);

    free(threadsElement);
    free(threadDataElement);

    free(threadsRow);
    free(threadDataRow);

    fclose(file);

    return 0;
}

