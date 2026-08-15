#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "user/user.h"

char *basename(char *path)
{
  char *base = path;
  char *p;

  for (p = path; *p; p++) {
    if (*p == '/')
      base = p + 1;
  }
  return base;
}

void find(char *path, char *target)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if ((fd = open(path, 0)) < 0) {
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  if (fstat(fd, &st) < 0) {
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  if (st.type == T_FILE) {
    if (strcmp(basename(path), target) == 0)
      printf("%s\n", path);
    close(fd);
    return;
  }

  if (st.type != T_DIR) {
    close(fd);
    return;
  }

  if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
    fprintf(2, "find: path too long\n");
    close(fd);
    return;
  }

  strcpy(buf, path);
  p = buf + strlen(buf);
  *p++ = '/';

  while (read(fd, &de, sizeof(de)) == sizeof(de)) {
    if (de.inum == 0)
      continue;
    if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
      continue;
    strcpy(p, de.name);
    find(buf, target);
  }
  close(fd);
}

int main(int argc, char *argv[])
{
  if (argc != 3) {
    fprintf(2, "usage: find <dir> <file>\n");
    exit(1);
  }
  find(argv[1], argv[2]);
  exit(0);
}
