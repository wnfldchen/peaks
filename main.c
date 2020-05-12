#include <stdio.h>
#include <stddef.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include "map/maps.h"
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
        assert(opt_code != '?');
    } while (opt_code != -1);
    assert(input_file != NULL);
    assert(output_file != NULL);
    for (Map_e map = MAP_0X; map <= MAP_22; map += 1) {
        printf("%d %p\n", map, get_map_p(map));
    }
    return 0;
}
