/* THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING
 * A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Changmao Li */
#include "defs.h"


int process_id = -1;
int shared_memory_id;
SharedMemory *shared_memory;
int message_queue_id;


int main(int argc, char *argv[]) {
    if ((shared_memory_id = shmget(88617, sizeof(SharedMemory), 0)) != -1) {
        if ((shared_memory = shmat(shared_memory_id, NULL, 0)) != (void *) -1) {
            if ((message_queue_id = msgget(88617, 0)) != -1) {
                message msg;
                if (argc > 1 && !strcmp(argv[1], "-k")) {
                    msg.msg_type = 2;
                    msg.msg_text = 0;
                    if (msgsnd(message_queue_id, &msg, sizeof(msg.msg_text), 0) != -1) {
                        msgrcv(message_queue_id, &msg, sizeof(msg.msg_text), 5, 0);
                        if (kill(msg.msg_text, SIGINT) != 0) {
                            fprintf(stderr, "kill pid %d error: ", msg.msg_text);
                            perror("");
                            exit(1);
                        }
                    } else {
                        perror("msgsnd error");
                        exit(1);
                    }
                }
                int i;
                for (i = 0; i < 20; i++) {
                    if (shared_memory->perfect_found[i] == 0) continue;
                    printf("perfect number found: ");
                    printf("%d\n", shared_memory->perfect_found[i]);
                }
                for (i = 0; i < 20; i++) {
                    if (shared_memory->summaries[i].pid == 0) continue;
                    printf("pid: %d ", shared_memory->summaries[i].pid);
                    printf("perfect number found: %d ", shared_memory->summaries[i].perfect_num_found);
                    printf("tested number: %d ", shared_memory->summaries[i].number_tested);
                    printf("skipped number: %d\n", shared_memory->summaries[i].number_skipped);
                }
                int total_perfect = 0;
                total_perfect = total_perfect + shared_memory->total_summaries[0];
                for (i = 0; i < 20; i++) {
                    total_perfect = total_perfect + shared_memory->summaries[i].perfect_num_found;
                }
                int total_tested = 0;
                total_tested = total_tested + shared_memory->total_summaries[1];
                for (i = 0; i < 20; i++) {
                    total_tested = total_tested + shared_memory->summaries[i].number_tested;
                }
                int total_skipped = 0;
                total_skipped = total_skipped + shared_memory->total_summaries[2];
                for (i = 0; i < 20; i++) {
                    total_skipped = total_skipped + shared_memory->summaries[i].number_skipped;
                }
                printf("total perfect number: %d\n", total_perfect);
                printf("total tested number: %d\n", total_tested);
                printf("total skipped number: %d\n", total_skipped);
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
}