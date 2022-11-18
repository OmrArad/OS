// Omer Arad 314096389
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>

char* err = "ERROR_FROM_EX4";
char* tosrv = "to_srv.txt";

// read the file from client
void readtosrv() {
    char input[100];
    char *clientID;
    char *firstArith;
    char *func;
    char *secondArith;
    char space[] = " ";
    int fdclient;
    char clientFileName[30] = "to_client_";


    int fdsrv = open(tosrv, O_RDWR, 0777);
    if (fdsrv < 0) {
        printf("ERROR_FROM_EX4\n");
//        perror("open tosrv: ");
        exit(1);
    }

    if (read(fdsrv, &input, 100) < 0) {
        printf("ERROR_FROM_EX4\n");
//        perror("read: ");
        close(fdsrv);
        exit(1);
    }

//    printf("read from to_srv: %s", input);

    clientID = strtok(input, space);
    firstArith = strtok(NULL, space);
    func = strtok(NULL, space);
    secondArith = strtok(NULL, space);

//    printf("clientID: %s\n", clientID);
    pid_t id = atoi(clientID);
//    printf("sending signal to: %d\n", id);

//    printf("firstArith: %s\n", firstArith);
    int num1 = atoi(firstArith);
//    printf("num1: %d\n", num1);

//    printf("func: %s\n", func);
//    printf("secondArith: %s\n", secondArith);


    // delete to_srv.
    if (close(fdsrv) < 0) {
        printf("ERROR_FROM_EX4\n");
//        perror("close: ");
        exit(1);
    }

    if (remove(tosrv) < 0) {
        printf("ERROR_FROM_EX4\n");
//        perror("remove: ");
        exit(1);
    }

    // create file to client
//    sprintf()
    strcat(clientFileName, clientID);
    strcat(clientFileName,".txt");

//    printf("creating file: %s\n", clientFileName);

    fdclient = open(clientFileName, O_RDWR | O_CREAT, 0777);
    if (fdclient < 0) {
        printf("ERROR_FROM_EX4\n");
//        perror("open client: ");
        exit(1);
    }

    // calculate result

    int num2 = atoi(secondArith);
    int function = atoi(func);
    int res;
    bool divO = false;

//    printf("num2: %d\n", num2);
//    printf("func: %d\n", function);

    switch (function) {
        case 1:
            res = num1 + num2;
            break;
        case 2:
            res = num1 - num2;
            break;
        case 3:
            res = num1 * num2;
            break;
        case 4:
            if (num2 == 0) {
                divO = true;
                break;
            }
            res = num1 / num2;
            break;
        default:
            printf("ERROR_FROM_EX4\n");
//            perror("not a function !");
            close(fdclient);
            remove(clientFileName);
            exit(1);
    }

//    printf("result: %d\n", res);

    // write in client file.
    int size = 100;
    char result[size];

    if(!divO) {
        sprintf(result, "%d", res);
    } else {
        sprintf(result, "%s", "CANNOT_DIVIDE_BY_ZERO");
    }

    if (write(fdclient, result, strlen(result)) < 0) {
        printf("ERROR_FROM_EX4\n");
//        perror("write: ");
        close(fdclient);
        remove(clientFileName);
        exit(1);
    }

    // let client know you're done.
    if (kill(id, SIGUSR2) < 0) {
        printf("ERROR_FROM_EX4\n");
//        perror("kill: ");
        close(fdclient);
        remove(clientFileName);
        exit(1);
    }

    // close fd.
    if (close(fdclient) < 0) {
        printf("ERROR_FROM_EX4\n");
//        perror("close: ");
        exit(1);
    }

    exit(0);
}

void srvclient(int signo) {
    int status;

    alarm(0);

    pid_t pid = fork();

    if (pid == -1) {
        printf("ERROR_FROM_EX4\n");
//        perror("fork: ");
        exit(1);
    } else if (pid == 0) {
        // child.
        readtosrv();
    } else {
        // return to service. wait for child at the end.
        // reset alarm and signal.
        alarm(60);
        signal(SIGUSR1, srvclient);
    }

}

void alarm_hand(int signo) {
    // close all fd.

    while(wait(NULL) != -1); //?
    printf("the server was closed because no service request "
           "was received for the last 60 seconds\n");

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

int main() {
    signal(SIGUSR1, srvclient);
    signal(SIGALRM, alarm_hand);

    // set alarm time.
    alarm(60);

    // del to_srv.
    if (access(tosrv, F_OK) == 0) {
        if (remove(tosrv) < 0) {
            printf("ERROR_FROM_EX4\n");
//            perror("remove: ");
            exit(1);
        }
    }

    while(1) {
        pause(); // waiting for client to send signal
    }

    while(wait(NULL) != 1);
    return 0;
}
