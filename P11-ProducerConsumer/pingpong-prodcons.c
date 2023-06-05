#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "ppos.h"

#define BUFFER_SIZE 5    // Define Buffer Size
#define SLEEP_TIME 1000  // Define Sleep time
#define RANDOM_RANGE 100 // Define range to generate a random number

// Initialize task variables and semaphore variables
task_t prod1, prod2, prod3, cons1, cons2;
semaphore_t s_buffer, s_item, s_vacancy;

// Initialize variables for last insertion and consumption
int last_insertion = -1;
int last_consumption = -1;

// Initialize buffer
int buffer[BUFFER_SIZE];

// Producer function
void produtor(void *arg)
{
    int item;
    while (1)
    {
        task_sleep(SLEEP_TIME);                // Sleep for a defined time
        item = rand() % RANDOM_RANGE;          // Generate a random item between 0 and RANDOM_RANGE
        sem_down(&s_vacancy);                  // Access vacancy semaphore
        sem_down(&s_buffer);                   // Access buffer semaphore
        if (++(last_insertion) == BUFFER_SIZE) // Increment last insertion and Check if insertion has reached buffer limit
        {
            last_insertion = 0; // Reset last insertion
        }
        buffer[last_insertion] = item;                 // Add item to buffer
        sem_up(&s_buffer);                             // Release buffer semaphore
        sem_up(&s_item);                               // Release item semaphore
        printf("%s produced %d\n", (char *)arg, item); // Print production message
    }
}

// Consumer function
void consumidor(void *arg)
{
    int item;
    while (1)
    {
        sem_down(&s_item);                       // Access item semaphore
        sem_down(&s_buffer);                     // Access buffer semaphore
        if (++(last_consumption) == BUFFER_SIZE) // Increment last consumed and Check if consumption has reached buffer limit
        {
            last_consumption = 0; // Reset last consumed
        }
        item = buffer[last_consumption];               // Get item from buffer
        sem_up(&s_buffer);                             // Release buffer semaphore
        sem_up(&s_vacancy);                            // Release vacancy semaphore
        printf("%s consumed %d\n", (char *)arg, item); // Print consumption message
        task_sleep(SLEEP_TIME);                        // Sleep for a defined time
    }
}


// Main function
int main()
{
    printf("main : início\n");
    srand(time(0));                                        // Use current time as seed for random generator
    ppos_init();                                           // Initialize ppos
    sem_init(&s_buffer, 1);                                // Initialize buffer semaphore
    sem_init(&s_item, 0);                                  // Initialize item semaphore
    sem_init(&s_vacancy, BUFFER_SIZE);                     // Initialize vacancy semaphore
    task_init(&prod1, produtor, "p1");                     // Create producer task 1
    task_init(&prod2, produtor, "p2");                     // Create producer task 2
    task_init(&prod3, produtor, "p3");                     // Create producer task 3

    task_init(&cons1, consumidor, "                  c1"); // Create consumer task 1
    task_init(&cons2, consumidor, "                  c2"); // Create consumer task 2
    task_wait(&prod1);                                     // Wait for producer task 1 to finish
    return 0;                                              // Return 0 upon successful execution
}