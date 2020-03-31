/* THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING
 * A TUTOR OR CODE WRITER BY OTHER STUDENTS - Changmao Li
 */
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <ar.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <utime.h>
#include <dirent.h>

struct meta{
    char name[16]; //room for null
    int mode;
    time_t mtime; // a time_t is long
    int size;
    int uid;
    int gid;
};

void clean_null(char*buff, int size){
    int i;
    for(i=0;i<size;i++){
        if (buff[i]=='\0'){
            buff[i]=' ';
        }
    }
}

void add_slash(char*buff, int size){
    int i;
    for(i=0;i<size;i++){
        if (buff[i]==' '){
            buff[i]='/';
            break;
        }
    }
}

int fill_ar_hdr(char*filename, struct ar_hdr*hdr){
    struct stat file_stat;
    if (stat(filename, &file_stat) == -1) {
        printf("%s: No such file or directory\n", filename);
        return 0;
    }
    if (!S_ISREG(file_stat.st_mode)) {
        printf("%s: The file is not a regular file\n", filename);
        return 0;
    }
    memset(hdr->ar_name, ' ', sizeof(hdr->ar_name));
    strcpy(hdr->ar_name, filename);
    clean_null(hdr->ar_name, sizeof(hdr->ar_name));
    add_slash(hdr->ar_name, sizeof(hdr->ar_name));
    char buff[16];
    memset(buff, ' ', 16);
    sprintf(buff, "%ld", file_stat.st_mtime);
    memset(hdr->ar_date, ' ', sizeof(hdr->ar_date));
    strncpy(hdr->ar_date,  buff, sizeof(hdr->ar_date));
    clean_null(hdr->ar_date, sizeof(hdr->ar_date));
    memset(buff, ' ', 16);
    sprintf(buff, "%d", file_stat.st_uid);
    memset(hdr->ar_uid, ' ', sizeof(hdr->ar_uid));
    strncpy(hdr->ar_uid,  buff, sizeof(hdr->ar_uid));
    clean_null(hdr->ar_uid, sizeof(hdr->ar_uid));
    memset(buff, ' ', 16);
    sprintf(buff, "%d", file_stat.st_gid);
    memset(hdr->ar_gid, ' ', sizeof(hdr->ar_gid));
    strncpy(hdr->ar_gid, buff, sizeof(hdr->ar_gid));
    clean_null(hdr->ar_gid, sizeof(hdr->ar_gid));
    memset(buff, ' ', 16);
    sprintf(buff, "%o", file_stat.st_mode);
    memset(hdr->ar_mode, ' ', sizeof(hdr->ar_mode));
    strncpy(hdr->ar_mode,  buff, sizeof(hdr->ar_mode));
    clean_null(hdr->ar_mode, sizeof(hdr->ar_mode));
    memset(buff, ' ', 16);
    sprintf(buff, "%lld", (long long int)file_stat.st_size);
    memset(hdr->ar_size, ' ', sizeof(hdr->ar_size));
    strncpy(hdr->ar_size,  buff, sizeof(hdr->ar_size));
    clean_null(hdr->ar_size, sizeof(hdr->ar_size));
    memset(buff, ' ', 16);
    sprintf(buff, "%2s", ARFMAG);
    strncpy(hdr->ar_fmag, buff, sizeof(hdr->ar_fmag));
    return file_stat.st_size;
}

int fill_meta(struct ar_hdr hdr, struct meta*meta){
    char* ptr;
    strcpy(meta->name, hdr.ar_name);
    meta->mode = (mode_t) strtol(hdr.ar_mode, &ptr, 8);
    meta->mtime = (time_t) atoll (hdr.ar_date);
    meta->size = (int)strtol(hdr.ar_size, (char**)NULL, 10);
    meta->uid = (int)strtol(hdr.ar_uid, (char**)NULL, 10);
    meta->gid = (int)strtol(hdr.ar_gid, (char**)NULL, 10);
    return meta->size;
}

void add_null(char*buff, int size){
    int i;
    for (i=0;i<size;i++){
        if(buff[i]==' '){
            buff[i]='\0';
        }
    }
}

void delete_slash(char*buff, int size){
    int i;
    for (i=0;i<size;i++){
        if(buff[i]=='/'){
            buff[i]='\0';
        }
    }
}

