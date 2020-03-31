/* THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING
 * A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Changmao Li */
#include "defs.h"

int process_id;
int shared_memory_id;
SharedMemory *shared_memory;
int message_queue_id;

void clear_summaries(){
    shared_memory->summaries[process_id].pid = 0;
    shared_memory->summaries[process_id].number_skipped = 0;
    shared_memory->summaries[process_id].perfect_num_found = 0;
    shared_memory->summaries[process_id].number_tested = 0;
}

/*handler for quit*/
void handler(int signum) {
    shared_memory->total_summaries[0] = shared_memory->total_summaries[0] + shared_memory->summaries[process_id].perfect_num_found;
    shared_memory->total_summaries[1] = shared_memory->total_summaries[1] + shared_memory->summaries[process_id].number_tested;
    shared_memory->total_summaries[2] = shared_memory->total_summaries[2] + shared_memory->summaries[process_id].number_skipped;
    if(process_id != -1) {
        clear_summaries();
    }
    execl("./report", "./report", "-k", 0);
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc >= 2) {
        int start_num = atoi(argv[1]);
        if (start_num >= 2 && start_num < 33554432) {
            struct sigaction signal;
            memset(&signal, 0, sizeof(signal));
            signal.sa_handler = handler;
            if (sigaction(SIGINT, &signal, NULL) == 0) {
                if (sigaction(SIGQUIT, &signal, NULL) == 0) {
                    if (sigaction(SIGHUP, &signal, NULL) == 0) {
                        int current_num;
                        int whichint = 0;
                        int flag = 0;
                        int whichbit = 0;
                        if ((shared_memory_id = shmget(88617, sizeof(SharedMemory), 0)) != -1) {
                            if ((shared_memory = shmat(shared_memory_id, NULL, 0)) != (void *) -1) {
                                if ((message_queue_id = msgget(88617, 0)) != -1) {
                                    message msg;
                                    int current_pid;
                                    current_pid = getpid();
                                    msg.msg_type = 1;
                                    msg.msg_text = current_pid;
                                    if (msgsnd(message_queue_id, &msg, sizeof(msg.msg_text), 0) != -1) {
                                        msgrcv(message_queue_id, (void *) &msg, sizeof(msg.msg_text), 4, 0);
                                        process_id = msg.msg_text;
                                        if (shared_memory->summaries[process_id].pid == current_pid) {
                                            current_num = start_num;
                                            while (1) {
                                                if (current_num >= 33554432) {
                                                    current_num = 2;
                                                    flag = 1;
                                                }
                                                if (current_num == start_num && flag == 1) {
                                                    handler(3);
                                                }
                                                whichbit = current_num % 32;
                                                whichint = current_num / 32;
                                                if ((shared_memory->number_bitmap[whichint] & (1 << whichbit)) != 0) {
                                                    shared_memory->summaries[process_id].number_skipped++;
                                                } else {
                                                    int sum = 0;
                                                    int i;
                                                    for(i = 1; i < current_num; i++) {
                                                        if (current_num % i != 0) continue;
                                                        sum += i;
                                                    }
                                                    if (current_num == sum) {
                                                        msg.msg_type = 3;
                                                        msg.msg_text = current_num;
                                                        if (msgsnd(message_queue_id, &msg, sizeof(msg.msg_text), 0) ==
                                                            -1) {
                                                            perror("msgsnd error");
                                                            exit(1);
                                                        } else {
                                                            shared_memory->summaries[process_id].perfect_num_found++;
                                                        }
                                                    }
                                                    shared_memory->number_bitmap[whichint] |= (1 << whichbit);
                                                    shared_memory->summaries[process_id].number_tested++;
                                                }
                                                current_num++;
                                            }
                                        } else {
                                            printf("process id inconsistent\n");
                                            exit(1);
                                        }
                                    } else {
                                        perror("msgsnd error");
                                        exit(1);
                                    }
                                } else {
                                    perror("msgget error");
                                    exit(1);
                                }
                            } else {
                                perror("shmat error");
                                exit(1);
                            }
                        } else {
                            perror("shmget error");
                            exit(1);
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
            perror("the start number is out of range.\n");
            exit(1);
        }
    } else {
        printf("Argument is not enough\n");
        exit(1);
    }
}
