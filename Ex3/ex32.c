//Yahav Zarfati

#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/wait.h>
#include <time.h>

#define SIZE 151
#define ERROR -1
#define TIMEOUT 5
#define IDENTICAL 100
#define SIMILAR 75
#define DIFFERENT 50
#define SEGFAULT 256
#define OPEN_ERROR "Error in: open\n"
#define READ_ERROR "Error in: read\n"
#define OPEN_DIR_ERROR "Error in: opendir\n"
#define EXEC_ERROR "Error in: execvp\n"
#define FORK_ERROR "Error in: fork\n"
#define DUP2_ERROR "Error in: dup2\n"
#define DUP_ERROR "Error in: dup\n"

//Function attempt to open file in given path, if path is valid return 1, else return -1
int isValid(char* path) {
    int fd;
    fd = open(path, O_RDONLY);
    //Check if function success
    if(fd < 0 ) {
        return ERROR;
    }
    close(fd);
    return 1;
}

//Function add ./ to given path
void connectToStr(char* str) {
    char temp[SIZE];
    strcpy(temp, str);
    strcpy(str, "./");
    strcat(str, temp);
}

//Function return pointer to student name from given path
char* getStudentName(char* path) {
    char temp[SIZE] = "";
    strcpy(temp, path);
    char *ptrEnd = strrchr(temp, '/');
    char *ptrTempPath = strstr(temp, ptrEnd);
    strcpy(ptrTempPath, "\0");
    char *studentName = strrchr(temp, '/');
    studentName++;
    return studentName;
}

//Function check if given filename has .c extension
int isFileC(const char *filename) {
    const char *dot = strrchr(filename, '.');
    //If file has no extension
    if(!dot || dot == filename)
        return 0;
    //If its .c extension
    if(strcmp(dot, ".c") == 0) {
        return 1;
    }
    return 0;
}

//Function redirect old file discriptor to new file discriptor
int ioRedirect(const int* newFd, const int* oldFd) {
    //Check if function success
    if(dup2(*newFd, *oldFd) == ERROR) {
        return ERROR;
    }
    close(*newFd);
    return 1;
}

//The function compare current student output to correct output, returns his grade
int compareStudentToOutput(char* pathToOutput, const int* fdout) {
    int status;

    char *comp_args[] = {"./comp.out", pathToOutput, "./temp.txt", NULL};
    pid_t  proc1;
    proc1 = fork();
    //Check if function success
    if(proc1 < 0) {
        close(*fdout);
        if(write(STDOUT_FILENO, FORK_ERROR, strlen(FORK_ERROR)) == ERROR){}
        return ERROR;
    }
    //Compare files using child process
    else if (proc1 == 0) {
        //Check if function success
        if (execvp("./comp.out", comp_args) < 0) {
            close(*fdout);
            if(write(STDOUT_FILENO, EXEC_ERROR, strlen(EXEC_ERROR)) == ERROR){}
            return ERROR;
        }
    }
    waitpid(proc1, &status, 0);
    return WEXITSTATUS(status);
}

//The function calculate each student grade and writes into results his grade
void calculateGrade(char* pathToOutput, char* currentDir, int* fdout, int* grade) {
    //Only if student has a .out file call function
    if(*grade >= 50) {
        int result = compareStudentToOutput(pathToOutput, fdout);
        //If output of student file is identical to correct output
        if (result == 1) {
            *grade = 100;
            //If output of student file is different from correct output
        } else if (result == 2) {
            *grade = 50;
            //If output of student file is similar to correct output
        } else if (result == 3) {
            *grade = 75;
        } else {
            close(*fdout);
            return;
        }
    }

    //Write to buffer student name
    char resBuf[SIZE] = "";
    strcpy(resBuf, getStudentName(currentDir));
    strcat(resBuf, ",");

    //According to student grade writes his grade and suitable comment in results.csv
    switch (*grade) {
        case 10:
            strcat(resBuf, "10,COMPILATION_ERROR\n");
            break;
        case 20:
            strcat(resBuf, "20,TIMEOUT\n");
            break;
        case 50:
            strcat(resBuf, "50,WRONG\n");
            break;
        case 75:
            strcat(resBuf, "75,SIMILAR\n");
            break;
        case 100:
            strcat(resBuf, "100,EXCELLENT\n");
            break;
        default:
            strcat(resBuf, "-1,ERROR GRADE\n");
    }
    if(write(*fdout, resBuf, sizeof(resBuf)) == ERROR) {}
}

