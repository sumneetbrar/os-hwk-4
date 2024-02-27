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
*/
void shell(void) {
    char *line = (char*) malloc(MAXBUF); // create an empty buffer to store the input
    char *list[MAXBUF / 2]; // array for command and arguments
    int run = 0; // default should be run in foreground

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

        // Look at the input and decide what to run
        if (*trimmedLine == '/') {
            int length = strlen(trimmedLine);
            if (trimmedLine[length - 1] == '&') {
                trimmedLine[length - 1] = '\0';
                trimmedLine = trim(trimmedLine);
                run = 1;
            }
            else run = 0;
            commandMode1(trimmedLine, list, run);
        }
        else if (strcmp(trimmedLine, "exit") == 0) {
            printf("Exiting shell...\n");
            free(line);
            exit(0);
        }
        else if (strncmp(trimmedLine, "cd", 2) == 0) {
            if (strlen(trimmedLine) == 2) {
                char *path = " ";
                changeDirectory(path, 1);
            }
            else {
                char *path = trimmedLine + 3;
                changeDirectory(path, 0);
            }
        }
        else if (strcmp(trimmedLine, "pwd") == 0) {
            char cwd[MAXBUF];
            getcwd(cwd, sizeof(cwd));
            printf("%s\n", cwd);
        }
        else {
            commandMode2(trimmedLine, list);
        }
    }
}

/*
 * Mode 1 - we were given the full path
*/
void commandMode1(char *line, char **list, int run) {
    int i = 0;
    list[i] = strtok(line, " ");
    while (list[i] != NULL && i < MAXBUF / 2) {
        list[++i] = strtok(NULL, " ");
    }

    if (access(list[0], F_OK | X_OK) == 0) {
        // File exists and is executable! Can run!

        // run foreground or background?
        if (run != 0) {
            runBackground(list[0], list);
        }
        else {
            runForeground(list[0], list);
        }
    }
    else {
        // No good! File doesn't exist or is not executable!
        printf("Not a valid path.\n");
    }
}


/**
 * Run in background
*/
void runBackground(char *line, char **list) {
    pid_t pid = fork();

    if (pid == 0) {  // child process
        if (execv(line, list)) exit(1);
        // child will become zombie and be automatically reaped?
    }
    // parent process returns to shell
    else {
        list = NULL;
    }
}


/**
 * Run in foreground
*/
void runForeground(char *line, char **list) {
    pid_t pid = fork();

    if (pid == 0) {  // child process
        if (execv(line, list)) {
            exit(1); // exec failed
        }
    }
    else {   // parent
        list = NULL;
        wait(NULL);
    }
}


/*
 * Mode 2 - we were not given the full path
*/
void commandMode2(char *line, char **list) {
    int i = 0;
    list[i] = strtok(line, " ");
    while (list[i] != NULL && i < MAXBUF / 2) {
        list[++i] = strtok(NULL, " ");
    }

    char cwd[MAXBUF];
    getcwd(cwd, sizeof(cwd));

    // cwd now holds the current working directory
    // now, concatenate it with the file name

    size_t length = strlen(cwd) + strlen(line) + 2;
    char *pathname = (char*) malloc(length);

    // construct path with current directory and command name
    snprintf(pathname, length, "%s/%s", cwd, line);

    struct stat fstat;

    if (stat(pathname, &fstat) == 0 && S_ISREG(fstat.st_mode)) {
        // it is present in this directory
        runForeground(line, list);
    }
    else {
        // it is not present in the current directory

        // get the path
        char *path = getenv("PATH");
        char *pathDup = strdup(path);

        int i = 0;
        char *token = strtok(pathDup, ":"); 
        while (token != NULL) {
            // construct path for each possible directory
            char *fullpath = malloc(strlen(token) + strlen(line) + 2);
            snprintf(fullpath, strlen(token) + strlen(line) + 2, "%s/%s", token, line);

            // if we find the file in one of the paths, break
            if (stat(fullpath, &fstat) == 0 && S_ISREG(fstat.st_mode)) {
                i = 1;
                runForeground(fullpath, list);
                free(fullpath);
                break;
            }

            free(fullpath);
            token = strtok(NULL, ":"); // get next directory
        }

        if(i != 1) {
            printf("Command not found.\n");
        }

        free(pathDup);
    }
    free(pathname);
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
void changeDirectory(char *path, int pathGiven) {
    // no path was given - go the home directory
    if (pathGiven == 1) {
        char *home = getenv("HOME");
        if(chdir(home) == -1) {
            perror("ERROR - changing directory failed ");
        }
    }
    else {
        if(chdir(path) == -1) {
            perror("ERROR - changing directory failed ");
        }
    }
}