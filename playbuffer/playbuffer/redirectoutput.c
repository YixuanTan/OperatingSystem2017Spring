#include <stdio.h>
#include <unistd.h>
int main() {
  close(2);
  fd = open("newfile.txt", O_WRONLY | O_CREAT | O_TRUNC, 0600);
  fprintf(stderr, "Hello\n");
  sleep(10);
  return 0;
}
