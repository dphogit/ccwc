#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct options {
  bool count_bytes;
  bool count_lines;
  bool count_words;
};

typedef struct options Options;

bool nooptsset(Options options) {
  return !options.count_bytes && !options.count_lines && !options.count_words;
}

void setallopts(Options *options) {
  options->count_bytes = true;
  options->count_lines = true;
  options->count_words = true;
}

unsigned long countbytes(FILE *fp) {
  if (fseek(fp, 0, SEEK_END) || ferror(fp)) {
    perror("fseek");
    fclose(fp);
    exit(EXIT_FAILURE);
  }

  long bytes = ftell(fp);
  if (bytes == -1L) {
    perror("fseek");
    fclose(fp);
    exit(EXIT_FAILURE);
  }

  return bytes;
}

unsigned long countlines(FILE *fp) {
  if (fseek(fp, 0, SEEK_SET) || ferror(fp)) {
    perror("fseek");
    fclose(fp);
    exit(EXIT_FAILURE);
  }

  int lines = 0;
  char ch;

  while ((ch = fgetc(fp)) != EOF) {
    if (ch == '\n') {
      lines++;
    }
  }

  return lines;
}

unsigned long countwords(FILE *fp) {
  if (fseek(fp, 0, SEEK_SET) || ferror(fp)) {
    perror("fseek");
    fclose(fp);
    exit(EXIT_FAILURE);
  }

  int words = 0;
  char ch;
  bool in_word;

  while ((ch = fgetc(fp)) != EOF) {
    if (isspace(ch)) {
      if (in_word) {
        words++;
      }
      in_word = false;
    } else if (!in_word) {
      in_word = true;
    }
  }

  return words;
}

int main(int argc, char *argv[]) {
  int opt;
  Options options = {false, false, false};

  while ((opt = getopt(argc, argv, "clw")) != -1) {
    switch (opt) {
    case 'c':
      options.count_bytes = true;
      break;
    case 'l':
      options.count_lines = true;
      break;
    case 'w':
      options.count_words = true;
      break;
    }
  }

  if (optind == argc) {
    fprintf(stderr, "Error: No filename provided\n");
    exit(EXIT_FAILURE);
  }

  // Default behaviour is to set on all options (when all are not set)
  if (nooptsset(options)) {
    setallopts(&options);
  }

  char *filename = argv[optind];
  FILE *fp = fopen(filename, "rb");
  if (fp == NULL) {
    perror("Could not open file");
    exit(EXIT_FAILURE);
  }

  if (options.count_bytes) {
    printf("%li ", countbytes(fp));
  }

  if (options.count_lines) {
    printf("%li ", countlines(fp));
  }

  if (options.count_words) {
    printf("%li ", countwords(fp));
  }

  printf("%s\n", filename);
  fclose(fp);

  return 0;
}
