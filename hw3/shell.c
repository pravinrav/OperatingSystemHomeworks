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

/* Whether the shell is connected to an actual terminal or not. */
bool shell_is_interactive;

/* File descriptor for the shell input */
int shell_terminal;

/* Terminal mode settings for the shell */
struct termios shell_tmodes;

/* Process group id for the shell */
pid_t shell_pgid;

// List of defacto commands
int cmd_exit(struct tokens *tokens);
int cmd_help(struct tokens *tokens);
int cmd_cd(struct tokens *tokens);
int cmd_pwd(struct tokens *tokens);

/* Built-in command functions take token array (see parse.h) and return int */
typedef int cmd_fun_t(struct tokens *tokens);

// List of ignore signals
int ignoreSignals[] = {SIGINT, SIGTERM, SIGQUIT, SIGCONT, SIGTSTP, SIGTTOU, SIGTTIN};

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

int locateProgramPath(char * name, char * argList[]) {

  char completePath[4096];
  char * PATH = getenv("PATH");

  char * token = strtok(PATH, ":");

  while (token != NULL) {
    // Build the path token by token
    sprintf(completePath, "%s/%s", token, name);

    // Check to see whether the current path is a valid file path
    // If so, execute this program at this file path
    int outcome = access(completePath, F_OK);
    if (outcome != -1) {
      return execv(completePath, argList);
    }

    token = strtok(NULL, ":");

  }

  return -1;
}

void processRedirect(int oldFile, int newFile) {
  dup2(oldFile, newFile);
  close(oldFile);
}


int startProgram(struct tokens * tokens) {

  int length = tokens_get_length(tokens);
  if (length == 0) {
    return -999;
  }

  int status = -999;
  int processID = fork();

  if (processID == 0) { // in child process

    char * argList[length + 1];

    int redirectInput = false;
    int redirectOutput = false; 
    int numArgs = 0;

    for (int j = 0; j < length; j++) {
      char * token = tokens_get_token(tokens, j);

      if (strcmp(token, ">") == 0) {
        redirectOutput = true;
      }

      else if (strcmp(token, "<") == 0) {
        redirectInput = true; 
      }

      else if (redirectInput) {
        processRedirect(open(token, O_RDONLY), STDIN_FILENO);
        redirectInput = false;
      }

      else if (redirectOutput) {
        processRedirect(creat(token,  S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH), STDIN_FILENO);
        redirectOutput = false;
      }

      else if (strcmp(token, "|") == 0) {
        char * precedingToken = tokens_get_token(tokens, j - 1);
        char * followingToken = tokens_get_token(tokens, j + 1);

        int p[2];

        p[0] = open(precedingToken, "r");
        p[1] = open(followingToken, "w");

        pipe(p);
      }

      else {
        argList[numArgs++] = token;
      }
    }

    char * programName = argList[0];
    argList[numArgs] = '\0';

    setpgid(0, 0);
    tcsetpgrp(shell_terminal, getpgrp());

    for (int k = 0; k < 7; k++) {
      signal(ignoreSignals[k], SIG_DFL);
    }

    int outcome = execv(programName, argList);

    if (outcome == -1) {
      locateProgramPath(programName, argList);
    }
    
  } 

  // We are in the parent process
  else {  
    waitpid(processID, &status, WUNTRACED);
    tcsetpgrp(shell_terminal, shell_pgid);
  }

  return status;
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

  for (int k = 0; k < 7; k++) {
    signal(ignoreSignals[k], SIG_IGN);
  }
}

int main(unused int argc, unused char *argv[]) {
  init_shell();

  static char line[4096];
  int line_num = 0;

  /* Please only print shell prompts when standard input is not a tty */
  if (shell_is_interactive)
    fprintf(stdout, "%d: ", line_num);

  while (fgets(line, 4096, stdin)) {
    /* Split our line into words. */
    struct tokens *tokens = tokenize(line);

    /* Find which built-in function to run. */
    int fundex = lookup(tokens_get_token(tokens, 0));

    if (fundex >= 0) {
      cmd_table[fundex].fun(tokens);
    } else {

      // We want to actually run the program
      startProgram(tokens);
    }

    if (shell_is_interactive)
      /* Please only print shell prompts when standard input is not a tty */
      fprintf(stdout, "%d: ", ++line_num);

    /* Clean up memory */
    tokens_destroy(tokens);
  }

  return 0;
}