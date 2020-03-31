/* THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING
 * A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Changmao Li */
#include <sys/shm.h>
#include <signal.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    pid_t pid;
    int perfect_num_found;
    int number_tested;
    int number_skipped;
} process;

typedef struct {
    int number_bitmap[1048576];
    int perfect_found[20];
    process summaries[20];
    int total_summaries[3];
} SharedMemory;

typedef struct {
    long msg_type;
    int msg_text;
} message;

extern int shared_memory_id;
extern int message_queue_id;
extern SharedMemory *shared_memory;
