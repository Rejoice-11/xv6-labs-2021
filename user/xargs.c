#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "kernel/param.h"
#include "user/user.h"

void run(char *cmd, char *args[])
{
  if (fork() == 0) {
    exec(cmd, args);
    fprintf(2, "xargs: exec %s failed\n", cmd);
    exit(1);
  }
  wait(0);
}

int main(int argc, char *argv[])
{
  char *cmd, *args[MAXARG], line[512], *p;
  int n, i, len;
  char c;

  if (argc < 2) {
    fprintf(2, "usage: xargs command [args...]\n");
    exit(1);
  }

  cmd = argv[1];
  n = 0;
  for (i = 1; i < argc && n < MAXARG - 1; i++)
    args[n++] = argv[i];
  args[n] = 0;

  len = 0;
  while (read(0, &c, 1) == 1) {
    if (c == '\n') {
      line[len] = 0;
      p = line;
      while (*p && n < MAXARG - 1) {
        while (*p == ' ' || *p == '\t')
          p++;
        if (*p == 0)
          break;
        args[n++] = p;
        while (*p && *p != ' ' && *p != '\t')
          p++;
        if (*p)
          *p++ = 0;
      }
      args[n] = 0;
      run(cmd, args);
      n = argc - 1;
      args[n] = 0;
      len = 0;
    } else if (len < sizeof(line) - 1) {
      line[len++] = c;
    }
  }

  exit(0);
}
