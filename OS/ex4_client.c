// Omer Arad 314096389
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

char* tosrv = "to_srv.txt";

void alarm_hand(int signo) {
    char clientFile[30] = "to_client_";
    char pid[10];
    sprintf(pid, "%d", getpid());
    strcat(clientFile, pid);
    strcat(clientFile, ".txt");

    // del to_srv.
    if (access(clientFile, F_OK) == 0) {
        if (remove(clientFile) < 0) {
            printf("ERROR_FROM_EX4\n");
//            perror("remove: ");
            exit(1);
        }
    }

    printf("Client closed because no response was received from the server for 30 seconds");

    // del to_srv.
    if (access(tosrv, F_OK) == 0) {
        if (remove(tosrv) < 0) {
            printf("ERROR_FROM_EX4\n");
//            perror("remove: ");
            exit(1);
        }
    }

    exit(1);
}

void print_result(int signo) {
    char clientFile[30] = "to_client_";
    char pid[10];
    char buf[63];
    alarm(0);

    sprintf(pid, "%d", getpid());
    strcat(clientFile, pid);
    strcat(clientFile, ".txt");

    int fdclient = open(clientFile, O_RDONLY, 0777);
    if (fdclient < 0) {
        printf("ERROR_FROM_EX4\n");
//        perror("open: ");
        remove(clientFile);
        // and all other files.
        exit(1);
    }

    int numBytes = read(fdclient, &buf, 63);
    if(numBytes < 0) {
        printf("ERROR_FROM_EX4\n");
//        perror("read: ");
        close(fdclient);
        remove(clientFile);
        exit(1);
    }
//    printf("read %d bytes\n", numBytes);
    buf[numBytes] = '\0';
    printf("%s\n", buf);

    if(close(fdclient) < 0) {
        printf("ERROR_FROM_EX4\n");
//        perror("close");
        remove(clientFile);
        exit(1);
    }

    if(remove(clientFile) < 0) {
        printf("ERROR_FROM_EX4\n");

//        perror("remove");
        exit(1);
    }
}

int main(int argc, char* argv[]) {
    int fdclient;
    int fdsrv;
    char cwd[1024];
    int count = 0;
    int buf[10];
    int r = syscall(SYS_getrandom, buf, sizeof(int) * 10, 0);
    if (r == -1) {
        printf("ERROR_FROM_EX4\n");
//        perror("getrandom: ");
        exit(-1);
    }

    if (argc != 5) {
        printf("ERROR_FROM_EX4\n");
        exit(1);
    }

    // create input string
    char output[100] = "";
    char pid[8];
    sprintf(pid, "%d", getpid());
    strcat(output, pid);
    strcat(output, " ");
    strcat(output, argv[2]);
    strcat(output, " ");
    strcat(output, argv[3]);
    strcat(output, " ");
    strcat(output, argv[4]);
    strcat(output,"\n");

    // try to create to_srv
//    remove(tosrv);
    fdsrv = open(tosrv, O_CREAT | O_EXCL | O_RDWR, 0777);
    while (fdsrv < 0 && count < 10) {
        int time = buf[count] % 6;
        time = (time < 0) ? -time:time;
        time = (time == 0) ? 1:time;
        count++;
        sleep(time);
//        printf("trying to open to_srv again\n");
        fdsrv = open(tosrv, O_WRONLY | O_CREAT | O_EXCL, 0777);
    }

    if(count == 10) {
//        perror("Couldn't open to_srv:");
        printf("ERROR_FROM_EX4\n");
        exit(1);
    }

    // write to to_srv file
//    printf("writing: %s\n", output);
    if (write(fdsrv, output, strlen(output)) < 0) {
//        perror("write: ");
        printf("ERROR_FROM_EX4\n");
        close(fdsrv);
        remove(tosrv);
        // and all other client files.
        exit(1);
    }

    if (close(fdsrv) < 0) {
//        perror("close: ");
        printf("ERROR_FROM_EX4\n");
        remove(tosrv);
        // and all other client files.
        exit(1);
    }

    // request service from server
    pid_t srvPid = atoi(argv[1]);
    if (kill(srvPid, SIGUSR1)) {
//        perror("kill: ");
        printf("ERROR_FROM_EX4\n");
        remove(tosrv);
        exit(1);
    }
    signal(SIGALRM, alarm_hand);
    signal(SIGUSR2, print_result);
    alarm(30);
    pause();
}