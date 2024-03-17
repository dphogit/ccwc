#include <ctype.h>
#include <locale.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wchar.h>

const int INIT_BUFSIZE = 1024;

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

bool nooptsset(Options options) {
  return !options.print_bytes && !options.print_lines && !options.print_words &&
         !options.print_chars;
}

void setdefaultopts(Options *options) {
  options->print_bytes = true;
  options->print_lines = true;
  options->print_words = true;
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

  // Use default options (lines, words, bytes) if no options are passed.
  if (nooptsset(options)) {
    setdefaultopts(&options);
  }

  // Use current environment's default locale for character handling functions
  setlocale(LC_CTYPE, "");

  FILE *stream = NULL;
  char *filename = NULL;

  char *buffer = malloc(sizeof(char) * INIT_BUFSIZE);

  if (buffer == NULL) {
    fprintf(stderr, "Error allocating memory\n");
    exit(EXIT_FAILURE);
  }

  // The stream we use to load in the buffer depends on whether a filename is
  // passed or not. If not, we use stdin (e.g. redirection or piped).
  if (optind == argc) {
    stream = stdin;
  } else {
    filename = argv[optind];
    stream = fopen(filename, "rb");
    if (stream == NULL) {
      perror("fopen");
      exit(EXIT_FAILURE);
    }
  }

  size_t buffer_size = INIT_BUFSIZE;
  size_t total_bytes = 0;
  size_t total_lines = 0;
  size_t total_words = 0;

  int ch;
  bool in_word;

  // Load the buffer copying each character from stream.
  while ((ch = fgetc(stream)) != EOF) {
    // Check if we need to increase buffer size => reallocate with double size
    if (total_bytes + 1 >= buffer_size) {
      buffer_size *= 2;
      buffer = realloc(buffer, buffer_size * sizeof(char));
      if (buffer == NULL) {
        fprintf(stderr, "Error reallocating memory\n");
        free(buffer);
        exit(EXIT_FAILURE);
      }
    }

    if (ch == '\n') {
      total_lines++;
    }

    // Words are counted through (non) whitespace detection as well as knowing
    // whether we are currently in the middle of a word or not.
    if (isspace(ch)) {
      if (in_word) {
        total_words++;
      }
      in_word = false;
    } else if (!in_word) {
      in_word = true;
    }

    buffer[total_bytes] = ch;
    total_bytes++;
  }

  buffer[total_bytes] = '\0';

  if (options.print_lines) {
    printf("%ju ", total_lines);
  }
  if (options.print_words) {
    printf("%ju ", total_words);
  }
  if (options.print_bytes) {
    printf("%ju ", total_bytes);
  }

  if (filename != NULL) {
    printf("%s\n", filename);
  }

  free(buffer);

  return 0;
}