//Function compile students C file and executes it, call function to grade student
void compileAndRun(char* pathToInput, char* pathToOutput, char* currentDir, int* fdout, int* grade) {
    *grade = 10;
    int status;

    //Create errors.txt file
    int errorFd;
    errorFd = open("errors.txt", O_CREAT | O_APPEND | O_WRONLY, 0666);
    //Check if function success
    if(errorFd < 0 ) {
        close(*fdout);
        if(write(STDOUT_FILENO, OPEN_ERROR, strlen(OPEN_ERROR)) == ERROR){}
        exit(ERROR);
    }

    pid_t  proc1;
    proc1 = fork();
    //Check if function success
    if(proc1 < 0) {
        close(*fdout);
        close(errorFd);
        if(write(STDOUT_FILENO, FORK_ERROR, strlen(FORK_ERROR)) == ERROR){}
        return;
    }

    //Compile file on son process
    else if (proc1 == 0) {
        //Redirect stderr to errors.txt file
        int stdErr = 2;
        //Check if function success
        if(ioRedirect(&errorFd, &stdErr) == ERROR) {
            close(*fdout);
            close(errorFd);
            if(write(STDOUT_FILENO, DUP2_ERROR, strlen(DUP2_ERROR)) == ERROR){}
            return;
        }

        char *gcc_args[] = {"gcc", "-o", "temp.out", currentDir, NULL};
        //Check if function success
        if(execvp("gcc", gcc_args) < 0) {
            close(*fdout);
            close(errorFd);
            if(write(STDOUT_FILENO, EXEC_ERROR, strlen(EXEC_ERROR)) == ERROR){}
            return;
        }
    }
    waitpid(proc1, &status, 0);

    //Open input file
    int inputFd;
    inputFd = open(pathToInput, O_RDONLY);
    //Check if function success
    if(inputFd < 0 ) {
        close(*fdout);
        close(errorFd);
        if(write(STDOUT_FILENO, OPEN_ERROR, strlen(OPEN_ERROR)) == ERROR){}
        return;
    }

    int tempFd;
    tempFd = open("temp.txt",  O_RDWR | O_CREAT, 0666);
    //Check if function success
    if(tempFd < 0 ) {
        close(*fdout);
        close(errorFd);
        close(inputFd);
        if(write(STDOUT_FILENO, OPEN_ERROR, strlen(OPEN_ERROR)) == ERROR){}
        return;
    }
    //Execute .out file if compiled successfully
    if(status != SEGFAULT) {
        *grade = 50;
        //Start timing for student .out file
        time_t start, end;
        start = time(NULL);

        pid_t proc2;
        proc2 = fork();
        //Check if function success
        if (proc2 < 0) {
            close(*fdout);
            close(errorFd);
            close(inputFd);
            close(tempFd);
            if(write(STDOUT_FILENO, FORK_ERROR, strlen(FORK_ERROR)) == ERROR){}
            return;
        }

            //Run file on son process
        else if (proc2 == 0) {
            //Save old stdout
            int oldStdOut = dup(STDOUT_FILENO);
            //Check if function success
            if (oldStdOut == ERROR) {
                close(*fdout);
                close(errorFd);
                close(inputFd);
                close(tempFd);
                if(write(STDOUT_FILENO, DUP_ERROR, strlen(DUP_ERROR)) == ERROR){}
                return;
            }

            //Redirect stdin to input file
            int stdIn = 0;
            //Check if function success
            if(ioRedirect(&inputFd, &stdIn) == ERROR) {
                close(*fdout);
                close(errorFd);
                close(inputFd);
                close(tempFd);
                if(write(STDOUT_FILENO, DUP2_ERROR, strlen(DUP2_ERROR)) == ERROR){}
                return;
            }

            //Redirect stdout to temp.txt file
            int stdOut = 1;
            //Check if function success
            if(ioRedirect(&tempFd, &stdOut) == ERROR) {
                close(*fdout);
                close(errorFd);
                close(inputFd);
                close(tempFd);
                if(write(STDOUT_FILENO, DUP2_ERROR, strlen(DUP2_ERROR)) == ERROR){}
                return;
            }
            char *temp_args[] = {"./temp.out", NULL};

            //Check if function success
            if (execvp("./temp.out", temp_args) < 0) {
                //Redirect back to old stdout to write error and check if function success
                if(ioRedirect(&oldStdOut, &stdOut) == ERROR) {
                    close(*fdout);
                    close(errorFd);
                    close(inputFd);
                    close(tempFd);
                    if(write(STDOUT_FILENO, DUP2_ERROR, strlen(DUP2_ERROR)) == ERROR){}
                    return;
                }
                close(*fdout);
                close(errorFd);
                close(inputFd);
                close(tempFd);
                if(write(STDOUT_FILENO, EXEC_ERROR, strlen(EXEC_ERROR)) == ERROR){}
                return;
            }
        }
        waitpid(proc2, &status, 0);

        //Measure end timeing of students .out file
        end = time(NULL);
        //Check if execution time is bigger than TIMEOUT
        if (end - start > TIMEOUT) {
            *grade = 20;
        }
    }
    //Close unnecessary fd's
    close(errorFd);
    close(inputFd);
    close(tempFd);

    calculateGrade(pathToOutput, currentDir, fdout, grade);
    //Remove excess files
    remove("temp.out");
    remove("temp.txt");

}

