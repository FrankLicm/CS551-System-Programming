/*THIS CODE IS MY OWN WORK. IT WAS WRITTEN WITHOUT CONSULTING
 * A TUTOR OR CODE WRITTEN BY OTHER STUDENTS --Changmao Li*/
#include "defs.h"

int sock_fd;


pthread_mutex_t mutex_lock = PTHREAD_MUTEX_INITIALIZER;

void signal_handler(int signum) {
    char buffer[100];
    snprintf(buffer, sizeof(buffer), "terminate|");
    if (write(sock_fd, buffer, strlen(buffer) + 1) == -1) {
        perror("write");
        exit(1);
    } else {
        exit(0);

    }
}

void *run_compute(void *temp) {
    char buffer[100];
    Range *range = (Range *) temp;
    long start = range->r_start;
    long end = range->r_end;
    for(;;) {
        long i = 0;
        time_t b = time(NULL);
        for(i= start; i <= end; i++)
        {
            long sum = 0;
            long t;
            for(t = 1; t < i; t++) {
                if (i % t != 0) continue;
                sum += t;
            }
            if(sum == i)
            {
                snprintf(buffer, sizeof(buffer), "perfect|%ld", (long)i);
                write(sock_fd, buffer, strlen(buffer) + 1);
            }
        }
        time_t f = time(NULL);
        int time_e = (int)(f - b);
        if(time_e == 0) {
            time_e = 15;
        }
        long n_range = (end - start) * 15 / time_e;
        if(n_range > 20000) {
            n_range = 20000;
        }
        pthread_mutex_lock(&mutex_lock);
        snprintf(buffer, sizeof(buffer), "range|%ld|%ld", end, n_range);
        write(sock_fd, buffer, strlen(buffer) + 1);
        read(sock_fd, buffer, sizeof(buffer));
        pthread_mutex_unlock(&mutex_lock);
        char *msg = strtok(buffer, "|");
        if (strcmp(msg, "range") != 0) {
            if (!strcmp(msg, "quit")) continue;
            snprintf(buffer, sizeof(buffer), "terminate|");
            write(sock_fd, buffer, strlen(buffer) + 1);
            exit(0);
        } else {
            msg = strtok(NULL, "|");
            long start_position = atol(msg);
            msg = strtok(NULL, "|");
            start = start_position;
            end = start_position + atol(msg);
        }
    }
}


int main(int argc, char *argv[]) {
    if (argc >= 4) {
        struct sigaction signal;
        memset(&signal, 0, sizeof(signal));
        signal.sa_handler = signal_handler;
        if (sigaction(SIGINT, &signal, NULL) == 0) {
            if (sigaction(SIGQUIT, &signal, NULL) == 0) {
                if (sigaction(SIGHUP, &signal, NULL) == 0) {
                    struct sockaddr_in sin;
                    long address;
                    address = *(long *) (gethostbyname(argv[1])->h_addr);
                    sin.sin_addr.s_addr = address;
                    sin.sin_family = AF_INET;
                    sin.sin_port = atoi(argv[2]);
                    for (;;) {
                        if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) != -1) {

                            if (connect(sock_fd, (struct sockaddr *) &sin, sizeof(sin)) != -1) break;
                            printf("Wait for connection\n");
                            close(sock_fd);
                            sleep(10);
                            continue;
                        } else {
                            perror("socket");
                            exit(1);
                        }
                    }
                    printf("Connection established\n");
                    char buf[100];
                    int thread_num = atoi(argv[3]);
                    pthread_t tid[thread_num];
                    snprintf(buf, sizeof(buf), "request|%d", thread_num);
                    write(sock_fd, buf, strlen(buf) + 1);
                    read(sock_fd, buf, sizeof(buf));
                    char *pch = strtok(buf, "|");
                    if (strcmp(pch, "request") == 0) {
                        int i;
                        for (i = 0; i < thread_num; i++) {
                            pch = strtok(NULL, "|");
                            long start_point = atol(pch);
                            pch = strtok(NULL, "|");
                            long range_size = atol(pch);
                            long end_point = start_point + range_size;
                            Range *tmpRange = malloc(sizeof(Range));
                            tmpRange->r_start = start_point;
                            tmpRange->r_end = end_point;
                            pthread_t tmp_thread;
                            pthread_create(&tmp_thread, NULL, run_compute, (void *) tmpRange);
                            tid[i] = tmp_thread;
                        }
                    }
                    for (;;) {
                        pthread_mutex_lock(&mutex_lock);
                        if (!recv(sock_fd, buf, sizeof(buf), MSG_DONTWAIT)) continue;
                        char *cmd = strtok(buf, "|");
                        if (strcmp(cmd, "quit") != 0) {
                            pthread_mutex_unlock(&mutex_lock);
                        } else {
                            snprintf(buf, sizeof(buf), "terminate|");
                            write(sock_fd, buf, strlen(buf) + 1);
                            exit(0);
                        }
                    }
                } else {
                    perror("sigaction:SIGHUP");
                    exit(1);
                }
            } else {
                perror("sigaction:SIGQUIT");
                exit(1);
            }
        } else {
            perror("sigaction:SIGINT");
            exit(1);
        }
    } else {
        printf("Arguments are not enough.\n");
        exit(1);
    }
}
