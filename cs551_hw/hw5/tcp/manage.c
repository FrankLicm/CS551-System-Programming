/*THIS CODE IS MY OWN WORK. IT WAS WRITTEN WITHOUT CONSULTING
 * A TUTOR OR CODE WRITTEN BY OTHER STUDENTS --Changmao Li*/
#include "defs.h"


int is_killed = 0;
Clients client_list[10];
struct pollfd poll_fd[10];
void signal_handler(int signum)
{
    int i;
    for(i =1; i < 10; i++){
        if(client_list[i].is_tested == 1){
            write(poll_fd[i].fd, "quit|", 100);
        }
        is_killed = 0;
    }
    exit(0);
}

void report_message(int tfd) {
    int j;
    char msg[1000];
    memset(&msg, 0, sizeof(msg));
    strcat(msg, "Perfect numbers: \n");
    strcat(msg, "PN    (host)\n");
    for(j = 0; j < 10; j++){
        if(client_list[j].is_tested == 1){
            int temp;
            for(temp = 0; temp < 10; temp++) {
                if(client_list[j].perfect_num_found[temp] != 0) {
                    char buffer[100];
                    snprintf(buffer, sizeof(buffer), "%ld", client_list[j].perfect_num_found[temp]);
                    strcat(msg, buffer);
                    strcat(msg, "    ");
                    strcat(msg, client_list[j].hostname);
                    strcat(msg, "\n");
                }
            }
        }
    }
    strcat(msg, "\n");
    strcat(msg, "Computers: \n");
    strcat(msg, "Host    Tested    Ranges\n");
    for(j = 0; j < 10; j++) {
        if(client_list[j].is_tested == 1) {
            char buffer[1000];
            snprintf(buffer, sizeof(buffer), "%s    %ld    ", client_list[j].hostname, client_list[j].tested_numbers);
            int k;
            for(k = 0; k < client_list[j].nb_thread; k++) {
                char range_buffer[100];
                snprintf(range_buffer, sizeof(range_buffer), "%ld-%ld ", client_list[j].range[k].r_start, client_list[j].range[k].r_end);
                strcat(buffer, range_buffer);
            }
            strcat(msg, buffer);
            strcat(msg, "\n");
        }
    }
    write(tfd, msg, strlen(msg) + 1);
}

