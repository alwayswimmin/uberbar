#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int chunkwm_query(char* buf, int n, char* program, char** args) {
  int r;
  int pipe_fd[2];
  if ((r = pipe(pipe_fd))) {
    snprintf(buf, n, "Pipe failed.\n");
    return r;
  }
  pid_t pid;
  pid = fork();
  if (pid == 0) {
    if ((r = dup2(pipe_fd[1], STDOUT_FILENO)) < 0) {
      dprintf(pipe_fd[1], "Dup2 failed.\n");
      close(pipe_fd[0]);
      close(pipe_fd[1]);
      _exit(r);
    }
    close(pipe_fd[0]);
    close(pipe_fd[1]);
    r = execv(program, args);
    dprintf(pipe_fd[1], "Execv failed.\n");
    _exit(r);
  } else if (pid > 0) {
    close(pipe_fd[1]);
    while (n > 0) {
      r = read(pipe_fd[0], buf, n);
      if (r <= 0)
        break;
      if (n - r)
        buf[r] = 0;
      else
        buf[n - 1] = 0;
      buf += r;
      n -= r;
    }
    close(pipe_fd[0]);
    pid = waitpid(pid, &r, 0);
    return r;
  } else {
    snprintf(buf, n, "Fork failed.\n");
    return pid;
  }
}

int print_desktop_info(int desktop, int first) {
  char desktop_string[16];
  snprintf(desktop_string, sizeof(desktop_string), "%d", desktop);
  char* program = "/usr/local/bin/chunkc";
  char* args[5];
  args[0] = "chunkc";
  args[1] = "tiling::query";
  args[2] = "--windows-for-desktop";
  args[3] = desktop_string;
  args[4] = NULL;
  char buf[1024];
  int r = chunkwm_query(buf, sizeof(buf), program, args);
  if (r < 0) {
    fprintf(stderr, "%s\n", buf);
    return r;
  }
  r = 0;
  char* start = buf;
  for (char* b = buf; *b && b - buf < sizeof(buf); ++b) {
    if (*b == ',') {
      *b = '\0';
      if (start == buf) {
        start = b + 2;
      }
    } else if (*b == '\n') {
      *b = '\0';
      if (start == buf || start - buf >= sizeof(buf)) {
        continue;
      }
      if (!r) {
        if (first) {
          printf("%d", desktop);
        } else {
          printf(" || %d", desktop);
        }
      }
      printf(" | %s", start);
      ++r;
      start = buf;
    }
  }
  return r;
}

int main() {
  char* program = "/usr/local/bin/chunkc";
  char* args[5];
  args[0] = "chunkc";
  args[1] = "tiling::query";
  args[2] = "--monitor";
  args[3] = "id";
  args[4] = NULL;
  char buf[1024];
  int r = chunkwm_query(buf, sizeof(buf), program, args);
  if (r) {
    fprintf(stderr, "%s\n", buf);
    return r;
  }
  args[2] = "--desktops-for-monitor";
  args[3] = buf;
  r = chunkwm_query(buf, sizeof(buf), program, args);
  if (r) {
    fprintf(stderr, "%s\n", buf);
    return r;
  }
  int first = 1;
  char* start = buf;
  for (char* b = buf; b - buf < sizeof(buf); ++b) {
    if (*b == ' ' || *b == '\0') {
      int desktop;
      r = sscanf(start, "%d", &desktop);
      if (!r)
        continue;
      start = b;
      r = print_desktop_info(desktop, first);
      if (r > 0) {
        first = 0;
      }
    }
    if (!*b) {
      break;
    }
  }
  if (!first) {
    printf("\n");
  }
  return 0;
}
