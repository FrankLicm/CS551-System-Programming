/* THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING
 * A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Changmao Li */
#include "defs.h"

int process_id = -1;
int shared_memory_id;
SharedMemory *shared_memory;
int message_queue_id;
int total_perfect = 0;

/*handler for kill*/
void handler(int signum) {
    int i;
    for(i = 0; i < 20; i++) {
        if (shared_memory->summaries[i].pid == 0) continue;
        if (kill(shared_memory->summaries[i].pid, SIGINT) == 0) continue;
        fprintf(stderr, "kill pid %d error: ", shared_memory->summaries[i].pid);
        perror("");
    }
    sleep(5);
    if (shmdt(shared_memory) == 0) {
        if (!shmctl(shared_memory_id, IPC_RMID, 0)) {
            if (!msgctl(message_queue_id, IPC_RMID, NULL)) {
                exit(0);
            } else {
                perror("msgctl error");
                exit(1);
            }
        } else {
            perror("shmctl error");
            exit(1);
        }
    } else {
        perror("shmdt error");
        exit(1);
    }
}

void clear_shared_memory(){
    memset(shared_memory->number_bitmap, 0, sizeof(shared_memory->number_bitmap));
    memset(shared_memory->perfect_found, 0, sizeof(shared_memory->perfect_found));
    memset(shared_memory->summaries, 0, sizeof(shared_memory->summaries));
    memset(shared_memory->total_summaries, 0, sizeof(shared_memory->total_summaries));
}

int main(int argc, char *argv[]) {
    if ((shared_memory_id = shmget(88617, sizeof(SharedMemory), IPC_CREAT | IPC_EXCL | 0666)) != -1) {
        shared_memory = shmat(shared_memory_id, NULL, 0);
        clear_shared_memory();
        if ((message_queue_id = msgget(88617, IPC_CREAT | IPC_EXCL | 0666)) != -1) {
            struct sigaction signal;
            memset(&signal, 0, sizeof(signal));
            signal.sa_handler = handler;
            if (sigaction(SIGINT, &signal, NULL) == 0) {
                if (sigaction(SIGQUIT, &signal, NULL) == 0) {
                    if (sigaction(SIGHUP, &signal, NULL) == 0) {
                        message msg;
                        while (1) {
                            msgrcv(message_queue_id, &msg, sizeof(msg.msg_text), -3, 0);
                            switch (msg.msg_type) {
                                case 1: {
                                    int i;
                                    for (i = 0; i < 20; i++) {
                                        if (shared_memory->summaries[i].pid != 0) continue;
                                        process_id = i;
                                    }
                                    if (process_id != -1) {
                                        shared_memory->summaries[process_id].pid = msg.msg_text;
                                        msg.msg_type = 4;
                                        msg.msg_text = process_id;
                                        if (msgsnd(message_queue_id, &msg, sizeof(msg.msg_text), 0) != -1) break;
                                        perror("msgsnd error");
                                        exit(1);
                                    } else {
                                        kill(msg.msg_text, SIGKILL);
                                    }
                                    break;
                                }
                                case 2: {
                                    msg.msg_type = 5;
                                    msg.msg_text = getpid();
                                    if (msgsnd(message_queue_id, &msg, sizeof(msg.msg_text), 0) != -1) break;
                                    perror("msgsnd error");
                                    exit(1);
                                }
                                case 3: {
                                    int found = 0;
                                    int i;
                                    for (i = 0; i < total_perfect; i++) {
                                        if (msg.msg_text != shared_memory->perfect_found[i]) continue;
                                        found = 1;
                                        break;
                                    }
                                    if (found != 0) break;
                                    shared_memory->perfect_found[total_perfect++] = msg.msg_text;
                                    break;
                                }
                            }
                        }
                    } else {
                        perror("sigaction SIGHUP error");
                        exit(1);
                    }
                } else {
                    perror("sigaction SIGQUIT error");
                    exit(1);
                }
            } else {
                perror("sigaction SIGINT error");
                exit(1);
            }
        } else {
            perror("msgget error");
            exit(1);
        }
    } else {
        perror("shmget error");
        exit(1);
    }
}
