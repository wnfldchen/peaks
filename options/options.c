// peaks. Copyright (c) 2020. Winfield Chen.

#include <stddef.h>
#include <getopt.h>
#include "options.h"

struct option const options[] = {
        {
                .name = "output-path",
                .has_arg = required_argument,
                .flag = NULL,
                .val = OUTPUT_PATH
        },
        {
                .name = "chromosome",
                .has_arg = required_argument,
                .flag = NULL,
                .val = CHROMOSOME
        },
        {
                .name = "min-p",
                .has_arg = required_argument,
                .flag = NULL,
                .val = MIN_P
        },
        {
                .name = "min-maf",
                .has_arg = required_argument,
                .flag = NULL,
                .val = MIN_MAF
        },
        {
                .name = "variants-file",
                .has_arg = required_argument,
                .flag = NULL,
                .val = VARIANTS_FILE
        },
        {
                .name = "table-1",
                .has_arg = no_argument,
                .flag = NULL,
                .val = TABLE_1
        },
        {
                .name = "max-procs",
                .has_arg = required_argument,
                .flag = NULL,
                .val = MAX_PROCS
        },
        {
                .name = "exclude-file",
                .has_arg = required_argument,
                .flag = NULL,
                .val = EXCLUDE_FILE
        },
        {
                .name = "pad",
                .has_arg = no_argument,
                .flag = NULL,
                .val = PAD
        },
        {
                .name = "find-file",
                .has_arg = required_argument,
                .flag = NULL,
                .val = FIND_FILE
        },
        {
                .name = "find-rep",
                .has_arg = no_argument,
                .flag = NULL,
                .val = FIND_REP
        },
        {
                .name = "find-rev",
                .has_arg = no_argument,
                .flag = NULL,
                .val = FIND_REV
        },
        {
                .name = "skip",
                .has_arg = no_argument,
                .flag = NULL,
                .val = SKIP
        },
        {
                .name = "map-file",
                .has_arg = required_argument,
                .flag = NULL,
                .val = MAP_FILE
        },
        {0}
};
