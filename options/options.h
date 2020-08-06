// peaks. Copyright (c) 2020. Winfield Chen.

#ifndef PEAKS_OPTIONS_H
#define PEAKS_OPTIONS_H
#include <getopt.h>
enum {
    OUTPUT_PATH,
    CHROMOSOME,
    MIN_P,
    MIN_MAF,
    VARIANTS_FILE,
    TABLE_1,
    MAX_PROCS,
    EXCLUDE_FILE,
    PAD,
    FIND
};
extern struct option const options[];
#endif //PEAKS_OPTIONS_H
