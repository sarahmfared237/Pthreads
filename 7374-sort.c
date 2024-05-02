#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

typedef struct {
    int* arr;
    int l;
    int r;
} ThreadData; // Structure to hold information that will be passed to each thread i

// Function to merge two subarrays into a single sorted array.
void merge(int arr[], int p, int q, int r) {
    int n1 = q - p + 1;
    int n2 = r - q;
    int L[n1], M[n2];

    // Copy data to temporary arrays L[] and M[].
    for (int i = 0; i < n1; i++)
        L[i] = arr[p + i];
    for (int j = 0; j < n2; j++)
        M[j] = arr[q + 1 + j];

    // Merge the temporary arrays back into arr[p..r].
    int i = 0;
    int j = 0;
    int k = p;
    while (i < n1 && j < n2) {
        if (L[i] <= M[j]) {
            arr[k] = L[i];
            i++;
        } else {
            arr[k] = M[j];
            j++;
        }
        k++;
    }

    // Copy the remaining elements of L[], if any.
    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }

    // Copy the remaining elements of M[], if any.
    while (j < n2) {
        arr[k] = M[j];
        j++;
        k++;
    }
}

// Function to perform merge sort on the array.
void* mergeSort(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    int l = data->l;
    int r = data->r;

    if (l < r) {
        int m = l + (r - l) / 2;

        // Create threads to sort each half.
        pthread_t leftThread, rightThread;

        ThreadData leftData = {data->arr, l, m};
        ThreadData rightData = {data->arr, m + 1, r};

        pthread_create(&leftThread, NULL, mergeSort, (void*)&leftData); //(identifier, deafult, function, arg to fun)
        pthread_create(&rightThread, NULL, mergeSort, (void*)&rightData);

        // Block main thread to ensure that the main thread waits for the completion of all threads created for merge sort before moving on to the next steps in the program. Clean up resources associated with that thread, such as its stack and thread ID and ensuring that there are no memory leaks.
        pthread_join(leftThread, NULL);
        pthread_join(rightThread, NULL);

        // Merge the sorted halves.
        merge(data->arr, l, m, r);
    }

    pthread_exit(NULL); // Terminate the calling thread and return a value. NULL is the exit status.
}

// Function to print the array.
void printArray(int arr[], int size) { 
    for (int i = 0; i < size; i++)
        printf("%d ", arr[i]);
    printf("\n");
}

int main(int argument_count, char *argument_values[]) { // Take input file as an argument.
    if (argument_count != 2) {
        printf("2 arguments required.\n");
        return 1;
    }
    FILE *file = fopen(argument_values[1], "r");

    if (file == NULL) {
        fprintf(stderr, "Error opening file.\n");
        return 1;
    }

    int size;
    fscanf(file, "%d", &size); // Read size of the input array.

    int *arr = malloc(size * sizeof(int));
    int arraySize = 0; // Variable to track the actual number of elements read

    // Read the array elements from the file
    while (fscanf(file, "%d", &arr[arraySize]) == 1) {
        arraySize++;
    }

    fclose(file);
    // Check if the actual size matches the size from the file
    if (arraySize != size) {
        fprintf(stderr, "Error: Array size and input size mismatch in the input file.\n");
        free(arr);
        return 0; // Exit the program with an error code
    }
    
    // Check if the length of the array is 0 or 1.
    if (size == 1) {
        printf("Array is already sorted: ");
        printArray(arr, size);
        free(arr);
        return 0; // Exit the program.
    }
    if (size == 0) {
        printf("No values to be sorted\n");
        free(arr);
        return 0; // Exit the program.
    }

    // Create initial structure representing the entire array.
    ThreadData initialData = {arr, 0, size - 1};

    // Measure start time
    struct timeval start, end; 
    gettimeofday(&start, NULL); 

    pthread_t initialThread; // Define the initial thread.
    pthread_create(&initialThread, NULL, mergeSort, (void*)&initialData); //Create the initial thread for merge sort.
    pthread_join(initialThread, NULL); // Block main thread to ensure that the main thread waits for the completion of the initial thread created for merge sort before moving on to the next steps in the program. Clean up resources associated with that thread, such as its stack and thread ID and ensuring that there are no memory leaks.

    gettimeofday(&end, NULL); 

    printf("Sorted array: ");
    printArray(arr, size);

    double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;

// Calculate the elapsed time in seconds.
    printf("END %f\n", elapsed_time);

    free(arr); // Free allocated memory.

    return 0;
}

