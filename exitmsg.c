#include <stdio.h>
#include <stdlib.h>

void usermsg_exit(const char *msg, const char *detail) {
  fputs(msg, stderr);
  fputs(": ", stderr);
  fputs(detail, stderr);
  fputc('\n', stderr);
  exit(1);
}

void sysmsg_exit(const char *msg) {
  perror(msg);
  exit(1);
}
