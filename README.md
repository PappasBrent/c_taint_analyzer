# COP 5021 Course Project

Authors: Arthur Amorim, Robnison Ferrer, and Brent Pappas

Each person actively contributed to and fully understands the solution.

A Clang plugin for C89 intra-procedural taint analysis.

<!--
See
    https://clang.llvm.org/docs/ClangPlugins.html
    https://github.com/rizsotto/Constantine
    https://github.com/appleseedlab/macro-analyzer
for instructions and examples for writing Clang plugins
-->

## Quickstart

### Download prerequisites

The following instructions assume an Ubuntu 22.04 LTS operating system:

- Make

  ```bash
  sudo apt install make
  ```

- CMake 3.28.1 (click [here](https://apt.kitware.com/) for download and install
  instructions)

- [LLVM 17 and Clang 17](https://apt.llvm.org/):

  ```bash
  wget -qO- https://apt.llvm.org/llvm-snapshot.gpg.key | sudo tee /etc/apt/trusted.gpg.d/apt.llvm.org.asc
  sudo apt install llvm-17 clang-17 libclang-17-dev
  ```

- [LLVM Lit](https://llvm.org/docs/CommandGuide/lit.html)

  ```bash
  https://pypi.org/project/lit
  ```

  After installing `lit`, change your  `CMakeLists.txt` to point to your
  installation of `lit`. For example, if you installed `lit` to
  `/home/me/.local/lit`, then you would change the following line of
  `CMakeLists.txt`:

  ```txt
  "LLVM_EXTERNAL_LIT": "/path/to/lit"
  ```

  to

  ```txt
  "LLVM_EXTERNAL_LIT": "/home/me/.local/lit"
  ```

### Building the Clang plugin

1. Configure the plugin:

    ```bash
    cmake \
        -DCMAKE_C_COMPILER=/usr/bin/clang-17 \
        -DCMAKE_CXX_COMPILER=/usr/bin/clang++-17 \
        -DClang_DIR=/usr/lib/cmake/clang-17 \
        -DLLVM_DIR=/usr/include/llvm-17 \
        -DLLVM_EXTERNAL_LIT=/path/to/lit \
        -B build \
        .
    ```

    where `/path/to/lit` is the path to your installation of LLVM `lit`.

    or (using
    [`CMakePresets.json`](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html))

    ```bash
    cmake --preset configure-make-debug
    ```

1. Build the plugin:

    ```bash
    cmake --build build
    ```

    or (using `CMakePresets.json`)

    ```bash
    cmake --build --preset build-make-debug
    ```

## Usage

### Unparser

To run the unparser on `some/program.c`, run Clang and load the plugin as
follows:

```bash
clang-17 -fplugin=build/lib/libc_taint.so -fsyntax-only -Xclang -plugin-arg-c_taint_analyzer -Xclang --unparse some/program.c
```

## Taint analyzer

To run the taint analyzer on `some/program.c`, run Clang and load the plugin as
follows:

```bash
clang-17 -fplugin=build/lib/libc_taint.so -fsyntax-only some/program.c
```

### Taint analyzer arguments

By default, the taint analyzer will only print the program's entry and exit
functions. You can modify this output with the following arguments:

| Argument               | Description                                    |
|------------------------|------------------------------------------------|
| `--print-block-labels` | Print blocks and their labels.                 |
| `--print-init`         | Print blocks init labels.                      |
| `--print-finals`       | Print block final labels.                      |
| `--print-flow`         | Print block flows.                             |
| `--print-kill`         | Print each block's set of killed variables.    |
| `--print-gen`          | Print each block's set of generated variables. |
| `--print-entry`        | Print the entry function for each block.       |
| `--print-exit`         | Print the exit function for each block.        |

For example:

```bash
clang-17 -fplugin=build/lib/libc_taint.so -fsyntax-only some/program.c -Xclang -plugin-arg-c_taint_analyzer -Xclang --print-block-labels
```

The above command tells the taint analyzer to print the program's blocks and
labels as well.

You can also prepend an argument with `no` to tell the taint analyzer not to
print a field in the output. For example:

```bash
clang-17 -fplugin=build/lib/libc_taint.so -fsyntax-only some/program.c -Xclang -plugin-arg-c_taint_analyzer -Xclang --no-print-entry
```

The above command tells the taint analyzer not to print the entry function for
each block.

## Development

We recommend downloading the [`clangd`](https://clangd.llvm.org/installation)
C/C++ language server to ease development.

This project also provides a `.clang-format` file for formatting. If you would
like to contribute, please ensure that you've formatted your code with
[`clang-format`](https://clang.llvm.org/docs/ClangFormat.html) before committing
it. Assuming you are on Ubuntu, you can download `clang-format` with the
following command:

```bash
sudo apt install clang-format
```

## Testing

Please follow the instructions listed in the [Development](#development) section
to install `clang-format` before running the project's tests, because the
unparser tests rely on it to avoid accidental failures due to whitespace.

To run the taint analyzer and unparsers' tests, first configure the project,
then build the its `check-c-taint` target like so:

```bash
cmake --build build -t check-c-taint
```
