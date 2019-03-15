/**
*FILE:jonsh.c
*AUTHOR: Jon Novak
*PURPOSE: Basic Shell
*/
#define _GNU_SOURCE //used for setenv and unsetenv
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<string.h>
#include<getopt.h>//helps accept command line options
#include<ctype.h>//used to help split commands

#define MAX_PROMPT 128 //max size of prompt
#define MAX_LINE_LEN 4096 //max size of line
#define MAX_WORDS 64	//max words
#define MAX_WORD_LENGTH 128	//max word length

void usage(void)
{
	printf("./jonsh -p <prompt>\n");
}

void jonsh_fail(char *msg) //prints error messages
{
	fprintf(stderr, "ERROR: %s\n", msg);
	exit(EXIT_FAILURE);
}

void jonsh_exec(char **cmd, int num_args, int back){ //Handles non built in commands and child spawning

	pid_t pid;
	char **exec_args = malloc(sizeof(char *) * (num_args + 1));
	if(exec_args == NULL){ //checks to see if memory allocated
		jonsh_fail("Failed to allocate memory for space.");
	}

	for(int i = 0; i < num_args; i++){
		exec_args[i] = cmd[i];
	}	
	exec_args[num_args] = NULL;	
	pid = fork();
	
	if(pid == -1){	//checks if process can fork
		jonsh_fail("Can't fork anymore processes");
	}
	else if(pid == 0){
		int error = 0;
		printf("Child PID: %d\n", getpid());
		error = execvp(cmd[0], exec_args);
		if(error != 0) {
			fprintf(stderr, "Command %s could not be found\n", cmd[0]);
		}
		_exit(0);
		
			}
	
	else{
	  	if(!back){	//handles background process
			int status = 0;
			waitpid(pid, &status, 0);
			if(WIFEXITED(status)){
				printf("pid %d exited normally\n", pid);
			}else if (WIFSIGNALED(status)){
				printf("pid %d was killed by a signal\n", pid);
			}
		}
	}
	for(int i = num_args - 1; i >= 0; i--){ //cleans out exec args
		exec_args[i][0] = '\0';
	}
	free(exec_args);
	
	return;
	
}

int main(int argc, char **argv)
{
	char *prompt = malloc(sizeof(char)*MAX_PROMPT); //setup prompt
	if(prompt == NULL){ //checks to see if memory allocated
		jonsh_fail("Failed to allocate memory for space.");
	}

	char *line = malloc(sizeof(char)*MAX_LINE_LEN); //setup line
	if(line == NULL){
		jonsh_fail("Failed to allocate memory for input line.");
	}

	char **command = malloc(sizeof(char *)*MAX_WORDS); //setup command word array
	if(command == NULL){
		jonsh_fail("Failed to allocate memory for command word array.");
	}
	
	for(int i = 0; i<MAX_WORDS;i++){
		command[i] = malloc(sizeof(char)*MAX_WORD_LENGTH); //allocate space in the array
		if(command[i] == NULL){
			jonsh_fail("Failed to allocate memory for word array contents.");
					}		
	}

	int opt = 0;
	prompt = "308sh> ";

	while ((opt = getopt(argc, argv, "p:")) != -1) //handles p flag
	{
		switch(opt) 	{
			case 'p':	
				prompt = optarg;
				break;
			default:
				usage();
				exit(EXIT_FAILURE);
				}
	}

	int lineIndex = 0;
	int cmdIndex = 0;
	int wordIndex = 0;
	char c = 0;
	int background = 0;

	while(1) {
		lineIndex = 0;
		cmdIndex = 0;
		wordIndex = 0;
		c = 0;
		background = 0;
		
		printf("%s", prompt);
		fgets(line, MAX_LINE_LEN, stdin); //gets user input
		printf("Your command: %s\n", line);	
		
		c = line[0];
		while (c != '\0') { //splits up commands
			

			if(!isspace(c)) { //checks if the char is a space
				command[cmdIndex][wordIndex] = c;
				wordIndex++;
			}
			else{
				command[cmdIndex][wordIndex] = '\0';
				cmdIndex++;
				wordIndex = 0;
			}
			lineIndex++;
			c = line[lineIndex];
		

				}

	for (int i = 0; i < cmdIndex; i++){
		printf("Command word %d: %s\n", i, command[i]);
			}

	//executes builtin commands
	if (strcmp(command[0], "exit") == 0) {	//handles exit command
		exit(EXIT_SUCCESS);
		}

	else if (strcmp(command[0], "pid") == 0) {	//handles pid command
		printf("Process ID: %d\n", getpid());
		}

	else if (strcmp(command[0], "ppid") == 0) {	//handles ppid command
		printf("Parent process ID: %d\n", getppid());
		}

	else if (strcmp(command[0], "pwd") == 0) {	//handles pwd command
		char cwd[4096];
		getcwd(cwd, sizeof(cwd));
		printf("Dir: %s\n", cwd);
		}

	else if (strcmp(command[0], "get") == 0){	//handles get command
		if (strlen(command[1]) ==0){
			fprintf(stderr, "You must specify a variable to get\n");
		}else{
			printf("%s : %s\n", command[1], getenv(command[1]));
		}
		}

	else if (strcmp(command[0], "cd") == 0) {	//handler cd command
		if(strlen(command[1]) == 0)
		{
		chdir(getenv("HOME"));
		}else{
		chdir(command[1]); 
			}
		}

	else if (strcmp(command[0], "set") == 0){	//handles set command
		if (strlen(command[1]) == 0){
			fprintf(stderr, "Usage: set <var> <variable>\n");
			}
		else if(strlen(command[2]) == 0){
			printf("Unsetting %s\n", command[1]);
			unsetenv(command[1]);
		}
		else{
			printf("Setting %s to %s\n", command[1], command[2]);
			setenv(command[1], command[2], 1);
		}
		}
		//END OF BUILT IN COMMANDS	
	else{//executes non builtin commands

		
		int args = 0;
		char *arg = command[0];
		while (strcmp(arg, "") != 0){
			if(strcmp(arg, "&") == 0){ //checks if process should be run in background
				background = 1;
				break;
			}
			args++;
			arg = command[args];
		}
		jonsh_exec(command, args, background);
	}
	
	
	
	//used to ensure that words are not hanging around	
	for(int i = cmdIndex; i >= 0; i--){
		command[cmdIndex][0] = '\0';
	}
	cmdIndex = 0;	
}	
	exit(EXIT_SUCCESS);
}

	
