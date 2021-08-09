#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#define MAX 100

//Maintain global array of commands history and their pid's, also count for order
int count = 0;
char history[MAX][MAX];
pid_t pids[MAX];
//For cd -
//char cwd[MAX], temp[MAX];
//char* oldPath = NULL;
char oldPath[MAX];

void parse(char** words, char* input, int* background, int* numOfWords) {
    int i = 0;
    //Clear words array
    memset(words, 0, MAX * (sizeof(words[0])));

    //Copy input to history, and remove & from history
    strcpy(history[count], input);

    //Disassemble user input into words, put words into array
    char *token = strtok(input, " ");
    while (token != NULL) {
        words[i] = token;
        token = strtok(NULL, " ");
        i++;
    }
    //Check for echo
    char* pos;
    if(strcmp("echo", words[0]) == 0) {
		int j;
        for(j = 2; j < i; j++) {
            pos = strchr(words[j], '"');
            *pos = '\0';
        }
    }
    *numOfWords = i;
    //Remove \n from history
    int lenHistory = strlen(history[count]) - 1;
    char* ptr = history[count] + lenHistory;
    *ptr = '\0';

    //Fix for exec func
    int lastWord = i - 1;
    int length = strlen(words[lastWord]);
    words[lastWord][length - 1] = '\0';

    //Check if run process in foreground
    if(strcmp(words[lastWord], "&") == 0) {
        *background = 1;
        words[lastWord] = '\0';     //Fix for exec func with &
        //Remove & from history
        lenHistory = strlen(history[count]) - 2;
        ptr = history[count] + lenHistory;
        *ptr = '\0';
    }

    //todo make echo func, replace " with ' memove command, take first word
    count++;
}

void jobs() {
    int i = 0;
    //int status;
    while(i < count - 1) {
        if(waitpid(pids[i], NULL, WNOHANG) == 0) {
            printf("%s\n", history[i]);
            fflush(stdout);
        }
        i++;
    }
}

void historyFunc() {
    int i = 0;
    //Iterate in history array
    while (i < count) {
        //If its last command print running
        if (i == count - 1) {
            printf("history RUNNING\n");
            break;
        }
        //Check if process is finished
        if (waitpid(pids[i], NULL, WNOHANG) == 0) {
            printf("%s RUNNING\n", history[i]);
        } else {
            printf("%s DONE\n", history[i]);
        }
        fflush(stdout);
        i++;
    }
}

int changeDirectory(char* words[], int numOfWords) {
    //char* oldPathTemp = getcwd(temp, sizeof(temp));
    char buffer[sizeof(oldPath)];
    getcwd(buffer, sizeof(buffer));
    //Check number of arguments
    if (numOfWords > 2) {
        printf("Too many arguments\n");
        return -1;
    }
    //Check flag ~
    if (numOfWords == 1 || strcmp(words[1], "~") == 0 || strcmp(words[1], "~/") == 0) {    //todo ~/ goto home maybe in if || words[1] == NULL
        chdir(getenv("HOME"));
        strcpy(oldPath, buffer);
        return 1;       //todo ~/ PATH case next if, inorder to go home use getenvHOME then strcat given PATH
    //Check flag -
    } else if (strcmp(words[1], "-") == 0) {
        if(oldPath == NULL) {
            printf("chdir failed\n");
            fflush(stdout);
            return -1;
        }
        if(chdir(oldPath) == -1){
            printf("chdir failed\n");
            fflush(stdout);
            return -1;
        }
        strcpy(oldPath, buffer);
        return 1;
    }
    if (chdir(words[1]) == -1) {
        printf("chdir failed\n");
        return -1;
    }
}

void runBash(char** words, int background) {
    pid_t pid;
    pid = fork();
    pids[count - 1] = pid;
    int status;
    //Check if its child process
    if(pid == 0) {
        //Check if exec failed
        if(execvp(words[0], words) == -1) {
            printf("exec failed\n");
            fflush(stdout);
        }
        exit(0);
    //Check if fork failed
    } else if(pid < 0) {
        printf("fork failed\n");
        fflush(stdout);
    //Check if its parent process
    } else {
        //Check if process requested to run foreground
        if(background != 1) {
            waitpid(pid, &status, WUNTRACED);
        }
    }
}

int execute(char** words, int background, int numOfWords) {
    //Check if its built-in command or run linux bash
    if(strcmp(words[0], "jobs") == 0) {
        jobs();
    } else if(strcmp(words[0], "history") == 0) {
        historyFunc();
    } else if(strcmp(words[0], "cd") == 0) {
        changeDirectory(words, numOfWords);
    } else if(strcmp(words[0], "exit") == 0) {
        return 1;
    } else {
        runBash(words, background);
    }
    return 0;
}

int main() {
    int stop = 0;
    int background = 0;
    int numOfWords = 0;
    char input[MAX] = "";
    char* words[MAX] = { NULL };
    //Run linux bash until user request
    while(stop != 1) {
        printf("$ ");
        fflush(stdout);
        //Parse user input
        if(fgets(input, MAX, stdin) != NULL) {
            parse(words, input, &background, &numOfWords);
        } else {
            printf("fgets error\n");
            fflush(stdout);
        }
        stop = execute(words, background, numOfWords);
    }
}
