#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct options {
  bool count_bytes;
};

typedef struct options Options;

bool no_opts_set(Options options) { return options.count_bytes; }

void set_all_opts(Options options) { options.count_bytes = true; }

unsigned long countnbytes(FILE *fp) {
  if (fseek(fp, 0, SEEK_END) || ferror(fp)) {
    perror("Could not get file size.");
    fclose(fp);
    exit(EXIT_FAILURE);
  }

  long bytes = ftell(fp);
  if (bytes == -1L) {
    perror("Could not get file size.");
    fclose(fp);
    exit(EXIT_FAILURE);
  }

  return bytes;
}

int main(int argc, char *argv[]) {
  int opt;
  Options options;

  while ((opt = getopt(argc, argv, "c")) != -1) {
    switch (opt) {
    case 'c':
      options.count_bytes = true;
      break;
    }
  }

  if (optind == argc) {
    fprintf(stderr, "Error: No filename provided\n");
    exit(EXIT_FAILURE);
  }

  // Default behaviour is to set on all options (when all are not set)
  if (no_opts_set(options)) {
    set_all_opts(options);
  }

  char *filename = argv[optind];
  FILE *fp = fopen(filename, "rb");
  if (fp == NULL) {
    perror("Could not open file");
    exit(EXIT_FAILURE);
  }

  if (options.count_bytes) {
    unsigned long bytes = countnbytes(fp);
    printf("%6li ", bytes);
  }

  printf("%s\n", filename);
  fclose(fp);

  return 0;
}
