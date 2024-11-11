#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>

#define LEN_BUF_FILE 5


int countCompletedFiles = 0;

void PrintNum(int num)
{
    if (num > 9) {
        PrintNum(num / 10);
        char out = '0' + num % 10;
        write(1, &out, 1);
    } else {
        char out = '0' + num;
        write(1, &out, 1);
    }
}

int PrepareString(char* filename)
{
    int filenameSize = strlen(filename);

    if (filename[filenameSize - 1] == '\n') {
        --filenameSize;
        filename[filenameSize] = 0;
    }

    return filenameSize;
}

void GenerateRandomFile(char* buf)
{
    for (char* p = buf; p - buf < LEN_BUF_FILE; ++p) {
        *p = 'A' + rand() % 26;
    }
}

int CheckFile(char* filename) {
    int fd = open(filename, O_CREAT | O_EXCL, 0666);
    if (fd == -1) {
        return 1;
    }
    close(fd);
    return 0;
}

void SignalAdapt()
{
    write(1, "\nCount completed files: ", 25);
    PrintNum(countCompletedFiles);
    write(1, "\n", 1);
}


int main(int argc, char** argv)
{
    srand(time(NULL));

    struct sigaction action;
    action.sa_handler = SignalAdapt;
    sigprocmask(0, NULL, &action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGINT, &action, NULL);

    char whileWords[100];
    memset(whileWords, 0, 100);
    strcpy(whileWords, "Print filename or $ to finish program: ");
    
    while (1) {
        char filename[FILENAME_MAX];
        memset(filename, 0, FILENAME_MAX);

        write(1, whileWords, 40);

        read(0, filename, FILENAME_MAX);

        int filenameSize = PrepareString(filename); 
        
        if (strcmp(filename, "$") == 0) {
            break;
        }

        char messageFile[LEN_BUF_FILE + 1];
        memset(messageFile, 0, LEN_BUF_FILE + 1);

        while (GenerateRandomFile(messageFile), CheckFile(messageFile)) {  }

        int fdRead = open(messageFile, O_RDONLY);
        int fdWrite = open(messageFile, O_WRONLY);

        int forkValue = fork();

        if (forkValue == 0) {
            close(fdRead);
            dup2(fdWrite, 1);
            close(2);

            execl("/bin/ls", "ls", "-ld", filename, NULL);
            exit(1);
        } else if (forkValue == -1) {
            write(2, "\nERROR: can't fork\n", 20);
            unlink(messageFile);
        }

        close(fdWrite);

        int sonResult;
        wait(&sonResult);

        if (sonResult == 256) {
            write(2, "\nERROR: cant exec ls in son");
        }
        
        char fileType;
        int readBytes = read(fdRead, &fileType, 1);
        close(fdRead);

        if (readBytes) {
            ++countCompletedFiles;
            switch (fileType) {
            case '-':
                write(1, "File type: regular file\n", 25);
                break;
            case 'd':
                write(1, "File type: directory\n", 22);
                break;
            case 'p':
                write(1, "File type: named pipe\n", 23);
                break;
            case 'l':
                write(1, "File type: soft link\n", 22);
                break;
            case 'c':
                write(1, "File type: character special files\n", 34);
                break;
            case 'b':
                write(1, "File type: block special files\n", 32);
                break;
            case 's':
                write(1, "File type: socket\n", 19);
                break;
            case 'D':
                write(1, "File type: door\n", 17);
                break;
            
            default:
                break;
            }
        } else {
            write(1, "Can't find file\n", 17);
        }
        unlink(messageFile);
    }

    // int forkValue = fork();
    // if (forkValue == 0) {
    //     sigaction(SIGINT, &(struct sigaction){.sa_handler = SIG_DFL}, NULL);
    //     strcpy(buf, "It's son\n");
    //     write(1, "COOL", 4);
    //     while (1) {}
    // }
    // printf("DAAD");
    // wait(NULL);
    exit(0);
}