void delete_null_and_add_slash(struct ar_hdr *hdr){
    char*head = (char*)malloc(sizeof(struct ar_hdr));
    memcpy(head, hdr, sizeof(struct ar_hdr));
    clean_null(head, sizeof(struct ar_hdr));
    add_slash(head, sizeof(struct ar_hdr));
    memcpy(hdr, head, sizeof(struct ar_hdr));
    free(head);
}

void parse_head(char* head, struct ar_hdr *hdr){
    add_null(head, sizeof(struct ar_hdr));
    delete_slash(head, sizeof(struct ar_hdr));
    memcpy(hdr, head, sizeof(struct ar_hdr));
}

int open_archive(char * file_name)
{
    int in_fd = open(file_name, O_RDONLY);
    char buf[16];

    if (in_fd == -1) {
        printf("%s: No such file or directory\n", file_name);
        exit(1);
    }
    else {
        if (read(in_fd, buf, 8) != 8){
            printf("%s: File format not recognized\n", file_name);
            exit(1);
        }
        buf[8] = '\0';
        if (strcmp(buf, ARMAG) != 0) {
            printf("%s: File format not recognized\n", file_name);
            exit(1);
        }
    }
    return in_fd;
}

int append(char * archive_name, char * file_name)
{
    struct ar_hdr* hdr= ( struct ar_hdr*) malloc(sizeof(struct ar_hdr));
    char check_archive[16];
    char buf[1];
    int bytes_written = 0;
    int read_error;
    int out_fd;
    int in_fd;
    if (!file_name)
        return -1;
    int file_size_bytes = fill_ar_hdr(file_name, hdr);
    if (!file_size_bytes)
        return -1;

    // check if archive file exists, if not creat one, and make sure format is right.
    in_fd = open(archive_name, O_RDONLY);
    if ((read_error = read(in_fd, check_archive, 8)) == 8) {
        check_archive[8] = '\0';
        if (strcmp(check_archive, ARMAG) != 0){
            printf("%s: File format incorrect\n", file_name);
            exit(1);
        }
    }
    else if (read_error == -1) {
        out_fd = open(archive_name, O_WRONLY | O_CREAT, 0666);
        printf("creating %s\n", archive_name);
        write(out_fd, "!<arch>\n", 8);
        close(out_fd);
    }
    close(in_fd);

    out_fd = open(archive_name, O_RDWR | O_APPEND);
    in_fd = open(file_name, O_RDONLY);
    if (in_fd == -1) {
        printf("%s: No such file or directory\n", file_name);
        exit(1);
    }
    int n_read = write(out_fd, hdr, sizeof(struct ar_hdr));
    if( n_read != sizeof(struct ar_hdr)) {
        perror("Error writing the header");
        exit(-1);
    }
    while (bytes_written < file_size_bytes) {
        read(in_fd, buf, 1);
        bytes_written += write(out_fd, buf, 1);
    }
    buf[0] = '\n';
    if (file_size_bytes%2 != 0)
        write(out_fd, buf, 1);
    close(out_fd);
    close(in_fd);
    free(hdr);
    return 1;
}

int extract(char *archive_name, char *file_name, int o)
{
    struct utimbuf file_times;
    int * file_size = malloc(sizeof(int));
    struct meta* meta = (struct meta*) malloc(sizeof(struct meta));
    int in_fd = open_archive(archive_name);
    int file_size1 = 0;
    int offset=-1;
    struct ar_hdr* hdr= (struct ar_hdr*) malloc(sizeof(struct ar_hdr));
    char*head = (char*)malloc(sizeof(struct ar_hdr));
    while (read(in_fd, head, sizeof(struct ar_hdr)) == sizeof(struct ar_hdr)) {
        parse_head(head, hdr);
        file_size1 = (int)strtol(hdr->ar_size, (char**)NULL, 10);
        if (strcmp(file_name, hdr->ar_name) == 0) {
            *file_size = fill_meta(*hdr, meta);
            offset = lseek(in_fd, -60, SEEK_CUR);
            close(in_fd);
            break;
        }
        if (file_size1%2)
            file_size1 += 1;
        lseek(in_fd, file_size1, SEEK_CUR);
    }
    close(in_fd);
    int bytes_written = 0;
    char buf[1];
    int out_fd;
    if (offset == -1) {
        printf("%s: No such file\n", file_name);
        free(file_size);
        free(hdr);
        free(meta);
        return -1;
    }
    in_fd = open_archive(archive_name);
    out_fd = open(file_name, O_RDWR | O_TRUNC | O_CREAT, 0666);
    lseek(in_fd, offset + 60, SEEK_SET);
    while (bytes_written < *file_size) {
        read(in_fd, buf, 1);
        bytes_written += write(out_fd, buf, 1);
    }
    fchmod(out_fd, meta->mode);
    if(o){
        file_times.actime = meta->mtime;
        file_times.modtime = meta->mtime;
        utime(file_name, &file_times);
    }
    close(out_fd);
    close(in_fd);
    free(file_size);
    free(hdr);
    free(meta);
    free(head);
    return 1;
}