int main(int argc, char *argv[]) {
    if (argc >= 2) {
        int nb_perfect_num = 0;
        int nb_host = 0;
        int nb_tested = 0;
        struct sigaction signal;
        memset(&signal, 0, sizeof(signal));
        signal.sa_handler = signal_handler;
        if (sigaction(SIGINT, &signal, NULL) == 0) {
            if (sigaction(SIGQUIT, &signal, NULL) == 0) {
                if (sigaction(SIGHUP, &signal, NULL) == 0) {
                    struct sockaddr_in sin;
                    memset(&sin, 0, sizeof(sin));
                    int socket_fd;
                    int clients[10];
                    struct hostent *h;
                    memset(&poll_fd, 0, sizeof(poll_fd));
                    sin.sin_addr.s_addr = INADDR_ANY;
                    sin.sin_port = atoi(argv[1]);
                    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) != -1) {
                        if (bind(socket_fd, (struct sockaddr *) &sin, sizeof(sin)) >= 0) {
                            listen(socket_fd, 10 - 1);
                            poll_fd[0].fd = socket_fd;
                            poll_fd[0].events = POLLIN;
                            int i;
                            for (i = 1; i < 10; i++) {
                                poll_fd[i].fd = -1;
                                poll_fd[i].events = POLLIN;
                            }
                            for (i = 0; i < 10; i++) {
                                memset(&client_list[i], 0, sizeof(Clients));
                            }
                            for(;;) {
                                poll(poll_fd, 10, -1);
                                if (!(poll_fd[0].revents & POLLIN)) {
                                    for (i = 1; i < 10; i++) {
                                        if (poll_fd[i].fd == -1 || !poll_fd[i].revents) continue;
                                        char buf[100];
                                        if (read(poll_fd[i].fd, buf, 100) <= 0) {
                                            close(poll_fd[i].fd);
                                            poll_fd[i].fd = -1;
                                        } else {
                                            char *cmd = strtok(buf, "|");
                                            if (strcmp(cmd, "request") != 0) {
                                                if (strcmp(cmd, "report") != 0) {
                                                    if (strcmp(cmd, "quit") != 0) {
                                                        if (strcmp(cmd, "perfect") != 0) {
                                                            if (strcmp(cmd, "range") != 0) {
                                                                if (strcmp(cmd, "terminate") != 0) continue;
                                                                nb_host--;
                                                                if (!is_killed || nb_host != 0) continue;
                                                                report_message(poll_fd[i].fd);
                                                                exit(0);
                                                            } else {
                                                                cmd = strtok(NULL, "|");
                                                                long range_start = atol(cmd);
                                                                cmd = strtok(NULL, "|");
                                                                long range_size = atol(cmd);
                                                                Range temp_range;
                                                                temp_range.r_start = nb_tested + 1;
                                                                temp_range.r_end =
                                                                        temp_range.r_start + range_size;
                                                                snprintf(buf, sizeof(buf), "range|%ld|%ld",
                                                                         temp_range.r_start,
                                                                         range_size);
                                                                write(poll_fd[i].fd, buf, strlen(buf) + 1);
                                                                client_list[i].tested_numbers +=
                                                                        range_size + 1;
                                                                int tmp;
                                                                for (tmp = 0;
                                                                     tmp < client_list[i].nb_thread; tmp++) {
                                                                    if (client_list[i].range[tmp].r_end !=
                                                                        range_start)
                                                                        continue;
                                                                    client_list[i].range[tmp] = temp_range;
                                                                    break;
                                                                }
                                                                nb_tested += range_size + 1;
                                                            }
                                                        } else {
                                                            cmd = strtok(NULL, "|");
                                                            long perfect = atol(cmd);
                                                            client_list[i].perfect_num_found[nb_perfect_num] = perfect;
                                                            nb_perfect_num++;
                                                        }
                                                    } else {
                                                        int j;
                                                        for (j = 0; j < 10; j++) {
                                                            if (!client_list[j].is_tested) continue;
                                                            write(poll_fd[j].fd, "quit|", 100);
                                                        }
                                                        is_killed = 1;
                                                    }
                                                } else {
                                                    report_message(poll_fd[i].fd);
                                                }
                                            } else {
                                                snprintf(buf, sizeof(buf), "request");
                                                cmd = strtok(NULL, "|");
                                                int nb_thread = atol(cmd);
                                                int thread_id;
                                                for (thread_id= 0; thread_id < nb_thread; thread_id++) {
                                                    char range_buf[100];
                                                    int range_start = nb_tested + 1;
                                                    int range_end = range_start + 20000;
                                                    snprintf(range_buf, sizeof(range_buf), "|%d|%d",
                                                             range_start,
                                                             20000);
                                                    strcat(buf, range_buf);
                                                    client_list[i].nb_thread = nb_thread;
                                                    client_list[i].tested_numbers += 20000 + 1;
                                                    client_list[i].is_tested = 1;
                                                    Range temp_range;
                                                    temp_range.r_start = range_start;
                                                    temp_range.r_end = range_end;
                                                    client_list[i].range[thread_id] = temp_range;
                                                    nb_tested += 20000 + 1;
                                                }
                                                write(poll_fd[i].fd, buf, strlen(buf) + 1);
                                                nb_host++;
                                            }

                                        }
                                    }
                                } else {
                                    unsigned int len = sizeof(sin);
                                    int fd;
                                    if ((fd = accept(poll_fd[0].fd, (struct sockaddr *) &sin, &len)) == -1) {
                                        perror("accept");
                                        exit(1);
                                    }
                                    for (i = 1; i < 10; i++) {
                                        if (poll_fd[i].fd == -1)
                                            break;
                                    }
                                    poll_fd[i].fd = fd;
                                    clients[i] = 0;
                                    if ((h = gethostbyaddr((char *) &sin.sin_addr.s_addr, sizeof(sin.sin_addr.s_addr),
                                                           AF_INET)) ==
                                        NULL) {
                                        perror("gethostbyaddr");
                                        exit(1);
                                    }
                                    strcpy((char *) &client_list[i].hostname, h->h_name);
                                }
                            }
                        } else {
                            perror("bind");
                            exit(1);
                        }
                    } else {
                        perror("socket");
                        exit(1);
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

