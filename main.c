#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wchar.h>
#include <wctype.h>

struct options {
  bool print_bytes;
  bool print_lines;
  bool print_words;
  bool print_chars;
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

  size_t total_bytes = 0;
  size_t total_newlines = 0;
  size_t total_words = 0;
  size_t total_chars = 0;

  wint_t wc;
  char mbbuffer[MB_CUR_MAX]; // Buffer to hold converted multibyte character
  bool in_word;

  // Iterate through each wide character from the stream.
  // Using wide characters allow us to handle chars that are multibyte.
  while ((wc = fgetwc(stream)) != WEOF) {
    total_chars++;

    // Get byte count of character (handles multibyte characters)
    size_t bytes = wcrtomb(mbbuffer, wc, NULL);
    if (bytes == (size_t)-1) {
      fprintf(stderr, "Invalid character sequence: %lc\n", wc);
      exit(EXIT_FAILURE);
    }
    total_bytes += bytes;

    // Line counting
    if (wc == L'\n') {
      total_newlines++;
      in_word = false;
    }

    // Word counting
    if (iswspace(wc)) {
      if (in_word) {
        total_words++;
      }
      in_word = false;
    } else if (!in_word) {
      in_word = true;
    }
  }

  if (options.print_lines) {
    printf("%zu ", total_newlines);
  }
  if (options.print_words) {
    printf("%zu ", total_words);
  }
  if (options.print_bytes) {
    printf("%zu ", total_bytes);
  }
  if (options.print_chars) {
    printf("%zu ", total_chars);
  }

  if (filename != NULL) {
    printf("%s", filename);
  }
  printf("\n");

  return 0;
}