void print_mode(int st_mode)
{
    if (st_mode & S_IRUSR) putchar('r'); else putchar('-');
    if (st_mode & S_IWUSR) putchar('w'); else putchar('-');
    if (st_mode & S_IXUSR) putchar('x'); else putchar('-');
    if (st_mode & S_IRGRP) putchar('r'); else putchar('-');
    if (st_mode & S_IWGRP) putchar('w'); else putchar('-');
    if (st_mode & S_IXGRP) putchar('x'); else putchar('-');
    if (st_mode & S_IROTH) putchar('r'); else putchar('-');
    if (st_mode & S_IWOTH) putchar('w'); else putchar('-');
    if (st_mode & S_IXOTH) putchar('x'); else putchar('-');
}

int tv(struct ar_hdr hdr)
{
    struct meta* meta = (struct meta*) malloc(sizeof(struct meta));
    int file_mode;
    int file_size;
    time_t timestamp;
    struct tm timestruct;
    char months[12][4] = {"Jan\0", "Feb\0", "Mar\0", "Apr\0", "May\0", "Jun\0", "Jul\0", "Aug\0", "Sep\0", "Oct\0", "Nov\0", "Dec\0"};
    file_size = fill_meta(hdr, meta);
    file_mode = meta->mode;
    print_mode(file_mode);
    putchar('\t');
    printf("%d", meta->uid);
    putchar('/');
    printf("%d", meta->gid);
    putchar('\t');
    printf("%d", meta->size);
    putchar('\t');
    timestamp = meta->mtime;
    timestruct = *localtime(&timestamp);
    printf("%s %d %02d:%02d %d", months[timestruct.tm_mon], timestruct.tm_mday, timestruct.tm_hour,
           timestruct.tm_min, timestruct.tm_year+1900);
    putchar('\t');
    printf("%s\n", meta->name);
    free(meta);
    return file_size;
}

int delete(char *archive_name, char *file_names[], int num_of_files)
{
    int in_fd;
    int out_fd;
    struct ar_hdr* hdr= ( struct ar_hdr*) malloc(sizeof(struct ar_hdr));
    int *isdeleted = (int*)malloc(sizeof(int) * (num_of_files));
    memset(isdeleted, 0, num_of_files);
    struct stat old_stat;
    if (stat(archive_name, &old_stat) == -1) {
        printf("%s: No such file or directory\n", archive_name);
        exit(1);
    }
    in_fd = open_archive(archive_name);
    unlink(archive_name);
    if((out_fd = open(archive_name, O_CREAT | O_WRONLY | O_APPEND, old_stat.st_mode)) == -1) {
        printf("The new archive file cannot be created\n");
        exit(1);
    }
    write(out_fd, ARMAG, SARMAG);
    lseek(in_fd, SARMAG, SEEK_SET);
    char*head = (char*)malloc(sizeof(struct ar_hdr));
    while(read(in_fd, head, sizeof(struct ar_hdr)) == sizeof(struct ar_hdr)) {
        parse_head(head, hdr);
        int file_size_bytes = (int)strtol(hdr->ar_size, (char**)NULL, 10);
        int flag = 0;
        int i;
        for(i = 0; i < num_of_files; i++){
            if(!strcmp(file_names[i], hdr->ar_name) && isdeleted[i] == 0) {
                isdeleted[i] = 1;
                flag = 1;
                break;
            }
        }
        if(flag == 1) {
            if(file_size_bytes % 2 == 1)
                file_size_bytes++;
            lseek(in_fd, file_size_bytes, SEEK_CUR);
        } else {
            delete_null_and_add_slash(hdr);
            write(out_fd, hdr, sizeof(struct ar_hdr));
            int bytes_read = 0;
            char buffer[1];
            while (bytes_read < file_size_bytes){
                read(in_fd, buffer, 1);
                write(out_fd, buffer, 1);
                bytes_read++;
            }
            if (lseek(in_fd, 0, SEEK_CUR) % 2 == 1){
                lseek(in_fd, 1, SEEK_CUR);
            }
            if (lseek(out_fd, 0, SEEK_CUR) % 2 == 1){
                write(out_fd, "\n", 1);
            }
        }
    }
    close(out_fd);
    close(in_fd);
    free(isdeleted);
    free(hdr);
    free(head);
    return 1;
}

