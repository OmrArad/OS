// Omer Arad 314096389
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ctype.h>

// close file
void closefd(int fd) {
    if (close(fd) < 0) {
        char error[20];
        snprintf(error, sizeof error, "Error in: close()");
        perror(error);
        exit(0);
    }
}

// read error
void readError(int fd1, int fd2) {
    char error[20];
    snprintf(error, sizeof error, "Error in: read()");
    perror(error);
    closefd(fd1);
    closefd(fd2);
    exit(0);
}

// open error
void openError() {
    char error[20];
    snprintf(error, sizeof error, "Error in: open()");
    perror(error);
}

// main function
int main(int argc, char* argv[])
{
    int fd1;    // first file descriptor
    int fd2;    // second file descriptor
    int x1;     // first file read variable
    int x2;     // second file read variable
    char ch1;   // first file char pointer
    char ch2;   // second file char pointer
    int ret = 1;    // return value can change to 3 if files are similar

    // check arg count
    if (argc != 3) {
        perror("argument number isn't valid");
        exit(0);
    }

    // open each file and check for errors
    if ((fd1 = open(argv[1], 'r')) < 0) {
        openError();
        exit(0);
    }
    if ((fd2 = open(argv[2], 'r')) < 0) {
        openError();
        exit(0);
    }

    // scan both files and check if they are identical, similar, different
    while (1) {
        if ((x1 = read(fd1, &ch1, 1)) < 0) {
            readError(fd1, fd2);
            exit(0);
        }
        if ((x2 = read(fd2, &ch2, 1)) < 0) {
            readError(fd1, fd2);
            exit(0);
        }
        if (x1 ==0 && x2 == 0) {
            // reached end of file and all chars are identical
            closefd(fd1);
            closefd(fd2);
            return ret;
        } else if (x1 == 0) {
            // only one file and reached the end, they are not identical
            while (ch2 == ' ' || ch2 == '\n') {
                if ((x2 = read(fd2, &ch2, 1)) < 0) {
                    readError(fd1, fd2);
                    exit(0);
                }
                if (x2 == 0) {
                    // second file has trailing spaces or new lines, they are similar
                    closefd(fd1);
                    closefd(fd2);
                    return 3;
                }
            }
            // found a char that isnt whitespace or new line, files are different
            closefd(fd1);
            closefd(fd2);
            return 2;
        } else if (x2 == 0) {
            // only one file and reached the end, they are not identical
            while (ch1 == ' ' || ch1 == '\n') {
                if ((x1 = read(fd1, &ch1, 1)) < 0) {
                    readError(fd1, fd2);
                    exit(0);
                }
                if (x1 == 0) {
                    // second file has trailing spaces or new lines, they are similar
                    closefd(fd1);
                    closefd(fd2);
                    return 3;
                }
            }
            // found a char that isnt whitespace or new line, files are different
            closefd(fd1);
            closefd(fd2);
            return 2;
        } 

        // skip whitespaces and end of lines, only if both chars are different
        if (ch1 != ch2) {
            ret = 3;
            while (ch1 == ' ' || ch1 == '\n') {
                if ((x1 = read(fd1, &ch1, 1)) < 0) {
                    readError(fd1, fd2);
                    exit(0);
                }
                if (x1 == 0) {
                    // only one file has reached the end, they are not identical
                    // will never enter because files will not end in whitespace
                    closefd(fd1);
                    closefd(fd2);
                    return 2;
                }
            }

            // skip whitespaces
            while (ch2 == ' ' || ch2 == '\n') {
                if ((x2 = read(fd2, &ch2, 1)) < 0) {
                    readError(fd1, fd2);
                    exit(0);
                }
                if (x2 == 0) {
                    // only one file and reached the end, they are not identical
                    // will never enter because files will not end in whitespace
                    closefd(fd1);
                    closefd(fd2);
                    return 2;
                }
            }
        }

        if ( ch1 != ch2) {
            if (tolower(ch1) != tolower(ch2)) {
                // found two chars that aren't identical
                closefd(fd1);
                closefd(fd2);
                return 2;
            } else {
                // files are similar
                ret = 3;
            }
        }
    }

}
