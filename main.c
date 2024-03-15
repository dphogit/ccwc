#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  FILE *fp = fopen("test.txt", "rb");
  if (fp == NULL) {
    perror("Could not open file");
    exit(1);
  }

  int failed_seek = fseek(fp, 0, SEEK_END);
  if (failed_seek || ferror(fp)) {
    perror("Could not get file size.");
    exit(1);
  }

  long bytes = ftell(fp);
  if (bytes == -1L) {
    perror("Could not get file size.");
    exit(1);
  }

  printf("%6li %s\n", bytes, "test.txt");

  return 0;
}
