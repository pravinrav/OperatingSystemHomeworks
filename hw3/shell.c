#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include "tokenizer.h"

/* Convenience macro to silence compiler warnings about unused function parameters. */
#define unused __attribute__((unused))

FILE * input; 
FILE * output;

/* Whether the shell is connected to an actual terminal or not. */
bool shell_is_interactive;

/* File descriptor for the shell input */
int shell_terminal;

/* Terminal mode settings for the shell */
struct termios shell_tmodes;

/* Process group id for the shell */
pid_t shell_pgid;

int cmd_exit(struct tokens *tokens);
int cmd_help(struct tokens *tokens);
int cmd_cd(struct tokens *tokens);
int cmd_pwd(struct tokens *tokens);

/* Built-in command functions take token array (see parse.h) and return int */
typedef int cmd_fun_t(struct tokens *tokens);

/* Built-in command struct and lookup table */
typedef struct fun_desc {
  cmd_fun_t *fun;
  char *cmd;
  char *doc;
} fun_desc_t;

fun_desc_t cmd_table[] = {
  {cmd_help, "?", "show this help menu"},
  {cmd_exit, "exit", "exit the command shell"},
  {cmd_cd, "cd", "change the current working directory"},
  {cmd_pwd, "pwd", "print the current working directory"}
};

int cmd_pwd(unused struct tokens * tokens) {
  char workingDirectory[100];
  getcwd(workingDirectory, sizeof(workingDirectory));
  fprintf(stdout, "%s\n", workingDirectory);

  return 1;
}

int cmd_cd(struct tokens *tokenList) {
  size_t length = tokens_get_length(tokenList);

  const char * path = tokens_get_token(tokenList, length - 1); 

  chdir(path);

  return 1;
}

/* Prints a helpful description for the given command */
int cmd_help(unused struct tokens *tokens) {
  for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
    printf("%s - %s\n", cmd_table[i].cmd, cmd_table[i].doc);
  return 1;
}

/* Exits this shell */
int cmd_exit(unused struct tokens *tokens) {
  exit(0);
}

/* Looks up the built-in command, if it exists. */
int lookup(char cmd[]) {
  for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
    if (cmd && (strcmp(cmd_table[i].cmd, cmd) == 0))
      return i;
  return -1;
}

char * substringSearch(char * bigPath, char* littlePath) {
  size_t len = strlen(bigPath);
  if (len == 0) {
    return NULL;
  }
  int length = 0;
  while (length < len) {
    if (bigPath[length] != ':') {
      length++;
    }
  }

  char * allocated = malloc(sizeof(char) * (length + 2 + strlen(littlePath)));
  strncpy(allocated, bigPath, length);
  strcat(allocated, "/"); 
  strcat(allocated, littlePath);

  int filePresent;

  filePresent = access(allocated, F_OK);
  if (filePresent != -1) {
    return allocated;
  }

  return substringSearch(bigPath + length + 1, littlePath);

}
  



char * getProperPath(char * command) {
  char * currentPath = getenv("PATH");
  char * allocatedCMD = malloc(4096); 
  if (command[0] == '.') {
    allocatedCMD = realpath(command, allocatedCMD);
    int filePresent = access(allocatedCMD, -1);
    if (filePresent != 1) {
      return allocatedCMD;
    } else {
      return NULL;
    } 
  }
  return substringSearch(currentPath, command);
}

/* Intialization procedures for this shell */
void init_shell() {
  /* Our shell is connected to standard input. */
  shell_terminal = STDIN_FILENO;

  /* Check if we are running interactively */
  shell_is_interactive = isatty(shell_terminal);

  if (shell_is_interactive) {
    /* If the shell is not currently in the foreground, we must pause the shell until it becomes a
     * foreground process. We use SIGTTIN to pause the shell. When the shell gets moved to the
     * foreground, we'll receive a SIGCONT. */
    while (tcgetpgrp(shell_terminal) != (shell_pgid = getpgrp()))
      kill(-shell_pgid, SIGTTIN);

    /* Saves the shell's process id */
    shell_pgid = getpid();

    /* Take control of the terminal */
    tcsetpgrp(shell_terminal, shell_pgid);

    /* Save the current termios to a variable, so it can be restored later. */
    tcgetattr(shell_terminal, &shell_tmodes);
  }
}

int main(unused int argc, unused char *argv[]) {
  init_shell();

  static char line[4096];
  int line_num = 0;

  /* Please only print shell prompts when standard input is not a tty */
  if (shell_is_interactive)
    fprintf(stdout, "%d: ", line_num);

  input = stdin; 
  output = stdout;

  while (fgets(line, 4096, stdin)) {
    /* Split our line into words. */
    struct tokens *tokens = tokenize(line);

    /* Find which built-in function to run. */
    int fundex = lookup(tokens_get_token(tokens, 0));

    if (fundex >= 0) {

      int counter = 0;
      size_t lengthTokens = tokens_get_length(tokens);

      for (int k = 0; k < lengthTokens; k++) {
        char * token = tokens_get_token(tokens, k);

        if (token[0] == '<') {
          k++;
          char * file = tokens_get_token(tokens, k);
          input = fopen(file, "r");

          fgets(line, 4096, input);
          tokens = tokenize(line);

          counter = counter + 1;
        }

        if (token[0] == '>') {
          k++;
          char * file = tokens_get_token(tokens, k);

          output = fopen(file, "w");

          counter = counter + 2;
        }

      }

      cmd_table[fundex].fun(tokens);

      if (counter & 1 != 0) {
        fclose(input);
        input = stdin;
      } else if (counter & 2 == 0) {
        fclose(output);
        output = stdout;
      }


    } else {
      /* REPLACE this to run commands as programs. */
      // COMMENT: fprintf(stdout, "This shell doesn't know how to run programs.\n");
      size_t tokenLength = tokens_get_length(tokens);

      char * programName = tokens_get_token(tokens, 0);
      char * programPath = getProperPath(programName);
      if (programPath == NULL) {
        printf("no program named this");
      } else {
        int status;
        pid_t process = fork();

        if (process == 0) { // If we are in the child process
          setpgid(getpid(), getpid()); 
          int counter = 0;

          char * parameterList[tokenLength + 1];
          int index = 0;

          for (int j = 0; j < tokenLength; j++) {
            char * token = tokens_get_token(tokens, j);
            if (token[0] == '<') {
              j++;
              char * file = tokens_get_token(tokens, j);
              freopen(file, "r", stdin);
              counter = counter + 1;
            }
            else if (token[0] == '<') {
              j++;
              char * file = tokens_get_token(tokens, j);
              freopen(file, "w", stdout);
              counter = counter + 2;
            }
            else {
              parameterList[index++] = tokens_get_token(tokens, j);
            }
          }

          parameterList[index] = NULL;
          execv(programPath, parameterList); 

          if (counter & 2 != 0) {
            fclose(stdout);
          }

          if (counter & 1 != 0) {
            fclose(stdin);
          }

          exit(0);
        }

        if (process > 0) {  // In parent process
          wait(&status); // wait for child to finish
        }
        
      }

    }

    if (shell_is_interactive)
      /* Please only print shell prompts when standard input is not a tty */
      fprintf(stdout, "%d: ", ++line_num);

    /* Clean up memory */
    tokens_destroy(tokens);
  }

  return 0;
}
