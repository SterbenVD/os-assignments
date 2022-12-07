#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

// Global variables for n and k
size_t numThread;
size_t numbers;

// Find whether a number is perfect or Not.
bool perfectNum(size_t num)
{
    size_t sum = 0;
    for (size_t i = 1; i <= num / 2 && sum <= num; i++)
    {
        if (num % i == 0)
            sum += i;
    }
    if (num == sum)
        return 1;
    else
        return 0;
}

// Function for the child thread. Generates the numbers and prints it to array in heap. Returns pointer of the array
void *threadsolve(void *parentdata)
{
    int threadNo = *(int *)parentdata;
    size_t SIZE = 512;
    char integer[6];
    sprintf(integer, "%d", threadNo);

    // Creating Log FILE
    FILE *logfile = fopen(strcat(integer, ".log"), "w");
    size_t *arr = (size_t *)malloc(64 * sizeof(size_t)); // Array for perfect numbers
    size_t sizearr = 1;
    for (size_t i = threadNo + 1; i < numbers; i += numThread)
    {
        bool res = perfectNum(i);
        if (res == 1)
        {
            fprintf(logfile, "%u is a Perfect Number\n", i);
            arr[sizearr++] = i;
            arr[0]++;
        }
        else
            fprintf(logfile, "%u is not a Perfect Number\n", i);
    }
    fclose(logfile);
    pthread_exit(arr); // Return pointer of the array
}

// Main function
int main()
{
    // Read n and k
    FILE *input = fopen("input.txt", "r");
    remove("output.txt");
    fscanf(input, "%u%u", &numbers, &numThread);
    fclose(input);
    size_t thread = 0;
    pthread_t threadId[numThread];
    size_t threadComm[numThread]; // Which threadNo is the thread
    for (size_t i = 0; i < numThread; i++)
        threadComm[i] = i;
    do // Making k threads
    {
        pthread_create(&threadId[thread], NULL, threadsolve, &threadComm[thread]);
        thread++;
    } while (thread < numThread);
    for (size_t i = 0; i < thread; i++) // Wait for each thread
    {
        size_t *ptr;
        pthread_join(threadId[i], (void **)&ptr);
        size_t num = ptr[0], size = ptr[0];
        // Print to output file
        FILE *output = fopen("output.txt", "a");
        fprintf(output, "T%u: ", i);
        if (num > 0)
        {
            while (num--)
            {
                fprintf(output, "%u", ptr[size - num]);
                if (num != 0)
                    fprintf(output, ", ");
                else
                    fprintf(output, "\n");
            }
        }
        else
            fprintf(output, "None\n");
        fclose(output);
        free(ptr);
    }
    return 0;
}
