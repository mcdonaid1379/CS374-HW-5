#include "main.h"

/*log file created globally as it will be needed in every function*/
FILE *log_file;

void log_Command (struct Command *c){
    int i;


    fprintf(log_file, "\nstruct Command log:\ncommand: %s\n", c->command);

    /*logs all the arguments*/
    for (i = 0; i < c->args_counter; i++){
        fprintf(log_file, "arg%d: %s\n", i, c->args[i]);
    }

    fprintf(log_file, "\ninput file: %s", c->input_name);
    fprintf(log_file, "\noutput file: %s", c->output_name);
    fprintf(log_file, "\nbackground process status: %d", c->background);
    fprintf(log_file, "\ncomment status: %d", c->comment);

}

struct Command *get_command () {
    char input[MAX_LENGTH];
    char *token;
    /*delimiter for parsing input*/
    const char delim[2] = " ";
    struct Command *line = (struct Command *)malloc(sizeof(struct Command));
    line->args_counter = 0;
    line->background = 0;
    fprintf(log_file, "command for line struct memory allocated successfully and default values set\n");


    printf(": ");
    
    /*get command line input*/
    fgets(input, MAX_LENGTH, stdin);

    /*get first token*/
    token = strtok(input, delim);
    fprintf(log_file, "first token read\n");

    /*FIXME: does not currently deal with the \n char for the last token*/
    /*iterates through all tokens*/
    while (token != NULL){
        if (strcmp(token, "<") == 0){
            fprintf(log_file, "< parsed\n");
            token = strtok(NULL, delim);
            
            /*store the input file name*/
            line->input_name = token;

            /*store FILE* in line from fopen*/
            line->input_file = fopen(line->input_name, "r");

            /*checks if the file exists*/
            if (line->input_file == NULL){
                printf("cannot open %s for input\n", token);
            } else {
                fprintf(log_file, "File %s successfully opened\n", line->input_name);
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
            line->output_file = fopen(line->output_name, "w");

            /*checks if the file exists*/
            if (line->output_file == NULL){
                printf("cannot open %s for output\n", token);
            } else {
                fprintf(log_file, "File %s successfully opened\n", line->output_name);
            }

            /*reads the next token*/
            token = strtok(NULL, delim);
            fprintf(log_file, "next token read\n");

            continue;

        } else if (strcmp(token, "&\n") == 0) {
            fprintf(log_file, "& parsed\n");

            /*sets the background process status to 1 indicating an & is present*/
            line->background = 1;

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


    /*FIXME: Remove newline character from end of last token*/
    /* After the while loop */
    if (line->args_counter > 0 && line->args[line->args_counter - 1][strlen(line->args[line->args_counter - 1]) - 1] == '\n') {
    /* Remove the newline character */
        line->args[line->args_counter - 1][strlen(line->args[line->args_counter - 1]) - 1] = '\0';
    }

    log_Command(line);

    return line;
}

int exit_smallsh () {
    /*TODO: Kill all other processes*/
    fprintf(log_file, "exit called\n");
    return 1;
}

int run_built_in_command (struct Command *cmd) {
    fprintf(log_file, "running built in command: %s\n", cmd->command);
    int exit_status;

    if (strcmp(cmd->command, "cd") == 0){
        /*TODO: run cd*/
    } else if (strcmp(cmd->command, "exit") == 0){
        exit_status =  exit_smallsh();
    } else if (strcmp(cmd->command, "status") == 0){
        /*TODO: run status*/
    }

    /*FIXME: fix return value*/
    return exit_status;
}

int run_command (struct Command *cmd){
    fprintf(log_file, "\n\nrunning command: %s\n", cmd->command);
    int exit_status;

    /*check for built in commands*/
    if (strcmp(cmd->command, "cd") == 0 || strcmp(cmd->command, "exit") == 0 || strcmp(cmd->command, "status") == 0) {
        exit_status = run_built_in_command(cmd);
    }
    
    return exit_status;
}

void cmd () {
    int exit_status = 0;
    
    while (exit_status != 1){
        /*Get the command*/
        struct Command *cmd = get_command();

        /*run the command*/
        exit_status = run_command(cmd);
    }
    /*TODO: add handle exit properly*/

    return;
    
}

void init_log_file(){
    /*initialize the log file*/
    log_file = fopen("logfile.txt", "w");
    if (log_file == NULL) {
        printf("Error opening log file!\n");
        return;
    }

    fprintf(log_file, "Log file opened\n");
    return;
}

int main () {
    init_log_file();
    
    printf("$ smallsh\n");

    cmd();
    return 0;
}