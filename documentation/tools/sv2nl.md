# sv2nl

## Table of Contents

- [1. Aim](#aim)
- [2. Usage](#usage)
- [3. Performance](#performance)
- [4. Current Issues](#current-issues)

## Aim

`sv2nl` is a tool that maps structural variants to non-linear transcripts.

## Usage

`sv2nl -h` will show the help message. Currently, the tool only supports VCF files as input. DNA structural variations
are from `delly` and Non-linear transcripts are from `scannls`. The output is threes `tsv` files, one for each type of
structural variation (duplication, inversion, translocation). However, You can use the `-m` option to merge all the
output files into one.

```console
$ sv2nl -h
Map structural Variation to Non-Linear Transcription
Usage:
  sv2nl [OPTION...] [sv non-linear]

      --sv arg          The file path of segment information from delly
      --non-linear arg  The file path of non-linear information from scannls
      --dis arg         The distance threshold for trans mapper (default: 1000000)
  -o, --output arg      The file path of output (default: output.tsv)
  -t, --thread arg      The number of thread program use (default: 4)
  -m, --merge           If provided only merge outputs into one file
  -d, --debug           Print debug info
  -h, --help            Print help
  -v, --version         Print the current version number
```

- Example 1

`example.sv.vcf` is the DNA structural variation file from `delly`. `example.non-linear.vcf` is the non-linear
transcript. Option `-t` define the number of threads. Option `-o` define the output file prefix. If you don't provide
`-o` option, the output file prefix will be `output.tsv`. You will get three output files (`output.tsv.dup`
, `output.tsv.inv`, `output.tsv.tra`). If `-m` option is provided, you will get one output file (`output.tsv`).

```console
$ sv2nl example.sv.vcf example.non-linear.vcf -o example.tsv -t 4
```

## Performance

total runtime: 417.94s.
calls to allocation functions: 222033991 (531256/s)
temporary memory allocations: 8531158 (20412/s)
peak heap memory consumption: 25.66MB
peak RSS (including heaptrack overhead): 77.19MB
total memory leaked: 0B

The performance is tested simply by `time` and `heapstack` command. It is not a benchmark.
The slowest part is the gzip decompression and compression.

| SV File Size | SV Records | NL File Size | NL Records | Time | Peak Memory Usage |
|:------------:|:----------:|:------------:|:----------:|:----:|:-----------------:|
|     26MB     |   72'496   |    2.4MB     |   10'510   | 18S  |      25.66MB      |

## Current Issues
