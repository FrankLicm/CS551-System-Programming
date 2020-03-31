/*THIS CODE IS MY OWN WORK. IT WAS WRITTEN WITHOUT CONSULTING
 * A TUTOR OR CODE WRITTEN BY OTHER STUDENTS --Changmao Li*/
#include "defs.h"

int sock_fd;

int main(int argc, char *argv[]) {
    if (argc >= 3) {
        struct sockaddr_in sock_in;
        sock_in.sin_addr.s_addr = *(long *) gethostbyname(argv[1])->h_addr;
        sock_in.sin_family = AF_INET;
        sock_in.sin_port = atoi(argv[2]);
        for (;;) {
            if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) != -1) {
                if (connect(sock_fd, (struct sockaddr *) &sock_in, sizeof(sock_in)) != -1) break;
                printf("Wait for connection\n");
                close(sock_fd);
                sleep(10);
                continue;
            } else {
                perror("socket");
                exit(1);
            }
        }
        write(sock_fd, argc != 4 || strcmp(argv[3], "-k") ? "report|" : "quit|", 100);
        char buffer[1000];
        memset(&buffer, 0, sizeof(buffer));
        read(sock_fd, buffer, 1000);
        printf("%s", buffer);
    } else {
        printf("Arguments are not enough.\n");
        exit(1);
    }
}