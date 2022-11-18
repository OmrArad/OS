// Omer Arad 314096389
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include<unistd.h>
#include<stdlib.h>

#define MAXLEN 100 // max length of command
#define MAXCOM 100 // max number of commands

static const char* history[MAXCOM];
static unsigned h_count = 0;

// Function to add current path upon shell initiation
void init(int argc, char* argv[])
{
    if (argc == 1) {
        return;
    }

    char* ppath;

    for (int i = 1; i < argc; i++) {
        ppath = getenv("PATH");
        strcat(ppath,":");
        strcat(ppath,argv[i]);
    }

    setenv("PATH", ppath, 1);

}

// Function to print shell line
void prompt()
{
    printf("$ ");
    fflush(stdout);
}

// Add command to history
void addHistory(int pid_t, char** parsed)
{
    int i;
    char cmd[MAXLEN];
    //int pid_t = getpid();
    snprintf(cmd,sizeof cmd, "%d", pid_t);

    for (i = 0; i < MAXLEN; i++) {
        if (parsed[i] == NULL) {
            break;
        }
        strcat(cmd, " ");
        strcat(cmd, parsed[i]);
    }

    history[h_count++] = strdup(cmd);
}

// Function to take input
int takeInput(char* str)
{
    char buf[MAXLEN];

    scanf(" %[^\n]s", buf);
    if (strlen(buf) != 0) {
        strcpy(str,buf);
        return 0;
    } else {
        return 1;
    }
}




// Function where the system command is executed
void execCommand(char** parsed)
{
    int status;
    // Forking a child
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork failed");
        return;
    } else if (pid == 0) {
        if (execvp(parsed[0],parsed) < 0) {
            char error[100];
            snprintf(error, sizeof error, "%s failed", parsed[0]);
            perror(error);
            exit(0);
        }
    } else {
        // wait for child to terminate
        wait(&status);
        addHistory(pid, parsed);
        return;
    }
}

// Function for executing a built-in command
int execBuiltIn(char** parsed)
{
    int numOfCmd = 3, func = 0;
    int pid_t = getpid();
    int i;
    char* listOfCmd[numOfCmd];

    listOfCmd[0] = "exit";
    listOfCmd[1] = "cd";
    listOfCmd[2] = "history";

    for (i = 0; i < numOfCmd; i++) {
        if (strcmp(parsed[0], listOfCmd[i]) == 0) {
            func += (i + 1);
            break;
        }
    }

    switch (func) {
        case 1:
            exit(0);
        case 2:
            addHistory(pid_t,parsed);
            chdir(parsed[1]);
            return 1;
        case 3:
            // history
            addHistory(pid_t,parsed);
            for (i = 0; i < h_count; i++) {
                printf("%s\n", history[i]);
            }
            return 1;
        default:
            break;
    }

    return 0;
}

// Function for parsing command
void parseCommand(char* str, char** parsed)
{
    int i;
    parsed[0] = strtok(str, " ");
    for (i = 1; i < MAXCOM; i++) {
        parsed[i] = strtok(NULL, " ");

        if (parsed[i] == NULL)
            break;
        if (strlen(parsed[i]) == 0)
            i--;

    }
}

// Function for processing given input
int processInput(char* str, char** parsed)
{
    parseCommand(str, parsed);

    if (execBuiltIn(parsed)) {
        return 0;
    } else {
        return 1;
    }
}

// Main function
int main(int argc, char* argv[])
{
    char inputString[MAXLEN], *command[MAXCOM];
    int execFlag = 0;
    int pid;
    init(argc, argv);

    while (1) {
        // print shell line
        prompt();
        // take input
        if (takeInput(inputString)) {
            continue;
        }

        // process input
        execFlag = processInput(inputString, command);

        // execute
        if (execFlag == 1) {
            execCommand(command);
        }
    }
    return 0;
}