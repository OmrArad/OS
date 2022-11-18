// Omer Arad 314096389
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ctype.h>
#include <dirent.h>
#include <stdbool.h>

// close file
void closeFd(int fd) {
    if (close(fd) < 0) {
        perror("Error in: close()");
    }
}

// open error
void openError() {
    perror("Error in: open()");
}

void printError(char* func) {
    char error[20];
    snprintf(error, sizeof error, "Error in: %s()", func);
    perror(error);
}

void changeDir(char* path) {
    if (chdir(path) < 0) {
        printError("chdir");
    }
}

// Function to add given path
void addPath(char* path)
{
    char* ppath;
    ppath = getenv("PATH");
    strcat(ppath,":");
    strcat(ppath,path);
    setenv("PATH", ppath, 1);

}


// Function where the system command is executed
void execCommand(char** parsed)
{
    int status;
    // Forking a child
    pid_t pid = fork();

    if (pid == -1) {
        printError("fork");
        return;
    } else if (pid == 0) {
        if (execvp(parsed[0],parsed) < 0) {
            printError(parsed[0]);
            exit(-1);
        }
    } else {
        // wait for child to terminate
        wait(&status);
        return;
    }
}

// Function where the system command is executed
int execProg(char** parsed)
{
    int status;
    // Forking a child
    pid_t pid = fork();

    if (pid == -1) {
        printError("fork");
        exit(-1);
    } else if (pid == 0) {
        if (execv(parsed[0],parsed) < 0) {
            printError(parsed[0]);
            exit(-1);
        }
        return 0;
    } else {
        // wait for child to terminate
        wait(&status);
        return WEXITSTATUS(status);
    }
}

// open directory and return a pointer
DIR* parseConf(char* line, char const* error) {
    DIR* dip;
    char* l = line;

    if ((dip = opendir(l)) == NULL) {
        if (write(1, error, strlen(error)) < 0) {
            // write failed
        }
        exit(-1);
    }
    return dip;
}

// get file descriptor for given path(line)
int getFd(char* line, char const* error) {
    int retFd;
    char* l = line;
//    printf("opening: %s\n", l);
    if ((retFd = open(l, O_RDONLY)) < 0) {
        if (write(1, error, strlen(error)) < 0) {
            // write failed
        }
        openError();
    }
    return retFd;
}

// create an absolute path to given path
void createAbsolut(char* mainDir, char* path, char* absolut) {
    strcpy(absolut, mainDir);
    strcat(absolut, "/");
    strcat(absolut, path);
}

//int execGcc(char* full) {
//
//}
//
//int runWithInput() {
//
//}

// write to results.csv
void writeResult(int res, char* path, int resFd) {
    int grade;
    char* reason;
    char result[150];
    switch (res) {
        case 1:
            grade = 100;
            reason = ",100,EXCELLENT\n";
            break;
        case 2:
            grade = 50;
            reason = ",50,WRONG\n";
            break;
        case 3:
            grade = 75;
            reason = ",75,SIMILAR\n";
            break;
        case 4:
            reason = ",10,COMPILATION_ERROR\n";
            break;
        case 5:
            reason = ",0,NO_C_FILE\n";
            break;
        default :
            break;
    }

    strcpy(result, path);
    strcat(result, reason);
    if ((write(resFd,result, strlen(result))) < 0) {
        // write failed
    }
}

// remove given file
void rmFile(char* str) {
    if (remove(str) < 0) {
        printError("remove");
    }
}
//
//int execComp(char* cwd, char* output, char* mainDir, char* l3, char* l1, char* path, int resFd) {
//    createAbsolut(cwd, "output.txt", output);
//    chdir(mainDir);
//    char* comp[] = {"./comp.out",  l3, output, NULL};
//    int retVal = execProg(comp);
//    chdir(l1);
//    chdir(path);
//    rmFile("output.txt");
//
//    // write result to csv file
//    writeResult(retVal, path, resFd);
//}


