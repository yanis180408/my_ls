# my_ls

A from-scratch re-implementation of the Unix `ls` command in C. It lists the contents of the current directory, supports the `-a` and `-t` options (including grouped flags such as `-at`), and prints entries in green.

## Features

- **Default listing**: entries of the current directory sorted alphabetically, one per line.
- **`-a`**: include hidden entries (names starting with `.`, including `.` and `..`).
- **`-t`**: sort by last modification time, newest first (nanosecond precision). Entries with the same timestamp are sorted by name.
- **Grouped options**: `-at`, `-ta`, `-a -t`, etc. are all accepted.
- **Colored output**: every name is printed in light green.
- **Error handling**: unknown options, `opendir`, `lstat`, `malloc` and `realloc` failures are reported on `stderr`, memory is always freed, and the program exits with status `1`.

## Build

```bash
gcc -Wall -Wextra -Werror -o my_ls my_ls.c
```

> Replace `my_ls.c` with the name of your source file.

## Usage

```bash
./my_ls [-a] [-t]
```

| Command         | Result                                                   |
|-----------------|----------------------------------------------------------|
| `./my_ls`       | Visible files, alphabetical order                        |
| `./my_ls -a`    | All files, including hidden ones, alphabetical order     |
| `./my_ls -t`    | Visible files, newest first                              |
| `./my_ls -at`   | All files, newest first                                  |

Unknown option example:

```bash
$ ./my_ls -x
Unknown option: -x
```

## How it works

1. **Option parsing**: `argv` is walked once. Each argument starting with `-` is scanned character by character, which is what makes grouped flags like `-at` work.
2. **Reading the directory**: `opendir` / `readdir` iterate over the entries of `.`. Hidden entries are skipped unless `-a` is set.
3. **Storing entries**: each entry is stored in a dynamic array of `t_file` (name + `struct timespec` modification time). The array starts at 16 slots and doubles when full (`realloc`). Names are copied with `malloc`, since `readdir` reuses its buffer. The modification time comes from `lstat`, which does not follow symlinks.
4. **Sorting**: an insertion sort driven by a single `compare` function:
   - with `-t`: compare seconds, then nanoseconds (newest first);
   - on a tie, or without `-t`: compare names with `my_strcmp`.
5. **Printing and cleanup**: names are printed one per line, then every name and the array are freed in a single cleanup block shared by the success and error paths.

## Project structure

```
.
├── my_ls.c      # whole implementation
└── README.md
```

Helper functions written by hand:

| Function      | Role                                              |
|---------------|---------------------------------------------------|
| `my_strcmp`   | Compare two strings (replacement for `strcmp`)    |
| `my_strlen`   | Length of a string                                |
| `my_strcpy`   | Copy a string                                     |
| `compare`     | Ordering of two entries (name or time)            |
| `sort_files`  | Insertion sort over the entries array             |

Get the project :
git clone [REPOSITORY_URL]
cd my_ls

The Core Team
Made at Qwasar SV -- Software Engineering School <img alt='Qwasar SV -- Software Engineering School's Logo' src='https://storage.googleapis.com/qwasar-public/qwasar-logo_50x50.png' width='20px' />

## Limitations

- Only lists the **current directory**; path arguments are ignored.
- Only `-a` and `-t` are implemented (no `-l`, `-r`, `-R`, ...).
- Sorting is by byte value (`my_strcmp`), so uppercase letters come before lowercase ones, unlike `ls` in a locale-aware terminal.
- Relies on `st_mtim`, which is available on Linux but not on every Unix (macOS uses `st_mtimespec`).

## Author

Yanis Lakos


