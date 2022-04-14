#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <string.h>

#define DEFAULT_ARG_LENGTH 1028
#define TOKEN_DELIM " \t\r\n\a"

int shell_cd(char** args);
int shell_exit(char** args);

char* function_names[] = {
  "cd",
  "exit"
};

int (*defined_functions[]) (char **) = {
  &shell_cd,
  &shell_exit
};

char* read_line() {
  int buffsize = DEFAULT_ARG_LENGTH;
  int pos = 0;
  char* buffer = malloc(sizeof(char)*buffsize);
  char c;

  if(!buffer) {
    fprintf(stderr, "shell: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while(1) {
    c = getchar();

    if(c == EOF || c == '\n') {
      buffer[pos] = '\0';
      return buffer;
    } else {
      buffer[pos] = c;
    }
    pos++;

    if(pos >= buffsize) {
      buffsize += DEFAULT_ARG_LENGTH;
      buffer = realloc(buffer, buffsize);
      if(!buffer) {
        fprintf(stderr, "shell: reallocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

char** parse_line(char* line) {
  int buffsize = DEFAULT_ARG_LENGTH;
  char** tokens = malloc(buffsize * sizeof(char*));
  char* token;
  int pos = 0;

  if(!tokens) {
    fprintf(stderr, "shell: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, TOKEN_DELIM);
  while(token != NULL) {
    tokens[pos] = token;
    token = strtok(NULL, TOKEN_DELIM);

    pos++;
    if(pos >= buffsize) {
      buffsize += DEFAULT_ARG_LENGTH;
      tokens = realloc(tokens, buffsize * sizeof(char*));
      if(!tokens) {
        fprintf(stderr, "shell: reallocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
  tokens[pos] = NULL;
  return tokens;
}

//This handles commands that need to be executed within the shell
int shell_launch(char** args) {
  pid_t pid, wpid;
  int status;

  pid = fork();
  if(pid == 0) {
    //Exec never returns a value unless it fails
    if(execvp(args[0], args) == -1) {
      perror("shell");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    perror("lsh");
  } else {
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while(!WIFEXITED(status) && !WIFSIGNALED(status));
  }
  return 1;
}


int shell_cd(char** args) {
  if(args[1] == NULL) {
    fprintf(stderr, "Expected an argument after \"cd\"\n");
  }else {
    if(chdir(args[1])==-1) {
      perror("shell");
    }
  }
  return 1;
}

int shell_exit(char** args) {
  return 0;
}

int exec_args(char** args) {
  if(args[0] == NULL) {
    return 1;
  }

  for(int i = 0; i < 2; i++) {
    if(strcmp(args[0], function_names[i])==0) {
      return (*defined_functions[i])(args);
    }
  }
  return shell_launch(args);

}

void shell_intro() {
  printf("Shell Implementation by Colby Wang\n");
  printf("Use at your own risk!\n\n");
}


void shell_loop() {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
  shell_intro();
  char* input;
  char** args;
  int status;
  do {
    printf("> ");
    input = read_line();
    args = parse_line(input);
    status = exec_args(args);
  } while(status != 0);
}


int main() {
  shell_loop();
  return 0;
}
