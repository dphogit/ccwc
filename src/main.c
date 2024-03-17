#include <locale.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wchar.h>
#include <wctype.h>

const int MIN_NUMBER_WIDTH = 6;

struct options {
  bool print_bytes;
  bool print_newlines;
  bool print_words;
  bool print_chars;
};

typedef struct options Options;

bool nooptsset(Options options) {
  return !options.print_bytes && !options.print_newlines &&
         !options.print_words && !options.print_chars;
}

void setdefaultopts(Options *options) {
  options->print_bytes = true;
  options->print_newlines = true;
  options->print_words = true;
}

// Return a suitable width for non-negative integer n for printing the counts.
// Returns the minimum between number of digits and a predefined minimum.
int getnumberwidth(uintmax_t n) {
  if (n < 0) {
    fprintf(stderr, "Must be non-negative value!");
  } else if (n == 0) {
    return 1;
  }

  int width = 0;
  while (n > 0) {
    width++;
    n /= 10;
  }
  return width > MIN_NUMBER_WIDTH ? width : MIN_NUMBER_WIDTH;
}

void printcount(uintmax_t count) {
  printf("%*zu ", getnumberwidth(count), count);
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
      options.print_newlines = true;
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

  // The stream we use to process depends on whether a filename is
  // passed or not. If passed we use the file contents, otherwise
  // we use stdin (e.g. redirection or piped).
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

  if (options.print_newlines) {
    printcount(total_newlines);
  }
  if (options.print_words) {
    printcount(total_words);
  }
  if (options.print_chars) {
    printcount(total_chars);
  }
  if (options.print_bytes) {
    printcount(total_bytes);
  }

  if (filename != NULL) {
    printf("%s", filename);
  }
  printf("\n");

  return 0;
}
