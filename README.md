# ccwc

[Coding challenge]("https://codingchallenges.fyi/challenges/challenge-wc") tool build our own version of the Unix wc command line tool. Read more about wc on the [man page](https://linux.die.net/man/1/wc).

I wanted to do this simple challenge because I wanted to develop in C and try Neovim as my development environment (I'm liking it!).

Things I learned:
- Vim/Neovim environments (NvChad, Lua configs, plugins, LSP, keybindings etc.)
- C (File operations, streams, `getopt`, character functions, locales)
- C tooling (Make/Makefile, clangd, clang-format)

## Build and Usage

Uses a `Makefile` to build into `bin/ccwc`. Simply run `make` to create the `ccwc` executable located in `bin`. Then run with: 

`bin/ccwc [OPTION]... [FILE]`

Prints the word, line, character, and byte count for FILE. If no FILE, reads from standard input.

| Option | Description |
|---|---|
| `-c` | print byte counts |
| `-m` | print character counts |
| `-l` | print newline counts |
| `-w` | print word counts |

If no options are passed, will by default print word (`-w`), newlines (`-l`) 
and byte (`-c`) counts.

## Examples

A `test.txt` file is given (provided by the challenge).

```bash
# Providing [FILE]
>bin/ccwc test.txt
    7145   58164  342190 test.txt
```

```bash
# No [FILE], read from stdin
>bin/ccwc test.txt | ccwc -l
    7145
```

