#include "main.h"

/*log file created globally as it will be needed in every function*/
FILE *log_file;
int exit_status;
/*0 for disabled 1 for enabled*/
volatile int foreground_only = 0;

void log_Command (struct Command *c){
    int i;


    fprintf(log_file, "\nstruct Command log:\ncommand: %s\n", c->command);

    /*logs all the arguments*/
    for (i = 0; i < c->args_counter; i++){
        fprintf(log_file, "arg%d: %s\n", i, c->args[i]);
    }
    fprintf(log_file, "\nargument counter: %d", c->args_counter);

    fprintf(log_file, "\ninput file: %s", c->input_name);
    fprintf(log_file, "\noutput file: %s", c->output_name);
    fprintf(log_file, "\nbackground process status: %d", c->background);
    fprintf(log_file, "\ncomment status: %d", c->comment);

}

void remove_newline(char *str) {
        int len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

void expand_variables(struct Command *cmd) {
    int i;
    pid_t pid = getpid();
    char pid_str[MAX_PID_LENGTH + 1]; // +1 for null terminator
    char *found;

    // Convert PID to string
    sprintf(pid_str, "%d", pid);
    int pid_length = strlen(pid_str);

    for (i = 0; i < cmd->args_counter; i++) {
        found = strstr(cmd->args[i], "$$");

        while (found != NULL) {
            memmove(found + pid_length, found + 2, strlen(found + 2) + 1);
            //memmove(found + pid_length, found + 2, (strlen(found) - strlen("$$")) + 1); // +1 for null terminator
            memcpy(found, pid_str, pid_length);

            found = strstr(found + pid_length, "$$"); // Find next occurrence
        }
    }
}



struct Command *get_command () {
    char input[MAX_LENGTH];
    char *token;
    /*delimiter for parsing input*/
    const char delim[2] = " ";
    struct Command *line = (struct Command *)malloc(sizeof(struct Command));
    line->args_counter = 0;
    line->background = 0;
    line->comment = 0;
    fprintf(log_file, "\n\ncommand for line struct memory allocated successfully and default values set\n");


    printf(": ");
    fflush(stdout);
    
    /*get command line input*/
    fgets(input, MAX_LENGTH, stdin);
    remove_newline(input);

    /*get first token*/
    token = strtok(input, delim);
    fprintf(log_file, "first token read\n");

    /*iterates through all tokens*/
    while (token != NULL){
        if (strcmp(token, "<") == 0){
            fprintf(log_file, "< parsed\n");
            token = strtok(NULL, delim);
            
            /*store the input file name*/
            line->input_name = token;

            /*store FILE* in line from fopen*/
            line->input_file = open(line->input_name, O_RDONLY);

            /*checks if the file exists*/
            if (line->input_file == -1){
                printf("cannot open %s for input\n", token);
                fflush(stdout);
            } else {
                fprintf(log_file, "File %s successfully opened fo input with fd: %d\n", line->input_name, line->input_file);
            }

            /*reads the next token*/
            token = strtok(NULL, delim);
            fprintf(log_file, "next token read\n");

            continue;
            
        } else if (strcmp(token, ">") == 0){
            fprintf(log_file, "> parsed\n");
            token = strtok(NULL, delim);

            /*store the output file name*/
            line->output_name = token;
            
            /*store FILE* in line from fopen*/
            line->output_file = open(line->output_name, O_WRONLY | O_CREAT | O_TRUNC, 0666); 

            /*checks if the file exists*/
            if (line->output_file == -1){
                printf("cannot open %s for output\n", token);
                fflush(stdout);
            } else {
                fprintf(log_file, "File %s successfully opened for output with fd: %d\n", line->output_name, line->output_file);
            }

            /*reads the next token*/
            token = strtok(NULL, delim);
            fprintf(log_file, "next token read\n");

            continue;

        } else if (strcmp(token, "&") == 0) {
            fprintf(log_file, "& parsed\n");


            fprintf(log_file, "foreground only mode = %d\n", foreground_only);
            /*sets the background process status to 1 indicating an & is present*/
            if (foreground_only == 1){
                //dont switch to background process
                line->background = 0;
            } else {
                line->background = 1;
            }
            fprintf(log_file, "background status = %d\n", line->background);

            /*reads the next token*/
            token = strtok(NULL, delim);
            fprintf(log_file, "next token read\n");

            continue;
        } else if (strcmp(token, "#") == 0){
            fprintf(log_file, "# parsed\n");

            /*sets the comment status to 1 indicating an & is present*/
            line->comment = 1;

            /*reads the next token*/
            token = strtok(NULL, delim);
            fprintf(log_file, "next token read\n");

            continue;
        } else if (line->command == NULL) {

            line->command = token;
            fprintf(log_file, "first command added to line struct successfully\n");

            /*reads the next token*/
            token = strtok(NULL, delim);
            fprintf(log_file, "next token read\n");

            continue;

        } else {
            /*handles arguments*/
            line->args[line->args_counter] = token;
            line->args_counter ++;
            fprintf(log_file, "argument %d added to line->args\n", line->args_counter);

            token = strtok(NULL, delim);
            fprintf(log_file, "next token read\n");
        }
        
    }

    log_Command(line);

    return line;
}

void exit_smallsh (struct Command *cmd) {
    /*FIXME: Kill all other processes*/
    /*FIXME: fix exit needing to be called twice sometimes*/
    fprintf(log_file, "exit called\n");
    free(cmd);
    exit(0);
}

void status () {
    printf("exit value %d\n", exit_status);
    fflush(stdout);
    return;
}

void cd (struct Command *cmd) {
    fprintf(log_file, "cd called\nchanging directory to %s\n", cmd->args[0]);
    chdir(cmd->args[0]);
    return;
}

void run_built_in_command (struct Command *cmd) {
    fprintf(log_file, "running built in command: %s\n", cmd->command);

    if (strcmp(cmd->command, "cd") == 0){
        cd(cmd);
    } else if (strcmp(cmd->command, "exit") == 0){
        exit_smallsh(cmd);
    } else if (strcmp(cmd->command, "status") == 0){
        status();
    }

    return;
}


void exec_fore (struct Command *cmd){
    int i;
    exit_status = 0;

    pid_t pid = fork();

    if (pid < 0) {
        /*fork failure*/
        fprintf(log_file, "Fork failed");
        exit_status = 1;
    } else if (pid == 0) {
        /*Child process*/ 

        /*Constructing the argument list for execvp*/
        char *args[cmd->args_counter + 2]; /*+2 for the command and NULL terminator*/ 
        args[0] = cmd->command;
        for (i = 0; i < cmd->args_counter; i++) {
            args[i + 1] = cmd->args[i];
        }
        args[cmd->args_counter + 1] = NULL;

        /*child process signal handler*/
        /*FIXME: does not currently work, is overridden by previous signal*/
        //signal(SIGINT, fore_SIGINT_handler);

        //redirects the ouput to the file if it has been opened
        if (cmd->output_file){
            if (cmd->output_file != -1){
                dup2(cmd->output_file, STDOUT_FILENO);
                dup2(cmd->output_file, STDERR_FILENO);
                close(cmd->output_file);
            }
        }

        //redirects the input
        if (cmd->input_file){
            if (cmd->input_file != -1){
                dup2(cmd->input_file, STDIN_FILENO);
                close(cmd->input_file);
            }
        }

        execvp(cmd->command, args);

        /*If execvp returns, it means there was an error*/ 
        fprintf(log_file, "Exec failed");
        exit_status = 1;
    } else {
        /*Parent process*/ 
        int status;
        /*Signal handler for SIGINT reporting*/
        //signal(SIGCHLD, fore_sigchld_handler);

        /*Waiting for the child process to finish*/ 
        waitpid(pid, &status, 0);
    }

}

void sigchld_handler(int signum) {
    int status;
    pid_t pid;
    
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (WIFEXITED(status)) {
            printf("Background Process with PID %d completed.\n", pid);
            fflush(stdout);
        } else {
            printf("Background Process with PID %d terminated abnormally.\n", pid);
            fflush(stdout);
        }
    }
}

