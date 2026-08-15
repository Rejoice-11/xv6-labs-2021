#include "kernel/types.h"
#include "user/user.h"

void sieve(int left[])
{
  int prime, n, right[2];

  if (read(left[0], &prime, sizeof(prime)) != sizeof(prime)) {
    close(left[0]);
    exit(0);
  }
  printf("prime %d\n", prime);

  pipe(right);
  if (fork() == 0) {
    close(left[0]);
    close(right[1]);
    sieve(right);
    exit(0);
  }

  close(right[0]);
  while (read(left[0], &n, sizeof(n)) == sizeof(n)) {
    if (n % prime != 0)
      write(right[1], &n, sizeof(n));
  }
  close(left[0]);
  close(right[1]);
  wait(0);
}

int main(void)
{
  int p[2], i;

  pipe(p);
  if (fork() == 0) {
    close(p[1]);
    sieve(p);
    exit(0);
  }

  close(p[0]);
  for (i = 2; i <= 35; i++)
    write(p[1], &i, sizeof(i));
  close(p[1]);
  wait(0);
  exit(0);
}
