/*THIS CODE IS MY OWN WORK. IT WAS WRITTEN WITHOUT CONSULTING
 * A TUTOR OR CODE WRITTEN BY OTHER STUDENTS --Changmao Li*/
#include <stdio.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <pthread.h>
#include <netdb.h>
#include <poll.h>


typedef struct {
    long r_start;
    long r_end;
} Range;

typedef struct {
    int is_tested;
    int nb_thread;
    long perfect_num_found[10];
    int nb_perfect_num;
    char hostname[40];
    long tested_numbers;
    Range range[10];
} Clients;

extern Clients client_list[10];