void exec_back (struct Command *cmd){
    int i, j;
    exit_status = 0;

    fprintf(log_file, "running %s in the background \n", cmd->command);
    pid_t pid = fork();

    if (pid < 0) {
        /*fork failure*/
        fprintf(log_file, "\nFork failed");
        exit_status = 1;
    } else if (pid == 0) {
        /*Child process*/ 
        /*outputting pid*/
        pid_t current_pid = getpid();
        printf("Background Process PID: %d\n", current_pid);
        fflush(stdout);
        /*Constructing the argument list for execvp*/ 
        /*Dynamically allocate memory for args array*/ 
        char **args = (char **)malloc((cmd->args_counter + 2) * sizeof(char *));
        if (args == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }

        /*Constructing the argument list for execvp*/ 
        args[0] = cmd->command;
        for (i = 0; i < cmd->args_counter; i++) {
            // Dynamically allocate memory for each argument
            args[i + 1] = strdup(cmd->args[i]);
            if (args[i + 1] == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
                // Free allocated memory before exiting
                for (j = 0; j <= i; j++) {
                    free(args[j]);
                }
                free(args);
                exit(1);
            }
        }
        args[cmd->args_counter + 1] = NULL;

        //redirects the ouput to the file if it has been opened
        if (cmd->output_file){
            if (cmd->output_file != -1){
                dup2(cmd->output_file, STDOUT_FILENO);
                dup2(cmd->output_file, STDERR_FILENO);
                close(cmd->output_file);
            }
        }

        //redirects the input
        if (cmd->input_file){
            if (cmd->input_file != -1){
                dup2(cmd->input_file, STDIN_FILENO);
                close(cmd->input_file);
            }
        }

        execvp(cmd->command, args);

        /*If execvp returns, it means there was an error*/ 
        fprintf(log_file, "\nExec failed");
        exit_status = 1;
    } else {
        /*Parent process*/ 
        /*set up handler for SIGCHLD when the child terminates*/
        signal(SIGCHLD, sigchld_handler);

        return;
    }

}

