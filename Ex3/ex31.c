//Yahav Zarfati

#include <stdio.h>
#include <sys/fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define SIZE 150
#define IDENTICAL 1
#define DIFFERENT 2
#define SIMILAR 3
#define ERROR -1

//Function prepare file discriptors for program
void openPaths(int* fdin1, int* fdin2, char* path1, char* path2){
    char openError[] = "Error in: open\n";
    *fdin1 = open(path1, O_RDONLY);
    //Check if function success
    if(*fdin1 < 0) {
        write(STDOUT_FILENO, openError, sizeof(openError));
        exit(ERROR);
    }
    *fdin2 = open(path2, O_RDONLY);
    //Check if function success
    if(*fdin2 < 0) {
        write(STDOUT_FILENO, openError, sizeof(openError));
        close(*fdin1);
        exit(ERROR);
    }
}

//Function check if rest of the string has space or enter, return true else, return false
int checkRestString(const char* buf, int* index)
{
    //Iterate on buffer
    while(buf[*index] != '\0') {
        //Check if char is space or enter
        if(buf[*index] == ' ' || buf[*index] == '\n') {
            *index += 1;
            continue;
        }
        return 1;
    }
    return 0;
}

int main(int argc, char* argv[]) {
    int switchCaseDiff = 0;
    int spaceEnterDiff = 0;
    int fdin1;
    int fdin2;
    ssize_t charsr1;
    ssize_t charsr2;
    char buf1[SIZE + 1] = "";
    char buf2[SIZE + 1] = "";
    openPaths(&fdin1, &fdin2, argv[1], argv[2]);
    int i = 0, j = 0;
    char readError[] = "Error in: read\n";
    do {
        //Check if function success
        charsr1 = read(fdin1, buf1, sizeof(buf1));
        if(charsr1 < 0) {
            write(STDOUT_FILENO, readError, sizeof(readError));
            close(fdin1);
            close(fdin2);
            exit(ERROR);
        }
        charsr2 = read(fdin2, buf2, sizeof(buf2));
        //Check if function success
        if(charsr2 < 0) {
            write(STDOUT_FILENO, readError, sizeof(readError));
            close(fdin1);
            close(fdin2);
            exit(ERROR);
        }
        //Check if each byte for difference
        while(buf1[i] != '\0' && buf2[j] != '\0') {
            //Check if byte is different
            if(buf1[i] != buf2[j]) {
                //Check if difference is space or enter
                if(buf1[i] == ' ' || buf1[i] == '\n'){
                    spaceEnterDiff = 1;
                    i++;
                    continue;
                }
                //Check if difference is space or enter
                if(buf2[j] == ' ' || buf2[j] == '\n'){
                    spaceEnterDiff = 1;
                    j++;
                    continue;
                }
                //Check if difference is in switch case
                if(abs(buf1[i] - buf2[j]) != 32) {
                    close(fdin1);
                    close(fdin2);
                    return DIFFERENT;
                }
                switchCaseDiff = 1;
            }
            i++;
            j++;
        }
        //Handle case of different length strings
        if(strlen(buf1) != strlen(buf2)) {
            //Check which string need to scan
            if (buf1[i] == '\0') {
                //Unique case of enter end of file - 2 bytes
                if(buf1[j - 1] == '\n') {
                    j++;
                }
                //Check if rest of the string is not space or enter
                if (checkRestString(buf2, &j)) {
                    close(fdin1);
                    close(fdin2);
                    return DIFFERENT;
                }
                switchCaseDiff = 1;
                spaceEnterDiff = 1;
            } else if (buf2[j] == '\0') {
                //Unique case of enter end of file - 2 bytes
                if(buf1[i - 1] == '\n') {
                    i++;
                }
                //Check if rest of the string is not space or enter
                if (checkRestString(buf1, &i)) {
                    close(fdin1);
                    close(fdin2);
                    return DIFFERENT;
                }
                switchCaseDiff = 1;
                spaceEnterDiff = 1;
            }
        }
    }while(charsr1 == SIZE && charsr2 == SIZE);
    close(fdin1);
    close(fdin2);
    //If files are similar
    if(switchCaseDiff == 1 || spaceEnterDiff == 1) {
        return SIMILAR;
    }
    //If files are identical
    return IDENTICAL;

}
