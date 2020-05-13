#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include "map/maps.h"
#include "heap/heap.h"
int main(int argc, char ** argv) {
    enum {
        INPUT_FILE = 0x1,
        OUTPUT_FILE = 0x10
    };
    struct option const options[] = {
        {
            .name = "input-file",
            .has_arg = required_argument,
            .flag = NULL,
            .val = INPUT_FILE
        },
        {
            .name = "output-file",
            .has_arg = required_argument,
            .flag = NULL,
            .val = OUTPUT_FILE
        },
        {0}
    };
    FILE * input_file = NULL;
    FILE * output_file = NULL;
    int opt_code;
    do {
        opt_code = getopt_long(argc, argv, "", options, NULL);
        switch (opt_code) {
            case INPUT_FILE:
                input_file = fopen(optarg, "r");
                break;
            case OUTPUT_FILE:
                output_file = fopen(optarg, "w");
                break;
            default:
                break;
        }
        if (opt_code == '?') {
            if (input_file != NULL) {
                fclose(input_file);
            }
            if (output_file != NULL) {
                fclose(output_file);
            }
            fputs("Error parsing arguments\n", stderr);
            return EINVAL;
        }
    } while (opt_code != -1);
    if (input_file == NULL) {
        fputs("No such input file\n", stderr);
        return ENOENT;
    }
    if (output_file == NULL) {
        fputs("Error opening output file\n", stderr);
        return EBADF;
    }
    puts("Checking maps");
    for (Map_e map = MAP_0X; map <= MAP_22; map += 1) {
        printf("%d %p\n", map, get_map_p(map));
    }
    char line[256] = {0};
    uint8_t skip_header_lines = 1;
    while (fgets(line, 256, input_file) != NULL) {
        if (skip_header_lines) {
            skip_header_lines -= 1;
        } else {
            Association association = {0};
            sscanf(line,
                   "%255s %5c %2c %u %255s %255s %lf %lf %lf %lf %lf %lf %hhu",
                   association.rsid_255s,
                   association.pheno_5c,
                   association.chr_2c,
                   &association.pos_u,
                   association.a1_255s,
                   association.a2_255s,
                   &association.p_lf,
                   &association.beta_lf,
                   &association.se_lf,
                   &association.p_repro_lf,
                   &association.beta_repro_lf,
                   &association.se_repro_lf,
                   &association.nominal_hhu);
            if (association.chr_2c[0] == 'X') {
                association.chr_2hhu = 0;
            } else {
                sscanf(association.chr_2c,
                       "%2hhu",
                       &association.chr_2hhu);
            }
            emplace_array(association);
        }
    }
    fclose(input_file);
    fclose(output_file);
    return 0;
}
