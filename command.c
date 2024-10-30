#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

#define arg_size 100
#define buffer_size 1024

char *command(char *cmd, FILE *in)
{
  char *final_cmd;
  size_t cmd_len = strlen(cmd);

  if (cmd[0] == '.') // relative path
  {
    char *cwd = getcwd(NULL, 0);
    final_cmd = malloc(strlen(cwd) + cmd_len);
    strcpy(final_cmd, cwd);
    strcat(final_cmd, cmd + 1);
    free(cwd);
  }
  else if (cmd[0] == '/') // absolute path
  {
    final_cmd = cmd;
  }
  else // search in PATH
  {
    char *path = getenv("PATH");
    char *path_copy = malloc(strlen(path) + 1);
    strcpy(path_copy, path);

    char *token = strtok(path_copy, ":");
    while (token != NULL)
    {
      final_cmd = malloc(strlen(token) + cmd_len + 2);
      strcpy(final_cmd, token);
      strcat(final_cmd, "/");
      strcat(final_cmd, cmd);

      if (access(final_cmd, X_OK) == 0)
      {
        break;
      }

      free(final_cmd);
      final_cmd = NULL;
      token = strtok(NULL, ":");
    }
  }

  char *token = strtok(final_cmd, " "); // split command and arguments
  char *args[arg_size]; // max 100 arguments

  int i = 0;
  while (token != NULL && i < arg_size - 1)
  {
    args[i++] = token; // store command and arguments
    token = strtok(NULL, " "); // get next token
  }

  if (in != NULL) { // read input file
    char file_buffer[buffer_size]; // max 1024 characters per line
    while (fgets(file_buffer, sizeof(file_buffer), in) != NULL && i < arg_size - 1) {
      file_buffer[strcspn(file_buffer, "\n")] = 0;
      args[i++] = file_buffer;
    }
  }
  args[i] = NULL;

  int pipefd[2];
  pipe(pipefd);

  pid_t pid = fork(); // create child process to execute command

  if (pid == 0)
  {
    close(pipefd[0]);
    dup2(pipefd[1], STDOUT_FILENO);
    dup2(pipefd[1], STDERR_FILENO);
    close(pipefd[1]);

    execve(args[0], args, NULL);
  }

  else
  {
    // once the child process is done, the parent process reads output
    close(pipefd[1]);

    char buffer[buffer_size];
    ssize_t count;
    size_t total_size = 1;
    char *output = malloc(total_size);
    output[0] = '\0';

    while ((count = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) // read output
    {
      buffer[count] = '\0';
      total_size += count;
      char *new_output = realloc(output, total_size);
      output = new_output;
      strcat(output, buffer);
    }
    close(pipefd[0]); // close pipe

    waitpid(pid, NULL, 0); // wait for child process to finish

    free(final_cmd); // free memory
    return output;
  }
}
