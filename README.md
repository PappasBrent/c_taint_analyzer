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

### Building the Clang plugin

1. Configure the plugin:

    ```bash
    cmake \
        -DCMAKE_C_COMPILER=/usr/bin/clang-17 \
        -DCMAKE_CXX_COMPILER=/usr/bin/clang++-17 \
        -DClang_DIR=/usr/lib/cmake/clang-17 \
        -DLLVM_DIR=/usr/include/llvm-17 \
        -B build \
        .
    ```

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

To run the taint analyzer and unparsers' tests, first [build the
project](#building-the-clang-plugin). Then navigate into the `build/test`
directory and run `ctest` to run the tests:

```bash
cd build/test
ctest
```

Our test scripts assume that you have `ctest`, `bash`, and `diff` installed and
that your PATH environment variable points to them.
