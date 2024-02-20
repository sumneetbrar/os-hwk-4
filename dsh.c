/*
 * dsh.c
 *
 *  Created on: Feb 19, 2024
 *      Author: Sumneet
 */
#include "dsh.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <sys/types.h>
#include <errno.h>
#include <err.h>
#include <sys/stat.h>
#include <string.h>


/**
 * Run the shell
 * 
 * Todo: 
 * 1) check correctness of changeDirectory
 * 2) finish implementing mode 1
 * 3) finish implementing mode 2
*/
void shell(void) {
    char *line = (char*) malloc(MAXBUF); // create an empty buffer to store the input
    char *list[MAXBUF / 2 + 1]; // array for command and arguments

    while(1) {
        printf("dsh> ");  // prompt user
        fflush(stdout);   // Output does not print until stream is manually flushed?

        if (fgets(line, MAXBUF, stdin) == NULL) {
            printf("Exiting shell...\n");
            exit(0);  // exit the program if EOF is input
        }

        char *trimmedLine = trim(line);
        if (*trimmedLine == '\0') {
            continue;  // if empty, skip this iteration
        }
        // alternative - does this work the same?
        if (strlen(trimmedLine) == 0) {
            continue;
        }

        // Look at the input and decide what to run
        if (*trimmedLine == '/') {
            commandMode1(line, list);
        }
        else if (strcmp(trimmedLine, "exit") == 0) {
            printf("Exiting shell...\n");
            free(line);
            exit(0);
        }
        else if (strncmp(trimmedLine, "cd", 3) == 0) {
            char *path = trimmedLine + 3;
            changeDirectory(path);
        }
        else if (strcmp(trimmedLine, "pwd") == 0) {
            char cwd[MAXBUF];
            getcwd(cwd, sizeof(cwd));
            printf("%s\n", cwd);
        }
        else {
            commandMode2(line, list);
        }
    }
}

/*
 * Mode 1 - we were given the full path
*/
void commandMode1(char *line, char **list) {
    if (access(line, F_OK | X_OK) == 0) {
        // File exists and is executable! Can run!

        // run foreground or background?
        int length = strlen(line);
        if (line[length - 1] == '&') {
            line[length - 1] = '\0'; // remove the &
            runBackground(line, list);
        }
        else {
            runForeground(line, list);
        }
    }
    else {
        // No good! File doesn't exist or is not executable!
        printf("Not a valid path\n");
    }
}


/**
 * Run in background
*/
void runBackground(char *line, char **list) {
    printf("I'm running in the background");
}


/**
 * Run in foreground
*/
void runForeground(char *line, char **list) {
    int i = 0;
    list[i] = strtok(line, " ");
    while (list[i] != NULL && i < MAXBUF / 2) {
        list[++i] = strtok(NULL, " ");
    }

    pid_t pid = fork();

    // both child and parent process are active after printing. 
    if (pid == 0) {  // child process
        printf("Child process!\n");
        exit(0); // with this addition, parent runs first?
    }
    else {   // parent
        printf("Parent process!\n");
        wait(NULL);
    }
}


/*
 * Mode 2 - we were not given the full path
*/
void commandMode2(char *line, char **list) {
    printf("Mode 2 activated.");
}

/**
 * Function to trim the input string
*/
char *trim(char *str) {
    char *start = str; // point to the first char
    char *end; // pointer to last char

    // check for preceding white space
    while (*start == ' ' || *start == '\t' || *start == '\n') {
        start++;
    }

    // if trimmed input is empty, reprompt user
    if (*start == 0) return start;

    end = (strlen(start) - 1) + start; // calculate end

    // check for trailing whitespace
    while (*end == ' ' || *end == '\t' || *end == '\n') {
        end--;
    }

    // terminate string
    *(end + 1) = '\0';

    return start;
}


/**
 * Function to change the directory
*/
void changeDirectory(char *path) {
    chdir(path);
}