int append_all(char *archive_name)
{
    DIR * dirp;
    struct dirent * current_dir;
    int skip_file;
    if (!archive_name)
        return -1;
    dirp = opendir(".");
    current_dir = readdir(dirp);
    while (current_dir != NULL) {
        skip_file = (current_dir->d_type != DT_REG) || strcmp(current_dir->d_name, archive_name) == 0;
        if (!skip_file) {
            append(archive_name, current_dir->d_name);
        }
        current_dir = readdir(dirp);
    }
    closedir(dirp);
    return 1;
}

void print_usage(){
    printf("Usage: ./myar key afile name ...\n");
    printf(" keys:\n");
    printf("  q - quickly append named files to archive\n");
    printf("  x - extract named files\n");
    printf("  xo - extract named files restoring time\n");
    printf("  t - print a concise table of contents of the archive\n");
    printf("  tv - print a verbose table of contents of the archive\n");
    printf("  d - delete named files from archive\n");
    printf("  A - quickly append all \"regular\" files in the current directory\n");
}

int main(int argc, char *argv[])
{
    int i;
    int in_fd;
    int file_size = 0;
    if (argc <3) {
        printf("%s: Not enough arguments\n", argv[0]);
        print_usage();
    }
    if (argc >=3) {
        switch (argv[1][0]) {
            case 'q':
                if (argc < 4) {
                    printf("%s: Not enough arguments\n", argv[0]);
                    exit(1);
                }
                for (i = 3; i < argc; ++i) {
                    append(argv[2], argv[i]);
                }
                break;
            case 'x':
                if(argc < 4) {
                    printf("%s\n", "Not enough arguments");
                    return -1;
                }
                if(argv[1][1] == 'o'){
                    for (i = 3; i < argc; ++i) {
                        extract(argv[2], argv[i], 1);
                    }
                } else{
                    for (i = 3; i < argc; ++i) {
                        extract(argv[2], argv[i], 0);
                    }
                }
                break;
            case 't':
                if(argv[1][1] == 'v'){
                    in_fd = open_archive(argv[2]);
                    struct ar_hdr* hdr= (struct ar_hdr*) malloc(sizeof(struct ar_hdr));
                    char*head = (char*)malloc(sizeof(struct ar_hdr));
                    while (read(in_fd, head, sizeof(struct ar_hdr)) == sizeof(struct ar_hdr)) {
                        parse_head(head, hdr);
                        file_size = tv(*hdr);
                        if (file_size%2)
                            file_size += 1;
                        lseek(in_fd, file_size, SEEK_CUR);
                    }
                    if (close(in_fd) == -1){
                        free(hdr);
                        free(head);
                        exit(1);
                    }
                    free(hdr);
                    free(head);
                } else{
                    in_fd = open_archive(argv[2]);
                    struct ar_hdr* hdr= (struct ar_hdr*) malloc(sizeof(struct ar_hdr));
                    char*head = (char*)malloc(sizeof(struct ar_hdr));
                    while (read(in_fd, head, sizeof(struct ar_hdr)) == sizeof(struct ar_hdr)) {
                        parse_head(head, hdr);
                        printf("%s\n", hdr->ar_name);
                        file_size = (int)strtol(hdr->ar_size, (char**)NULL, 10);
                        if (file_size%2)
                            file_size += 1;
                        lseek(in_fd, file_size, SEEK_CUR);
                    }
                    if (close(in_fd) == -1){
                        free(hdr);
                        free(head);
                        exit(1);
                    }
                    free(hdr);
                    free(head);
                }
                break;
            case 'd':{
                if(argc < 4) {
                    printf("%s\n", "Not enough arguments");
                    return -1;
                }
                char* filenames[argc-3];
                int l;
                for (l=0; l < argc - 3; l++) {
                    filenames[l] = argv[l + 3];
                }
                delete(argv[2], filenames, argc-3);
                break;
            }
            case 'A':
                append_all(argv[2]);
                break;
            default:
                printf("wrong command\n");
                print_usage();
                break;
        }
    }
    return 0;
}