void run_command (struct Command *cmd){
    fprintf(log_file, "\n\nrunning command: %s\n", cmd->command);

    /*check for built in commands*/
    if (strcmp(cmd->command, "cd") == 0 || strcmp(cmd->command, "exit") == 0 || strcmp(cmd->command, "status") == 0) {
        run_built_in_command(cmd);
    } else if (cmd->background == 0){
        /*run command in foreground*/
        exec_fore(cmd);

    } else if (cmd->background == 1){
        /*run command in background*/
        exec_back(cmd);

    } else {
        printf("Error running command\n");
        fflush(stdout);
    }
    
    return;
}

void cmd () {
    exit_status = 0;
    
    while (1 < 2){
        /*Get the command*/
        struct Command *cmd = get_command();

        /*exapnds all the $$ to the current pid*/
        expand_variables(cmd);

        /*run the command*/
        run_command(cmd);
    }

    return;
}

void init_log_file(){
    /*initialize the log file*/
    log_file = fopen("logfile.txt", "w");
    if (log_file == NULL) {
        printf("Error opening log file!\n");
        fflush(stdout);
        return;
    }

    fprintf(log_file, "Log file opened\n");
    return;
}

void SIGINT_handler () {
    fprintf(log_file, "SIGINT ignored by main process\n");
    fflush(stdout);
    return;
}

void SIGTSTP_handler () {
    fprintf(log_file, "SIGTSTP received\n");
    if (foreground_only == 0){
        foreground_only = 1;
        printf("Enabling foreground only mode\n");
        fflush(stdout);
    } else {
        foreground_only = 0;
        printf("Disabling foreground only mode\n");
        fflush(stdout);
    }
}

int main () {
    init_log_file();

    signal(SIGINT, SIGINT_handler);

    signal(SIGTSTP, SIGTSTP_handler);
    
    printf("$ smallsh\n");
    fflush(stdout);

    cmd();
    return 0;
}