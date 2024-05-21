#ifndef MAIN_H
#define MAIN_H

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <bits/waitflags.h>

#define MAX_PID_LENGTH 10 

#define MAX_LENGTH 2048

/*Command struct is where the input from the command line will be stored*/
struct Command {
    /*initial command*/
    char *command;
    /*contains up to 512 args*/
    char *args[512];
    int args_counter;
    /*files for I/O*/
    char *input_name;
    FILE *input_file;
    char *output_name;
    FILE *output_file;
    /*0 if there is no & and 1 if there is*/
    int background;
    /*0 if there is no # and 1 if there is*/
    int comment;
};

void log_Command (struct Command *c);
void remove_newline(char *str);
struct Command *get_command ();
void exit_smallsh (struct Command *cmd);
void run_built_in_command (struct Command *cmd);
void exec_fore (struct Command *cmd);
void sigchld_handler(int signum);
void exec_back (struct Command *cmd);
void run_command (struct Command *cmd);
void cmd ();
void init_log_file();
int main ();
struct Command *get_command ();

#endif