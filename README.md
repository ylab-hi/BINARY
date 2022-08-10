[![Actions Status](https://github.com/ylab-hi/BINARY/workflows/MacOS/badge.svg)](https://github.com/ylab-hi/BINARY/actions)
[![Actions Status](https://github.com/ylab-hi/BINARY/workflows/Windows/badge.svg)](https://github.com/ylab-hi/BINARY/actions)
[![Actions Status](https://github.com/ylab-hi/BINARY/workflows/Ubuntu/badge.svg)](https://github.com/ylab-hi/BINARY/actions)
[![Actions Status](https://github.com/ylab-hi/BINARY/workflows/Style/badge.svg)](https://github.com/ylab-hi/BINARY/actions)
[![Actions Status](https://github.com/ylab-hi/BINARY/workflows/Install/badge.svg)](https://github.com/ylab-hi/BINARY/actions)
[![codecov](https://codecov.io/gh/TheLartians/ModernCppStarter/branch/master/graph/badge.svg)](https://codecov.io/gh/ylab-hi/BINARY)

# <center> **BI**oi**N**formatics **A**lgorithms lib**R**ar**Y** aka **BINARY**

## :star: Features

- Follows C++20 standard
- Supports modern C++ features
- Support Concurrency
- High performance
- Safety and security

## Library Introduction

The library is a collection of algorithms and data structures that are designed to be used in a modern C++
bioinformatics application. You can use the library in your own projects or as a part of a larger project.
The current project is developing and evolving, and changes will be made to the library as time goes on.

Mapping Structural Variants to Non-Linear Transcripts Variants

### Algorithms

The library contains algorithms for:

- Interval Tree
- VcfParser
- More to come

### Applications

The library contains applications for:

1. Mapping Structural Variants to Non-Linear Transcripts Variants

```console
$ sv2nl -h
```

<details>
 <summary><h2>How to Build</h2></summary>

### Build and run test suite

Use the following commands from the project's root directory to run the test suite.

```bash
cmake -S test -B build/test
cmake --build build/test
CTEST_OUTPUT_ON_FAILURE=1 cmake --build build/test --target test

# or simply call the executable:
./build/test/Sv2nlTests
```

To collect code coverage information, run CMake with the `-DENABLE_TEST_COVERAGE=1` option.

### Run clang-format

Use the following commands from the project's root directory to check and fix C++ and CMake source style.
This requires _clang-format_, _cmake-format_ and _pyyaml_ to be installed on the current system.

```bash
cmake -S test -B build/test

# view changes
cmake --build build/test --target format

# apply changes
cmake --build build/test --target fix-format
```

See [Format.cmake](https://github.com/TheLartians/Format.cmake) for details.

### Build the documentation

The documentation is automatically built and [published](https://github.com/ylab-hi/BINARY) whenever
a [GitHub Release](https://help.github.com/en/github/administering-a-repository/managing-releases-in-a-repository) is
created.
To manually build documentation, call the following command.

```bash
cmake -S documentation -B build/doc
cmake --build build/doc --target Sv2nlDocs
# view the docs
open build/doc/doxygen/html/index.html
```

To build the documentation locally, you will need Doxygen, jinja2 and Pygments on installed your system.

### Build everything at once

The project also includes an `all` directory that allows building all targets at the same time.
This is useful during development, as it exposes all subprojects to your IDE and avoids redundant builds of the library.

```bash
cmake -S . -B build
cmake --build build

# run tests
./build/test/Sv2nlTests
# format code
cmake --build build --target fix-format
# run standalone
./build/standalone/Greeter --help
# build docs
cmake --build build --target Sv2nlDocs
```

### Additional tools

The test and standalone subprojects include the [tools.cmake](cmake/tools.cmake) file which is used to import additional
tools on-demand through CMake configuration arguments.
The following are currently supported.

#### Sanitizers

Sanitizers can be enabled by configuring CMake
with `-DUSE_SANITIZER=<Address | Memory | MemoryWithOrigins | Undefined | Thread | Leak | 'Address;Undefined'>`.

#### Static Analyzers

Static Analyzers can be enabled by setting `-DUSE_STATIC_ANALYZER=<clang-tidy | iwyu | cppcheck>`, or a combination of
those in quotation marks, separated by semicolons.
By default, analyzers will automatically find configuration files such as `.clang-format`.
Additional arguments can be passed to the analyzers by setting the `CLANG_TIDY_ARGS`, `IWYU_ARGS` or `CPPCHECK_ARGS`
variables.

#### Ccache

Ccache can be enabled by configuring with `-DUSE_CCACHE=<ON | OFF>`.

</details>

## Acknowledgements

- https://github.com/TheLartians/ModernCppStarter
- https://github.com/samtools/htslib/blob/develop/htslib