//Function search in each student his C file, compiles it, run it - to be graded
void execute(char *basePath, char* pathToInput, char* pathToOutput, int* fdout) {
    struct dirent *dp1;
    DIR *dir1 = opendir(basePath);
    //Check if function success
    if(dir1 == NULL) {
        close(*fdout);
        if(write(STDOUT_FILENO, OPEN_DIR_ERROR, strlen(OPEN_DIR_ERROR)) == ERROR){}
        exit(ERROR);
    }

    //Run on each student directory
    while((dp1 = readdir(dir1)) != NULL) {
        int grade = 0;
        int noCFile = 1;
        //Ignore . and .. entries
        if (strcmp(dp1->d_name, ".") != 0 && strcmp(dp1->d_name, "..") != 0) {
            //Prep path of each student file for opendir
            char tempPath[SIZE];
            strcpy(tempPath, basePath);
            strcat(tempPath, "/");
            strcat(tempPath, dp1->d_name);

            struct dirent *dp2;
            DIR *dir2 = opendir(tempPath);

            //Check if function success
            if(dir2 == NULL) {
                close(*fdout);
                if(write(STDOUT_FILENO, OPEN_DIR_ERROR, strlen(OPEN_DIR_ERROR)) == ERROR){}
                exit(ERROR);
            }

            //Run on each file in student directory
            while((dp2 = readdir(dir2)) != NULL) {
                //Ignore . and .. entries
                if (strcmp(dp2->d_name, ".") != 0 && strcmp(dp2->d_name, "..") != 0) {
                    //Check if its the correct file and not directory
                    if (isFileC(dp2->d_name) && dp2->d_type != DT_DIR) {
                        noCFile = 0;
                        //Construct to the C file
                        char path[SIZE];
                        strcpy(path, tempPath);
                        strcat(path, "/");
                        strcat(path, dp2->d_name);
                        compileAndRun(pathToInput, pathToOutput, path, fdout, &grade);
                        break;
                    }
                }
            }
            //If student doesn't have C file, write his grade in csv
            if(noCFile) {
                char *ptrEnd = strrchr(tempPath, '/');
                char *ptrTempPath = strstr(tempPath, ptrEnd);
                ptrTempPath++;
                char noCFileBuf[SIZE] = "";
                strcpy(noCFileBuf, ptrTempPath);
                strcat(noCFileBuf, ",");
                strcat(noCFileBuf, "0,NO_C_FILE\n");
                if(write(*fdout, noCFileBuf, sizeof(noCFileBuf)) == ERROR){}
            }
            closedir(dir2);
        }
    }
    closedir(dir1);
}

