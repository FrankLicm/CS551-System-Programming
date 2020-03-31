/* THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING
 * A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Changmao Li */

#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>

int main(int argc, char *argv[]) {
    int check_argc_right = argc != 2;
    if(check_argc_right) {
        printf("Input error\n");
        printf("usage: ./uniqify [number of sorter processes>0] < [input file path]\n");
        return 1;
    }
    int n_sorter_processes = atoi(argv[1]);
    int check_processes_enough =  n_sorter_processes < 1;
    if(check_processes_enough) {
        printf("Not enough number of sorter processes or input error\n");
        printf("usage: ./uniqify [number of sorter processes>0] < [input file path]\n");
        return 1;
    }
    int fd1[n_sorter_processes][2];
    int fd2[n_sorter_processes][2];
    int i;
    for(i = 0; i < n_sorter_processes; i++) {
        if (pipe(fd1[i]) != -1) {
            if (pipe(fd2[i]) != -1) continue;
            printf("create pipe error\n");
            return 1;
        }
        else{
            printf("create pipe error\n");
            return 1;
        }
    }
    pid_t pid[n_sorter_processes];
    for(i = 0; i < n_sorter_processes; i++) {
        pid[i] = fork();
        if (pid[i] != -1) {
            if (pid[i] == 0) {
                int j;
                for (j = 0; j < n_sorter_processes; j++) {
                    if (i != j) {
                        if (close(fd1[j][0]) != -1) {
                            if (close(fd2[j][1]) != -1)continue;
                            printf("close fd error\n");
                            return 1;
                        }
                        else {
                            printf("close fd error\n");
                            return 1;
                        }
                    }
                }
                for (j = 0; j < n_sorter_processes; j++) {
                    if (close(fd1[j][1]) != -1) {
                        if (close(fd2[j][0]) != -1) continue;
                        printf("close fd error\n");
                        return 1;
                    }
                    else {
                        printf("close fd error\n");
                        return 1;
                    }
                }
                if (dup2(fd1[i][0], STDIN_FILENO) != -1) {
                    if (dup2(fd2[i][1], STDOUT_FILENO) != -1) {
                        execlp("sort", "sort", NULL);
                        if (close(fd1[i][0]) != -1) {
                            if (close(fd2[i][1]) != -1) continue;
                            printf("close fd error\n");
                            return 1;
                        }
                        else {
                            printf("close fd error\n");
                            return 1;
                        }
                    }
                    else {
                        printf("dup fd error\n");
                        return 1;
                    }
                }
                else {
                    printf("dup fd error\n");
                    return 1;
                }
            }
        }
        else {
            printf("create process error\n");
            return 1;
        }
    }
    int other_processes = fork();
    if(other_processes == 0) {
        for(i = 0; i < n_sorter_processes; i++) {
            if (close(fd1[i][0]) != -1) {
                if (close(fd1[i][1]) != -1) {
                    if (close(fd2[i][1]) != -1) continue;
                    printf("close fd error\n");
                    return 1;
                } else {
                    printf("close fd error\n");
                    return 1;
                }
            } else {
                printf("close fd error\n");
                return 1;
            }
        }
        char words[n_sorter_processes][42];
        FILE* from_sorter[n_sorter_processes];
        int n_empty = 0;
        for(i = 0; i < n_sorter_processes; i++) {
            from_sorter[i] = fdopen(fd2[i][0], "r");
            if (fgets(words[i], 42, from_sorter[i]) != NULL) continue;
            words[i][0] = '\0';
            n_empty++;
        }
        int next_word = -1;
        for (i = 0; i < n_sorter_processes; i++) {
            if (words[i][0] == '\0') continue;
            next_word = i;
            break;
        }
        for (i = next_word + 1; i < n_sorter_processes; i++) {
            if (words[i][0] == '\0' || strcmp(words[i], words[next_word]) >= 0) continue;
            next_word = i;
        }
        char word[42];
        strcpy(word, words[next_word]);
        int n_repeat = 1;
        while(n_empty < n_sorter_processes) {
            if (fgets(words[next_word], 42, from_sorter[next_word]) != NULL) {}
            else {
                n_empty++;
                words[next_word][0] = '\0';
            }
            next_word = -1;
            for (i = 0; i < n_sorter_processes; i++) {
                if (words[i][0] == '\0') continue;
                next_word = i;
                break;
            }
            for (i = next_word + 1; i < n_sorter_processes; i++) {
                if (words[i][0] == '\0' || strcmp(words[i], words[next_word]) >= 0) continue;
                next_word = i;
            }
            if (next_word != -1) {
                if (strcmp(word, words[next_word]) != 0) {
                    printf("%5d %s", n_repeat, word);
                    strcpy(word, words[next_word]);
                    n_repeat = 1;
                }
                else n_repeat++;
            }
            else break;
        }
        printf("%5d %s", n_repeat, word);
        for(i = 0; i < n_sorter_processes; i++) fclose(from_sorter[i]);
    }
    else if(other_processes != -1){
        FILE* from_parser[n_sorter_processes];
        for(i = 0; i < n_sorter_processes; i++) {
            if (close(fd1[i][0]) != -1) {
                if (close(fd2[i][0]) != -1) {
                    if (close(fd2[i][1]) != -1) {
                        from_parser[i] = fdopen(fd1[i][1], "w");
                    }
                    else {
                        printf("close fd error\n");
                        return 1;
                    }
                }
                else {
                    printf("close fd error\n");
                    return 1;
                }
            }
            else {
                printf("close fd error\n");
                return 1;
            }
        }
        int word_index = 0;
        char word[42];
        int n_processes = 0;
        if (scanf("%*[^a-zA-Z]")!=EOF){
            int read_string;
            while((read_string = fgetc(stdin))!= EOF) {
                if (!isalpha(read_string)) {
                    word[word_index] = '\0';
                    if (strlen(word) <= 4) {
                        word[0] = '\0';
                        word_index = 0;
                    }
                    else {
                        fputs(word, from_parser[n_processes % n_sorter_processes]);
                        fputs("\n", from_parser[n_processes % n_sorter_processes]);
                        n_processes++;
                        word[0] = '\0';
                        word_index = 0;
                    }
                }
                else {
                    word[word_index++] = tolower(read_string);
                    if (word_index != 25) continue;
                    word[25] = '\0';
                    fputs(word, from_parser[n_processes % n_sorter_processes]);
                    fputs("\n", from_parser[n_processes % n_sorter_processes]);
                    scanf("%*[a-zA-Z]");
                    n_processes++;
                    word[0] = '\0';
                    word_index = 0;
                }
            }
        }
        for(i = 0; i < n_sorter_processes; i++) fclose(from_parser[i]);
    }
    else{
        printf("create process error\n");
        return 1;
    }
    while(wait(NULL) != -1);
    return 0;
}
