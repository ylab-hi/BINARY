[![Ubuntu](https://github.com/ylab-hi/BINARY/actions/workflows/linux.yml/badge.svg)](https://github.com/ylab-hi/BINARY/actions/workflows/linux.yml)
[![Style](https://github.com/ylab-hi/BINARY/actions/workflows/style.yml/badge.svg)](https://github.com/ylab-hi/BINARY/actions/workflows/style.yml)
[![codecov](https://codecov.io/gh/ylab-hi/BINARY/branch/main/graph/badge.svg?token=RWC5iqNPVi)](https://codecov.io/gh/ylab-hi/BINARY)
[![c++20](https://img.shields.io/badge/C++-c%2B%2B20-green)](https://en.cppreference.com/w/cpp/20)
[![CodeQL](https://github.com/ylab-hi/BINARY/actions/workflows/codeql-analysis.yml/badge.svg)](https://github.com/ylab-hi/BINARY/actions/workflows/codeql-analysis.yml)
[![License](https://img.shields.io/github/license/ylab-hi/BINARY)](https://github.com/ylab-hi/BINARY/blob/main/LICENSE)
![compiler](https://img.shields.io/badge/Compiler-GCC11%20%7C%20GCC12-green)

# **BI**oi**N**formatics **A**lgorithms lib**R**ar**Y** aka **BINARY**

The library is a collection of algorithms and data structures that are designed for modern C++
bioinformatics applications. You can use the library in your own projects or as a part of a larger project.
The current project is developing and evolving, and changes will be made to the library as time goes on.

## Index

* [News](#news)
* [Features](#features)
* [Installation](documentation/pages/installations.md)
* [Algorithms](#algorithms)
* [Tools](#tools)
* [Acknowledgements](#acknowledgements)
* [Contributors](#contributors)

## News

- New version of the library: 0.1.0
- New tool [sv2nl](#tools) is released

## Features

- Embrace C++20 standard
- Supports modern C++ features
- Support concurrency and thread safety
- Priority is given to safety and clean design
- Hard to use wrongly
- Testing extensively
- Python bindings

## Algorithms

The library contains algorithms for:

- Interval Tree
- VcfParser
- More to come

## Tools

The library contains applications for:

1. Mapping Structural Variants to Non-Linear Transcripts Variants

```console
$ sv2nl -h
```

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
