  
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);
int lsh_history(char **args);
char *builtin_str[] = {"cd","ayuda","chau","historia"};

int(*builtin_func[]) (char **) = {&lsh_cd,&lsh_help,&lsh_exit,&lsh_history};
int lsh_num_builtins() {
	return sizeof(builtin_str) / sizeof(char *);
}
#define HISTORY_COUNT 30
#define LEN 30
#define lacena 30
char* alacena[lacena];
int lsh_history(char **args) {
	int i = 0;
	for (i; i < 30; i++) {
		if (alacena[i] == NULL) {
			break;
		}
		printf("%4d %s\n", i, alacena[i]);
	}
	return 1;
}
int lsh_cd(char **args)
{
	if (args[1] == NULL) {
		fprintf(stderr, "lsh: expected argument to \"cd\"\n");
	}
	else {
		if (chdir(args[1]) != 0) {
			perror("lsh");
		}
	}
	return 1;
}
int lsh_help(char **args)
{
	int i;
	printf("Shell basico\n");
	printf("comandos basicos y parser.\n");
	printf("comandos extra implementados:\n");

	for (i = 0; i < lsh_num_builtins(); i++) {
		printf("  %s\n", builtin_str[i]);
	}
	return 1;
}
int lsh_exit(char **args)
{
	return 0;
}
int current = 0;
void almacenar(char* args) {
	alacena[current] = strdup(args);
	current = current + 1;
}
int lsh_execute(char **args)
{
	int i;

	if (args[0] == NULL) {
		// An empty command was entered.
		return 1;
	}
	for (i = 0; i < lsh_num_builtins(); i++) {
		if (strcmp(args[0], builtin_str[i]) == 0) {
			return (*builtin_func[i])(args);
		}
	}
	return shell_execute(args);
} 

///////////////////////////////////////////////////////
int shell_execute(char **args)
{
	pid_t pid;
	int status;

	if (args[0] == NULL) {
		return 1;
	}

	pid = fork();
	if (pid == 0) {
		// Child process
		if (execvp(args[0], args) == -1) {
			perror("shell");
		}
		exit(EXIT_FAILURE);
	}
	else if (pid < 0) {
		// Error forking
		perror("shell");
	}
	else {
		// Parent process
		do {
			waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}

	return 1;
}

///////////////////////////////////////////////////////
#define SHELL_RL_BUFSIZE 1024

char *shell_read_line(void)
{
	int bufsize = SHELL_RL_BUFSIZE;
	int position = 0;
	char *buffer = malloc(sizeof(char) * bufsize);
	int c;

	if (!buffer) {
		fprintf(stderr, "shell: allocation error\n");
		exit(EXIT_FAILURE);
	}

	while (1) {
		c = getchar();

		if (c == EOF) {
			exit(EXIT_SUCCESS);
		}
		else if (c == '\n') {
			buffer[position] = '\0';
			return buffer;
		}
		else {
			buffer[position] = c;
		}
		position++;

		if (position >= bufsize) {
			bufsize += SHELL_RL_BUFSIZE;
			buffer = realloc(buffer, bufsize);
			if (!buffer) {
				fprintf(stderr, "shell: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}
	}
}

///////////////////////////////////////////////////////////
#define SHELL_TOK_BUFSIZE 64
#define SHELL_TOK_DELIM " \t\r\n\a"

char **shell_split_line(char *line)
{
	int bufsize = SHELL_TOK_BUFSIZE, position = 0;
	char **tokens = malloc(bufsize * sizeof(char*));
	char *token, **tokens_backup;

	if (!tokens) {
		fprintf(stderr, "shell: allocation error\n");
		exit(EXIT_FAILURE);
	}

	token = strtok(line, SHELL_TOK_DELIM);
	while (token != NULL) {
		tokens[position] = token;
		position++;

		if (position >= bufsize) {
			bufsize += SHELL_TOK_BUFSIZE;
			tokens_backup = tokens;
			tokens = realloc(tokens, bufsize * sizeof(char*));
			if (!tokens) {
				free(tokens_backup);
				fprintf(stderr, "shell: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}

		token = strtok(NULL, SHELL_TOK_DELIM);
	}
	tokens[position] = NULL;
	return tokens;
}

///////////////////////////////////////////////////////////
void shell_loop(void)
{
	char *line;
	char **args;
	int status;

	do {
		printf("$ ");
		line = shell_read_line();
		almacenar(line);
		args = shell_split_line(line);
		status = lsh_execute(args);

		free(line);
		free(args);
	} while (status);
}
///////////////////////////////////////////////////////////
int main(int argc, char **argv)
{

	shell_loop();

	return EXIT_SUCCESS;
}
