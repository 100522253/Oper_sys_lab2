#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

/* CONST VARS */
const int max_line = 1024;
const int max_commands = 10;
#define max_redirections 3 // stdin, stdout, stderr
#define max_args 15
#define BUFFER_SIZE 1024

/* VARS TO BE USED FOR THE STUDENTS */

// structure equivalent to the "argv" that stores the command line when
// executing a program.
char *argvv[max_args];
// redirections array. If a redirection is detected, the file name is referenced
// at the corresponding position
char *filev[max_redirections];
// indicates whether a command or command sequence is to be executed in
// foreground (0) or bg (1).
int background = 0;

int tokenizar_linea(char *linea, char *delim, char *tokens[], int max_tokens) {
	/*
	This function splits a char* line into different tokens based on a given
	character
	@return Number of tokens
	*/
	int i = 0;
	char *token = strtok(linea, delim);
	while (token != NULL && i < max_tokens - 1) {
		tokens[i++] = token;
		token = strtok(NULL, delim);
	}
	tokens[i] = NULL;
	return i;
}

void procesar_redirecciones(char *args[]) {
	/*
	This function processes the command line to evaluate if there are
	redirections. If any redirection is detected, the destination file is
	indicated in filev[i] array. filev[0] for STDIN filev[1] for STDOUT filev[2]
	for STDERR
	*/

	// initialization for every command
	filev[0] = NULL;
	filev[1] = NULL;
	filev[2] = NULL;
	// Store the pointer to the filename if needed.
	// args[i] set to NULL once redirection is processed
	for (int i = 0; args[i] != NULL; i++) {
		if (strcmp(args[i], "<") == 0) {
			filev[0] = args[i + 1];
			args[i] = NULL;
			args[i + 1] = NULL;
		} else if (strcmp(args[i], ">") == 0) {
			filev[1] = args[i + 1];
			args[i] = NULL;
			args[i + 1] = NULL;
		} else if (strcmp(args[i], "!>") == 0) {
			filev[2] = args[i + 1];
			args[i] = NULL;
			args[i + 1] = NULL;
		}
	}
}

void print_commands() {
	/*
	Delete for submission !!!!
	*/
	printf("Comando = %s\n", argvv[0]);
	for (int arg = 1; arg < max_args; arg++)
		if (argvv[arg] != NULL)
			printf("Args = %s\n", argvv[arg]);

	printf("Background = %d\n", background);
	if (filev[0] != NULL)
		printf("Redir [IN] = %s\n", filev[0]);
	if (filev[1] != NULL)
		printf("Redir [OUT] = %s\n", filev[1]);
	if (filev[2] != NULL)
		printf("Redir [ERR] = %s\n", filev[2]);
}

int procesar_linea(char *linea) {
	/*
	This function processes the input command line and returns in global
	variables: argvv -- command an args as argv filev -- files for redirections.
	NULL value means no redirection. background -- 0 means foreground; 1
	background.
	*/
	char *comandos[max_commands];
	int num_comandos = tokenizar_linea(linea, "|", comandos, max_commands);

	// Check if background is indicated
	if (strchr(comandos[num_comandos - 1], '&')) {
		background = 1;
		char *pos = strchr(comandos[num_comandos - 1], '&');
		// remove character
		*pos = '\0';
	}

	// Finish processing
	for (int i = 0; i < num_comandos; i++) {
		int args_count = tokenizar_linea(comandos[i], " \t\n", argvv, max_args);
		procesar_redirecciones(argvv);
		print_commands(); //!!! Deleted for submission
	}

	return num_comandos;
}

// void execute_command(int argc, char *argv[]) {
// 	int i, pid;
// 	if (argc < 2) {
// 		printf("Usage: exec-command <comand>\n");
// 		exit(-1);
// 	}
// 	pid = fork();
// 	if (pid == 0) { // crear hijo
// 		printf("Hijo creado, va a ejecutar el comando\n");
// 		// Ejecución comando
// 		execvp(argv[1], &argv[1]);
// 		printf("ERROR, aqui solo se llega si ha fallado el exec\n");
// 	}
// 	wait(NULL);
// 	printf("FIN del padre\n");
// }

int verify_first_line(const char *path) {
	int fd, sz;

	// Allocate memory according to how many bytes you
	// want to read of the file, in this case "## Script de SSOO\n" is 18
	// character long, so 18 bytes
	char *c = (char *)calloc(18, sizeof(char));

	// Open the file using the provided path and return an error if it
	// doesn't exist
	fd = open(path, O_RDONLY);
	if (fd < 0) {
		perror("Error opening file");
		exit(1);
	}

	// After opening, read 18 chars corresponding to the pseudo
	// shebang and store them in c
	sz = read(fd, c, 18);

	// Append this char to the end of c to know when it ends
	c[sz] = '\0';

	// Return -1 if the psudo shebang is incorrect
	if (strcmp(c, "## Script de SSOO\n") != 0) {
		errno = ENOEXEC;
		perror("Shebang incorrect or missing");
		return -1;
	}

	close(fd);

	return 0;
}

int check_empty_lines(const char *path) {
	// Open file
	int fd = open(path, O_RDONLY);
	if (fd == -1) {
		perror("Error opening file");
		return -1;
	}

	// Allocate memory to read file
	char buffer[BUFFER_SIZE];
	ssize_t bytes_read;

	// Assume the file starts with a newline to detect an
	// empty first line
	char prev_char = '\n';

	// Flag to check empty line
	int in_empty_line = 1;

	// Read file 1KB at a time and process it
	while ((bytes_read = read(fd, buffer, BUFFER_SIZE)) > 0) {
		for (ssize_t i = 0; i < bytes_read; i++) {
			char current_char = buffer[i];

			if (current_char == '\n') {
				if (in_empty_line) {
					// Throw error when empty line is detected
					errno = ENOMSG; // This was the most descriptive error
									// message I could find
					perror("Empty line detected");
					return -1;
				}
				// Assume next line is empty until next iteration
				in_empty_line = 1;

			} else if (current_char != ' ' && current_char != '\t') {
				// No longer on empty line
				in_empty_line = 0;
			}

			prev_char = current_char;
		}
	}

	if (bytes_read == -1) {
		perror("Read error");
	}

	close(fd);

	return 0;
}

int main(int argc, char *argv[]) {
	// Verify Shebang
	verify_first_line(argv[1]);

	// Check no empty lines
	check_empty_lines(argv[1]);

	// char example_line[] = "ls -l | grep scripter | wc -l !> redir_out.txt &";
	// int n_commands = procesar_linea(example_line);

	printf("\nEnd of program succesfull\n");

	return 0;
}