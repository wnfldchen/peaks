The peaks software repository

# Build

CMake 3.16 or higher is required.

```
mkdir cmake-build-debug
cd cmake-build-debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```
Gives a debug build. Similarly, the release build can use
folder `cmake-build-release` and option `-DCMAKE_BUILD_TYPE=Release`.

# Usage

```
peaks --output-path DIRECTORY [OPTIONS] INPUT_FILE_1 [INPUT_FILE_2 ...]
```

# Examples

```
./peaks --output-path . ../big40/Table1.txt
```
The output file has the same file name as the input file but in the output directory.
File `./Table1.txt` now contains the clusters in `../big40/Table1.txt` according to the
algorithm described in the paper "Enhanced Brain Imaging Genetics in UK Biobank."
The output should be identical to `../big40/Table2.txt`.

```
./peaks --output-path . --chromosome X ../big40/Table1.txt
```
The output is as before except filtered to only the X chromosome.
The output should be identical to `../big40/Table2X.txt`.

```
./peaks --output-path . ../big40/Table1fmg.txt
```
The output is the clusters in the Fisher hits.
The lead rsid of the last cluster should be `rs5955807` with associated phenotype `V1991`.
The output should be identical to `../big40/Table2fmg.txt`.

```
./peaks --output-path . --find rs5955807 --variants-file ~/Downloads/variants.txt --chromosome X ../big40/Table1.txt
```
Recreate `../big40/Table2X.txt` as in the second example.
Use the downloaded variants file to match `rs5955807` from the Fisher clusters against the output clusters.
In addition to the output file as before, the standard output prints two successful matches:
```
rs5955807 rs146090761 rs139593778
```
The first field is the query itself (`rs5955807`), which is followed by a space-separated list of matching
output clusters (`rs146090761 rs139593778`). If there are no matches, the list is empty.
If the query is not in the variants file, the list is replaced with the error message `NOT_IN_VARIANTS_FILE`.

# More options

In addition to the options in the examples above, there are the following extra options:

```
--min-p 7.5
```
Exclude all entries with negative log-10 p-value field less than 7.5.

```
--min-maf 0.01
```
Exclude all entries with MAF less than 0.01. Requires `--variants-file`.

```
--table-1
```
Generate Table 1 format files (intra-phenotype clustering).
Each input file should contain all the entries for one and only one phenotype.
Requires `--variants-file`.
The default operation without this flag is Table 2 format files (inter-phenotype clustering).

```
--max-procs 8
```
Use 8 worker processes to process input files.
Keep this value less than or equal to the number of input files.
Not only does this finish faster due to parallelism, it also saves memory over running peaks
on each file since shared read-only data (such as a variants file or exclude file) loaded pre-fork
is not physically duplicated in memory on copy-on-write operating systems such as Linux.

```
--exclude-file exclude.txt
```
Exclude all entries in the exclude file.

```
--pad
```
Pad input chromosome field value `X` into `0X` before output.