//Function parse config files, each path into array
void parseConfig(const int* fdin, char* pathToFiles, char* pathToInput, char* pathToOutput){
    char configBuf[500];
    ssize_t charsr;
    charsr = read(*fdin, configBuf, sizeof(configBuf));
    //Check if function success
    if(charsr < 0) {
        if(write(STDOUT_FILENO, READ_ERROR, strlen(READ_ERROR)) == ERROR){}
        close(*fdin);
        exit(ERROR);
    }
    int i = 0;
    //Get path to files into array
    while(configBuf[i] != '\n') {
        pathToFiles[i] = configBuf[i];
        i++;
    }
    i++;
    int j = 0;
    //Get path to input into array
    while(configBuf[i] != '\n') {
        pathToInput[j] = configBuf[i];
        i++;
        j++;
    }
    i++;
    j = 0;
    //Get path to output into array
    while(configBuf[i] != '\n') {
        pathToOutput[j] = configBuf[i];
        i++;
        j++;
    }
    close(*fdin);
    connectToStr(pathToFiles);
    connectToStr(pathToInput);
    connectToStr(pathToOutput);

    //Check each path if its valid
    if(isValid(pathToFiles) == ERROR) {
        char invalidDir[] = {"Not a valid directory\n"};
        if(write(STDOUT_FILENO, invalidDir, strlen(invalidDir)) == ERROR){}
        exit(ERROR);
    }

    if(isValid(pathToInput) == ERROR) {
        char invalidInput[] = {"Input file not exist\n"};
        if(write(STDOUT_FILENO, invalidInput, strlen(invalidInput)) == ERROR){}
        exit(ERROR);
    }

    if(isValid(pathToOutput) == ERROR) {
        char invalidOutput[] = {"Output file not exist\n"};
        if(write(STDOUT_FILENO, invalidOutput, strlen(invalidOutput)) == ERROR){}
        exit(ERROR);
    }
}

int main(int argc, char* argv[]) {
    char pathToConf[SIZE] = "";
    strcpy(pathToConf, "./");
    strcat(pathToConf, argv[1]);

    //Open config file
    int fdin;
    fdin = open(pathToConf, O_RDONLY);
    //Check if function success
    if(fdin < 0) {
        if(write(STDOUT_FILENO, OPEN_ERROR, strlen(OPEN_ERROR)) == ERROR){}
        exit(ERROR);
    }
    char pathToFiles[SIZE] = "";
    char pathToInput[SIZE] = "";
    char pathToOutput[SIZE] = "";
    parseConfig(&fdin, pathToFiles, pathToInput, pathToOutput);

    //Create csv file
    int fdout;
    fdout = open("results.csv", O_CREAT | O_APPEND | O_RDWR, 0666);
    //Check if function success
    if(fdout < 0) {
        close(fdin);
        if(write(STDOUT_FILENO, OPEN_ERROR, strlen(OPEN_ERROR)) == ERROR){}
        exit(ERROR);
    }

    execute(pathToFiles, pathToInput, pathToOutput, &fdout);
    close(fdout);

}
