#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int printable(const char* s) {
  if (!*s)
    return 0;
  while (*s != '\0') {
    if (!isspace((unsigned char) *s) && *s != '?')
      return 1;
    s++;
  }
  return 0;
}

int escape(char* dst, int n, const char* src) {
  int nwritten = 0;
  int l = strlen(src) + 1;
  char esc_char[] = {'\a','\b','\f','\n','\r','\t','\v','\\', ' '};
  char essc_str[] = { 'a', 'b', 'f', 'n', 'r', 't', 'v','\\', ' '};
  for (int i = 0; i < l; ++i) {
    int j;
    for (j = 0; j < sizeof(esc_char); j++) {
      if (src[i] == esc_char[j]) {
        if (nwritten < n) {
          *dst++ = '\\';
          ++nwritten;
        }
        if (nwritten < n) {
          *dst++ = essc_str[j];
          ++nwritten;
        }
        break;
      }
    }
    if (j == sizeof(esc_char) && nwritten < n)
      *dst++ = src[i];
      ++nwritten;
  }
  return nwritten;
}

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

int main() {
  char* program = "/usr/local/bin/chunkc";
  char* args[5];
  args[0] = "chunkc";
  args[1] = "tiling::query";
  args[2] = "--desktop";
  args[3] = "id";
  args[4] = NULL;
  char buf[1024];
  int r = chunkwm_query(buf, sizeof(buf), program, args);
  if (r) {
    fprintf(stderr, "%s\n", buf);
    return r;
  }
  printf("%s", buf);

  args[2] = "--desktop";
  args[3] = "mode";
  r = chunkwm_query(buf, sizeof(buf), program, args);
  if (r) {
    fprintf(stderr, "%s\n", buf);
    return r;
  }
  printf(" | %s", buf);

  args[2] = "--window";
  args[3] = "owner";
  r = chunkwm_query(buf, sizeof(buf), program, args);
  if (r) {
    fprintf(stderr, "%s\n", buf);
    return r;
  }
  if (printable(buf)) 
    printf(" | %s", buf);

  args[2] = "--window";
  args[3] = "name";
  r = chunkwm_query(buf, sizeof(buf), program, args);
  if (r) {
    fprintf(stderr, "%s\n", buf);
    return r;
  }
  if (printable(buf)) {
    printf(" | %s", buf);
    // printf(" | ");
    // for (char* c = buf; *c; ++c) {
    //   printf("%2x", *c);
    // }
  }

  return 0;
}