// main function
int main(int argc, char* argv[])
{
    int size = 150;
    DIR* dip;               // folder descriptor for first folder of conf file
    struct dirent *dit;     // dirent structure
    char conf[size * 3];           // full conf file
    char const nl[2] = "\n";   // new line
    char* l1;               // first line in conf file
    char* l2;               // second line in conf file
    char* l3;               // third line in conf file
    char absolutL2[size];
    char mainDir[size];
    char const* argsErr = "argument number isn't valid\n";
    char const* errorL1 = "Not a valid directory\n";
    char const* errorL2 = "Input file not exist\n";
    char const* errorL3 = "Output file not exist\n";

    int fd;    // first file descriptor
    int fdInput;
    int fdCorrectOutput;
    int resFd;
    int compError = 4;
    int noCFile = 5;
    bool foundCFile;

    // set errors to print in errors.txt
    int fdError = open("errors.txt", O_CREAT | O_WRONLY | O_APPEND | O_TRUNC);
    if (fdError < 0) {
        openError();
        exit(-1);
    }
    if ((chmod("errors.txt", S_IRWXU)) < 0) {
        printError("chmod");
        exit(-1);
    }
    if ((dup2(fdError, STDERR_FILENO)) < 0 ) {
        printError("dup2");
        exit(-1);
    }
    closeFd(fdError);

    // check arg count
    if (argc != 2) {
        perror(argsErr);
        exit(-1);
    }

    // open conf file
    if ((fd = open(argv[1], 'r')) < 0) {
        openError();
        exit(-1);
    }

    // read from conf file
    if ((read(fd, &conf, 450)) < 0) {
        printError("read");
        exit(-1);
    }

    // get fd for students folder
    l1 = strtok(conf, nl);
    dip = parseConf(l1, errorL1);

    // get fd for input file
    l2 = strtok(NULL, nl);
    if ((fdInput = getFd(l2, errorL2)) < 0) {
        exit(-1);
    }

    // get fd for correct output file
    l3 = strtok(NULL, nl);
    if ((fdCorrectOutput = getFd(l3, errorL3)) < 0) {
        exit(-1);
    }
    closeFd(fd);

    // create result file
    if ((resFd = open("results.csv", O_CREAT | O_WRONLY | O_APPEND)) < 0) {
        openError();
        exit(-1);
    }
    if ((chmod("results.csv", S_IRWXU)) < 0) {
        printError("chmod");
        exit(-1);
    }

    // save program working dir, and input output working dirs
    if (getcwd(mainDir, size) == NULL) {
        printError("getcwd");
        exit(-1);
    }
    if (strncmp(mainDir, l2, 2) != 0) {
        createAbsolut(mainDir, l2, absolutL2);
    } else {
        strcpy(absolutL2, l2);
    }

    addPath(mainDir);
    if(chdir(l1) < 0) {
        printError("chdir");
        exit(-1);
    }

    // for each directory in given dir do the following:
    while ((dit = readdir(dip)) != NULL)
    {
        DIR* dipSub;
        struct dirent *ditSub;
        char path[size];

        if (strcmp(dit->d_name, ".") == 0 || strcmp(dit->d_name, "..") == 0) {
            continue;
        }

        strcpy(path,dit->d_name);

        // if path is not a directory continue to next one
        if ((dipSub = opendir(path)) == NULL) {
            continue;
        }

        // set flag of existing c file to false
        foundCFile = false;

        // look for c file in each subdirectory
        while ((ditSub = readdir(dipSub)) != NULL) {
            // skip . and .. folders
            if (strcmp(ditSub->d_name, ".") == 0 || strcmp(ditSub->d_name, "..") == 0 || ditSub->d_type != DT_REG) {
                continue;
            }

            // check if a file ending with .c exists
            char* token1 = strtok(ditSub->d_name, ".");
            char lastToken[20];
            char full[size];
            strcpy(full,token1);
            while (token1 != NULL) {
                strcpy(lastToken,token1);
                token1 = strtok(NULL, ".");
                if(token1 != NULL) {
                    strcat(full, ".");
                    strcat(full, token1);
                }
            }

            if (strcmp(lastToken, "c") == 0) {
                foundCFile = true;

                // call gcc using execvp
                if (chdir(path) < 0) {
                    printError("chdir");
                    continue;
                }
                char cwd[size];
                char error[size];
                char* outputName = "output.txt";
                // check
                if ((getcwd(cwd, size)) == NULL) {
                    printError("getcwd");
                    continue;
                }

                char* gcc[] = {"gcc", full, "-o", "main.out", NULL};
                execCommand(gcc);
                // check for compilation errors
                if (access("main.out", F_OK) != 0) {
                    writeResult(compError, path, resFd);
                    chdir("..");
                    continue;
                }

                // run with input
                if ((fdInput = getFd(absolutL2, errorL2)) < 0) {
                    rmFile("main.out");
                    exit(-1);
                }
                // check
                if ((dup2(fdInput,0)) < 0) {
                    printError("dup2");
                    closeFd(fdInput);
                    rmFile("main.out");
                    continue;
                }
                closeFd(fdInput);

                // write to outputFile.txt
                // check
                int fdOutput;
                char outputFile[size];
                createAbsolut(cwd, outputName, outputFile);
                if ((fdOutput = open(outputName, O_CREAT | O_RDWR | O_TRUNC)) < 0) {
                    openError();
                    rmFile("main.out");
                    continue;
                }

                int savedStdout;
                if ((savedStdout = dup(1)) < 0) {
                    printError("dup");
                    continue;
                }

                if ((dup2(fdOutput,1)) < 0) {
                    printError("dup2");
                    rmFile("main.out");
                    rmFile(outputFile);
                    closeFd(fdOutput);
                    closeFd(savedStdout);
                    continue;
                }
                // check
                if ((chmod(outputName, S_IRWXU)) < 0) {
                    printError("chmod");
                    rmFile("main.out");
                    rmFile(outputFile);
                    closeFd(fdOutput);
                    closeFd(savedStdout);
                    continue;
                }

                char* runMain[] = {"./main.out",   NULL};
                if (execProg(runMain) < 0){
                    rmFile("main.out");
                    rmFile(outputFile);
                    closeFd(fdOutput);
                    closeFd(savedStdout);
                    continue;
                }
                closeFd(fdOutput);

                if (remove("main.out") < 0) {
                    printError("remove");
                    closeFd(savedStdout);
                    rmFile(outputFile);
                    continue;
                }

                if ((dup2(savedStdout,1)) < 0) {
                    printError("dup2");
                    closeFd(savedStdout);
                    rmFile(outputFile);
                    continue;
                }
                closeFd(savedStdout);

                // comp with correct outputFile
                if ((chdir(mainDir)) < 0) {
                    printError("chdir");
                    rmFile(outputFile);
                    continue;
                }

                int retVal;     // return value from comp
                char* comp[] = {"./comp.out", l3, outputFile, NULL};
                retVal = execProg(comp);
                if (retVal < 0) {
                    rmFile(outputFile);
                    changeDir(l1);
                    continue;
                }

                if (remove(outputFile) < 0) {
                    printError("remove");
                    changeDir(l1);
                    continue;
                }
                // write result to csv file
                writeResult(retVal, path, resFd);

                if ((chdir(l1)) < 0) {
                    printError("chdir");
                    continue;
                }
            }
        }

        // write to result if no c file exists
        if (!foundCFile) {
            writeResult(noCFile, path, resFd);
        }
    }

    closeFd(resFd);

    // close directory
    if (closedir(dip) < 0) {
        perror("Error in: closedir()");
        exit(-1);
    }
}
