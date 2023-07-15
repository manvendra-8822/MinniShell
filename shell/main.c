#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
 
// Function Declarations for builtin shell commands:
int minni_cd(char **args);
int minni_help(char **args);
int minni_exit(char **args);
int minni_customise(char **args);
 
// List of builtin commands, followed by their corresponding functions.
char *builtin_str[] = {
  "cd",
  "help",
  "exit",
  "custom",
  "date"
};
 
int (*builtin_func[]) (char **) = {
  &minni_cd,
  &minni_help,
  &minni_exit,
  &minni_customise
};
 
int minni_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}
 
// Builtin function implementations.
 
/**
   Builtin command: change directory.
   Always returns 1.
 */
int minni_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "minni: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("minni");
    }
  }
  return 1;
}
 
 
/**
   Builtin command: customize.
   args List of args. 
   Always returns 1.
 */
int minni_customise(char **args)
{
printf("Enter MinniShell to see the name of creater\n");
char str[30];
scanf("%s",str);
if(strcmp(str,"MinniOS"))
{
printf("Manvendra Raj Singh\n");
}
else
printf("Wrong input\n");
 
return 1;
}
/**
   Builtin command: print help.
   args List of args. 
   Always returns 1.
 */
int minni_help(char **args)
{
  int i;
  printf("My Project - MinniShell\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");
 
  for (i = 0; i < minni_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }
 
  printf("Use the man command for information on other programs.\n");
  return 1;
}
 
/**
    Builtin command: exit.
    List of args. 
    Always returns 0, to terminate execution.
 */
int minni_exit(char **args)
{
  printf("Exiting.....\n");
  return 0;
}
 
/**
  Launch a program and wait for it to terminate.
  args Null terminated list of arguments (including program).
  Always returns 1, to continue execution.
 */
int minni_launch(char **args)
{
  pid_t pid;
  int status;
 
  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("minni");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("minni");
  } else {
    // Parent process
    do {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }
 
  return 1;
}
 
/**
   @brief Execute shell built-in or launch program.
   @param args Null terminated list of arguments.
   @return 1 if the shell should continue running, 0 if it should terminate
 */
int minni_execute(char **args)
{
  int i;
 
  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }
 
  for (i = 0; i < minni_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }
 
  return minni_launch(args);
}
 
/**
   @brief Read a line of input from stdin.
   @return The line from stdin.
 */
char *minni_read_line(void)
{
#ifdef minni_USE_STD_GETLINE
  char *line = NULL;
  ssize_t bufsize = 0; // have getline allocate a buffer for us
  if (getline(&line, &bufsize, stdin) == -1) {
    if (feof(stdin)) {
      exit(EXIT_SUCCESS);  // We received an EOF
    } else  {
      perror("minni: getline\n");
      exit(EXIT_FAILURE);
    }
  }
  return line;
#else
#define minni_RL_BUFSIZE 1024
  int bufsize = minni_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;
 
  if (!buffer) {
    fprintf(stderr, "minni: allocation error\n");
    exit(EXIT_FAILURE);
  }
 
  while (1) {
    // Read a character
    c = getchar();
 
    if (c == EOF) {
      exit(EXIT_SUCCESS);
    } else if (c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;
 
    // If we have exceeded the buffer, reallocate.
    if (position >= bufsize) {
      bufsize += minni_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "minni: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
#endif
}
 
#define minni_TOK_BUFSIZE 64
#define minni_TOK_DELIM " \t\r\n\a"
/**
   Split a line into tokens (very naively).
   line The line.
   Null-terminated array of tokens.
 */
char **minni_split_line(char *line)
{
  int bufsize = minni_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token, **tokens_backup;
 
  if (!tokens) {
    fprintf(stderr, "minni: allocation error\n");
    exit(EXIT_FAILURE);
  }
 
  token = strtok(line, minni_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;
 
    if (position >= bufsize) {
      bufsize += minni_TOK_BUFSIZE;
      tokens_backup = tokens;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
		free(tokens_backup);
        fprintf(stderr, "minni: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
 
    token = strtok(NULL, minni_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}
 
/**
    Loop getting input and executing it.
 */
void minni_loop(void)
{
  char *line;
  char **args;
  int status;
 
  do {
    printf("> ");
    line = minni_read_line();
    args = minni_split_line(line);
    status = minni_execute(args);
 
    free(line);
    free(args);
  } while (status);
}
 
/**
    Main entry point.
    argc Argument count.
    argv Argument vector.
    status code
 */
int main(int argc, char **argv)
{
  // Load config files, if any.
  // Run command loop.
  minni_loop();
  // Perform any shutdown/cleanup.
 
  return EXIT_SUCCESS;
}