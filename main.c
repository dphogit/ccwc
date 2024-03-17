#include <ctype.h>
#include <locale.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wchar.h>

struct options {
  bool print_bytes;
  bool print_lines;
  bool print_words;
  bool print_chars;
};

struct wcresult {
  uintmax_t lines;
  uintmax_t words;
  uintmax_t bytes;
};

typedef struct options Options;
typedef struct wcresult WCResult;

bool nooptsset(Options options) {
  return !options.print_bytes && !options.print_lines && !options.print_words &&
         !options.print_chars;
}

void setdefaultopts(Options *options) {
  options->print_bytes = true;
  options->print_lines = true;
  options->print_words = true;
}

WCResult wc(const char *buffer) {
  WCResult result = {.lines = 0, .words = 0, .bytes = 0};

  bool in_word;
  size_t i = 0;

  while (buffer[i] != '\0') {
    result.bytes++;

    if (buffer[i] == '\n') {
      result.lines++;
    }

    // If we detect a space and we are currently in a word, increment word
    // count. Also check to reset `in_word` when we encounter a new word (next
    // char is not space and we are not in a word currently)
    if (isspace(buffer[i])) {
      if (in_word) {
        result.words++;
      }
      in_word = false;
    } else if (!in_word) {
      in_word = true;
    }

    i++;
  }

  return result;
}

// A separate function to count characters in a file. Considers wide characters
// that are larger than a byte, which the main `wc` function does not consider.
// TODO Integrate this with the new buffer implementation when ready.
uintmax_t countchars(const char *filename) {
  FILE *fp = fopen(filename, "rb");
  if (fp == NULL) {
    perror("Could not open file");
    exit(EXIT_FAILURE);
  }
  uintmax_t chars = 0;
  wchar_t ch;

  while ((ch = fgetwc(fp)) != WEOF) {
    chars++;
  }

  fclose(fp);
  return chars;
}

int main(int argc, char *argv[]) {
  int opt;
  Options options = {false, false, false, false};

  while ((opt = getopt(argc, argv, "lwcm")) != -1) {
    switch (opt) {
    case 'c':
      options.print_bytes = true;
      break;
    case 'l':
      options.print_lines = true;
      break;
    case 'w':
      options.print_words = true;
      break;
    case 'm':
      options.print_chars = true;
    }
  }

  // Use the default options of lines, words, and byte counts if no option
  // is explicitly passed.
  if (nooptsset(options)) {
    setdefaultopts(&options);
  }

  FILE *pFile = NULL;
  char *filename = NULL;
  char *buffer = NULL;

  if (optind == argc) {
    // No filename provided, read contents of stdin
    buffer = malloc(sizeof(char) * 1024);
    if (buffer == NULL) {
      perror("malloc");
      exit(EXIT_FAILURE);
    }

    size_t buffer_size = 1024;
    size_t bytes_read = 0;
    int c;

    while ((c = fgetc(stdin)) != EOF) {
      // Check if buffer needs resizing
      if (bytes_read + 1 >= buffer_size) {
        buffer_size *= 2; // double buffer size
        buffer = realloc(buffer, buffer_size * sizeof(char));
        if (buffer == NULL) {
          fprintf(stderr, "Error reallocating memory\n");
          free(buffer);
          exit(EXIT_FAILURE);
        }
      }

      buffer[bytes_read] = c;
      bytes_read++;
    }

    buffer[bytes_read] = '\0';
  } else {
    // Filename provided, open the file.
    filename = argv[optind];
    pFile = fopen(filename, "rb");
    if (pFile == NULL) {
      perror("fopen");
      exit(EXIT_FAILURE);
    }
    // Move file position to end of file so we can get it's size
    if (fseek(pFile, 0, SEEK_END)) {
      perror("fseek SEEK_END");
      exit(EXIT_FAILURE);
    }

    // Get file size
    long size = ftell(pFile);
    if (size == -1L) {
      perror("ftell");
      exit(EXIT_FAILURE);
    }

    // Move file position back to start of file for reading.
    if (fseek(pFile, 0, SEEK_SET)) {
      perror("fseek SEEK_SET");
      exit(EXIT_FAILURE);
    }

    // Allocate memory for a buffer to contain whole file
    buffer = (char *)malloc(sizeof(char) * size);
    if (buffer == NULL) {
      perror("malloc");
      exit(EXIT_FAILURE);
    }

    // Copy file contents into buffer
    size_t res = fread(buffer, sizeof(char), size, pFile);
    if (res != size) {
      fprintf(stderr, "Error reading file\n");
      exit(EXIT_FAILURE);
    }
  }

  // Use current environment's default locale for character handling functions
  setlocale(LC_CTYPE, "");

  WCResult result = wc(buffer);

  if (options.print_lines) {
    printf("%ju ", result.lines);
  }
  if (options.print_words) {
    printf("%ju ", result.words);
  }
  if (options.print_bytes) {
    printf("%ju ", result.bytes);
  }

  // if (options.print_chars) {
  //   printf("%ju ", countchars(filename));
  // }

  if (filename == NULL) {
    printf("\n");
  } else {
    printf("%s\n", filename);
  }

  free(buffer);

  return 0;
}
