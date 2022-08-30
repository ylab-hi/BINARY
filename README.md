[![Ubuntu](https://github.com/ylab-hi/BINARY/actions/workflows/linux.yml/badge.svg)](https://github.com/ylab-hi/BINARY/actions/workflows/linux.yml)
[![Style](https://github.com/ylab-hi/BINARY/actions/workflows/style.yml/badge.svg)](https://github.com/ylab-hi/BINARY/actions/workflows/style.yml)
[![codecov](https://codecov.io/gh/ylab-hi/BINARY/branch/main/graph/badge.svg?token=RWC5iqNPVi)](https://codecov.io/gh/ylab-hi/BINARY)
[![c++20](https://img.shields.io/badge/C++-c%2B%2B20-green)](https://en.cppreference.com/w/cpp/20)
[![CodeQL](https://github.com/ylab-hi/BINARY/actions/workflows/codeql-analysis.yml/badge.svg)](https://github.com/ylab-hi/BINARY/actions/workflows/codeql-analysis.yml)
[![License](https://img.shields.io/github/license/ylab-hi/BINARY)](https://github.com/ylab-hi/BINARY/blob/main/LICENSE)
![compiler](https://img.shields.io/badge/Compiler-GCC10%20%7C%20GCC11%20%7C%20GCC12-green)

# <center> **BI**oi**N**formatics **A**lgorithms lib**R**ar**Y** aka **BINARY**

## Index

* [News](#news)
* [Features](#features)
* [Introduction](#library-introduction)

## News

- New version of the library: 0.1.0

## Features

- Embrace C++20 standard
- Supports modern C++ features
- Support concurrency and thread safety
- Priority is given to safety and clean design
- Hard to use wrongly
- Testing extensively
- Python bindings

## Library Introduction

The library is a collection of algorithms and data structures that are designed for modern C++
bioinformatics applications. You can use the library in your own projects or as a part of a larger project.
The current project is developing and evolving, and changes will be made to the library as time goes on.

### Algorithms

The library contains algorithms for:

- Interval Tree
- VcfParser
- More to come

### Tools

The library contains applications for:

1. Mapping Structural Variants to Non-Linear Transcripts Variants

```console
$ sv2nl -h
```

<details>
 <summary><h2>How to Build</h2></summary>

### Build everything at once

The project also includes an `all` directory that allows building all targets at the same time.
This is useful during development, as it exposes all subprojects to your IDE and avoids redundant builds of the library.

```bash
cmake -S . -B build
cmake --build build

# run tests
./build/test/BinaryTests

# format code
cmake --build build --target fix-format

# run standalone
./build/standalone/sv2nl --help

# build docs
cmake --build build --target BinaryDocs
```

### Build and run test suite

Use the following commands from the project's root directory to run the test suite.

```bash
cmake -S test -B build/test
cmake --build build/test
CTEST_OUTPUT_ON_FAILURE=1 cmake --build build/test --target test

# or simply call the executable:
./build/test/BinaryTests
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
cmake --build build/doc --target BinaryDocs
# view the docs
open build/doc/doxygen/html/index.html
```

To build the documentation locally, you will need Doxygen, jinja2 and Pygments on installed your system.

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

- https://github.com/samtools/htslib/blob/develop/htslib
- https://github.com/doctest/doctest
- https://github.com/jarro2783/cxxopts
- https://github.com/gabime/spdlog
- https://github.com/TheLartians/ModernCppStarter

## Contributors ✨

Thanks goes to these wonderful people ([emoji key](https://allcontributors.org/docs/en/emoji-key)):

<!-- ALL-CONTRIBUTORS-LIST:START - Do not remove or modify this section -->
<!-- prettier-ignore-start -->
<!-- markdownlint-disable -->
<table>
  <tr>
    <td align="center"><a href="https://yangyangli.top"><img src="https://avatars.githubusercontent.com/u/38903141?v=4?s=100" width="100px;" alt=""/><br /><sub><b>yangliz5</b></sub></a><br /><a href="#infra-cauliyang" title="Infrastructure (Hosting, Build-Tools, etc)">🚇</a></td>
  </tr>
</table>

<!-- markdownlint-restore -->
<!-- prettier-ignore-end -->

<!-- ALL-CONTRIBUTORS-LIST:END -->
