#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <string.h>

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

// Function for the child process. Generates the numbers and prints in shared memory 
int childsolve(int procNo, size_t max, int ratio, int totalProc)
{
    size_t SIZE = 512;
    char integer[6];
    sprintf(integer, "%d", procNo);
    // Creating SHM
    int fd = shm_open(integer, O_CREAT | O_RDWR, 0666);
    ftruncate(fd, SIZE);
    char *ptr = (char *)mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    // Creating Log FILE
    FILE *logfile = fopen(strcat(integer, ".log"), "w");
    size_t *arr = (size_t *)malloc(64 * sizeof(size_t)); // Array for perfect numbers
    size_t sizearr = 0;
    for (size_t i = procNo + 1; i < max; i += totalProc)
    {
        bool res = perfectNum(i);
        if (res == 1)
        {
            fprintf(logfile, "%u is a Perfect Number\n", i);
            arr[sizearr++] = i;
        }
        else
            fprintf(logfile, "%u is not a Perfect Number\n", i);
    }
    sprintf(ptr, "%u\n", sizearr); // Printing the number of perfect numbers into SHM
    // Printing the perfect numbers to SHM
    for (size_t i = 0; i < sizearr; i++)
    {
        ptr += sizeof(size_t) + 1;
        sprintf(ptr, "%u\n", arr[i]);
    }
    free(arr);
    fclose(logfile);
    return 1;
}

// Function to produce children, and read the numbers in SHM to produce output file.
int solve(size_t numbers, int numProc)
{
    int status, proc = 0;
    pid_t pid;
    int procId[numProc];
    do // Making k child processes
    {
        pid = fork();
        if (pid == 0) // Child Process
        {
            status = childsolve(proc, numbers, numbers / numProc, numProc);
            exit(status);
        }
        else if (pid < 0) // If fork() fails
            status = -1;
        else if (pid > 0)
            procId[proc] = pid;
        proc++;
    } while (pid != 0 && proc < numProc);
    // Wait for child process, reading from SHM and printing it to output file 
    for (int i = 0; i < numProc; i++)
    {
        int stat;
        waitpid(procId[i], &stat, 0);
        const size_t SIZE = 512;
        char name[6];
        sprintf(name, "%d", i);
        int fd = shm_open(name, O_RDONLY, 0666);
        char *ptr = (char *)mmap(0, SIZE, PROT_READ, MAP_SHARED, fd, 0);
        size_t num = 1;
        sscanf(ptr, "%u", &num);
        FILE *output = fopen("output.txt", "a");
        fprintf(output, "P%s: ", name);
        if (num > 0)
        {
            while (num--)
            {
                ptr += sizeof(size_t) + 1;
                size_t per;
                sscanf(ptr, "%u", &per);
                fprintf(output, "%u", per);
                if(num!=0)
                    fprintf(output, ", ");
                else
                    fprintf(output, "\n");
            }
        }
        else
            fprintf(output, "None\n");
        fclose(output);
        shm_unlink(name);
    }
    return status;
}

int main()
{
    FILE *input = fopen("input.txt", "r");
    remove("output.txt");
    size_t n;
    int k;
    fscanf(input, "%u%d", &n, &k);
    fclose(input);
    solve(n, k);
    return 0;
}