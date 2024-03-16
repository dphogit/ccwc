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

WCResult wc(const char *filename) {
  FILE *fp = fopen(filename, "rb");
  if (fp == NULL) {
    perror("Could not open file");
    exit(EXIT_FAILURE);
  }

  WCResult result = {.lines = 0, .words = 0, .bytes = 0};

  char ch;
  bool in_word;

  while ((ch = fgetc(fp)) != EOF) {
    result.bytes++;

    if (ch == '\n') {
      result.lines++;
    }

    // If we detect a space and we are currently in a word, increment word
    // count. Also check to reset `in_word` when we encounter a new word (next
    // char is not space and we are not in a word currently)
    if (isspace(ch)) {
      if (in_word) {
        result.words++;
      }
      in_word = false;
    } else if (!in_word) {
      in_word = true;
    }
  }

  fclose(fp);
  return result;
}

// A separate function to count characters in a file. Considers wide characters
// that are larger than a byte, which the main `wc` function does not consider.
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

  if (optind == argc) {
    fprintf(stderr, "Error: No filename provided\n");
    exit(EXIT_FAILURE);
  }

  if (nooptsset(options)) {
    setdefaultopts(&options);
  }

  char *filename = argv[optind];

  // Use current environment's default locale for character handling functions
  setlocale(LC_CTYPE, "");

  WCResult result = wc(filename);

  if (options.print_lines) {
    printf("%ju ", result.lines);
  }
  if (options.print_words) {
    printf("%ju ", result.words);
  }
  if (options.print_bytes) {
    printf("%ju ", result.bytes);
  }
  if (options.print_chars) {
    printf("%ju ", countchars(filename));
  }

  printf("%s\n", filename);

  return 0;
}
