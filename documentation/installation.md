# Installations

## Build everything at once

The project also includes an `all` directory that allows building all targets at the same time.
This is useful during development, as it exposes all subprojects to your IDE and avoids redundant builds of the library.

__NOTE:__ If you are using compiler in conda environment you need to define `-DCMAKE_PREFIX_PATH` environment variable
to your conda environment with related compiler. For
example: `cmake -S . -B build -DCMAKE_PREFIX_PATH=/home/user/miniconda3/`.

```console
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

## Build and run test suite

Use the following commands from the project's root directory to run the test suite.

```console
cmake -S test -B build/test
cmake --build build/test
CTEST_OUTPUT_ON_FAILURE=1 cmake --build build/test --target test

# or simply call the executable:
./build/test/BinaryTests
```

To collect code coverage information, run CMake with the `-DENABLE_TEST_COVERAGE=1` option.

## Run clang-format

Use the following commands from the project's root directory to check and fix C++ and CMake source style.
This requires _clang-format_, _cmake-format_ and _pyyaml_ to be installed on the current system.

```console
cmake -S test -B build/test

# view changes
cmake --build build/test --target format

# apply changes
cmake --build build/test --target fix-format
```

See [Format.cmake] for details.

## Build the documentation

The documentation is automatically built and [published] whenever
a [GitHub Release] is
created.
To manually build documentation, call the following command.

```console
cmake -S documentation -B build/doc
cmake --build build/doc --target BinaryDocs
# view the docs
open build/doc/doxygen/html/index.html
```

To build the documentation locally, you will need Doxygen, jinja2 and Pygments on installed your system.

## Additional tools

The test and standalone subprojects include the [tools.cmake] file which is used to import additional
tools on-demand through CMake configuration arguments.
The following are currently supported.

### Sanitizers

Sanitizers can be enabled by configuring CMake
with `-DUSE_SANITIZER=<Address | Memory | MemoryWithOrigins | Undefined | Thread | Leak | 'Address;Undefined'>`.

### Static Analyzers

Static Analyzers can be enabled by setting `-DUSE_STATIC_ANALYZER=<clang-tidy | iwyu | cppcheck>`, or a combination of
those in quotation marks, separated by semicolons.
By default, analyzers will automatically find configuration files such as `.clang-format`.
Additional arguments can be passed to the analyzers by setting the `CLANG_TIDY_ARGS`, `IWYU_ARGS` or `CPPCHECK_ARGS`
variables.

### Ccache

Ccache can be enabled by configuring with `-DUSE_CCACHE=<ON | OFF>`.

[tools.cmake]:  ../cmake/tools.cmake

[format.cmake]: https://github.com/TheLartians/Format.cmake

[published]: https://github.com/ylab-hi/BINARY

[gitHub release]: https://help.github.com/en/github/administering-a-repository/managing-releases-in-a-repository
