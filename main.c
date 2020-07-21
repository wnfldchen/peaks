#define _GNU_SOURCE
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <alloca.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "maps.h"
#include "heap.h"
#include "format.h"

int main(int argc, char ** argv) {
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
                .name = "find",
                .has_arg = required_argument,
                .flag = NULL,
                .val = FIND
        },
        {0}
    };
    char * output_path = NULL;
    FILE * variants_file = NULL;
    FILE * exclude_file = NULL;
    uint8_t chromosome = (uint8_t) (-1);
    uint8_t table_1_mode = 0;
    uint8_t pad = 0;
    char * find = NULL;
    double min_p = 0.0;
    double min_maf = 0.0;
    int max_procs = 1;
    int opt_code;
    int errsv;
    do {
        opt_code = getopt_long(argc, argv, "", options, NULL);
        switch (opt_code) {
            case OUTPUT_PATH:
                output_path = strdupa(optarg);
                break;
            case CHROMOSOME:
                if (optarg[0] == 'X' ||
                    (optarg[0] && optarg[1] == 'X')) {
                    chromosome = 0;
                } else if (sscanf(optarg,
                                  "%2hhu",
                                  &chromosome) != 1) {
                    errsv = errno;
                    perror("--chromosome");
                    return errsv;
                }
                break;
            case MIN_P:
                if (sscanf(optarg, "%lf", &min_p) != 1) {
                    errsv = errno;
                    perror("--min-p");
                    return errsv;
                }
                break;
            case MIN_MAF:
                if (sscanf(optarg, "%lf", &min_maf) != 1) {
                    errsv = errno;
                    perror("--min-maf");
                    return errsv;
                }
                break;
            case VARIANTS_FILE:
                variants_file = fopen(optarg, "r");
                if (!variants_file) {
                    errsv = errno;
                    perror("--variants-file");
                    return errsv;
                }
                break;
            case TABLE_1:
                table_1_mode = 1;
                break;
            case MAX_PROCS:
                if (sscanf(optarg, "%d", &max_procs) != 1) {
                    errsv = errno;
                    perror("--max-procs");
                    return errsv;
                }
                break;
            case EXCLUDE_FILE:
                exclude_file = fopen(optarg, "r");
                if (!exclude_file) {
                    errsv = errno;
                    perror("--exclude-file");
                    return errsv;
                }
                break;
            case PAD:
                pad = 1;
                break;
            case FIND:
                find = strdupa(optarg);
                break;
            case '?':
                fputs("Error parsing arguments\n", stderr);
                return EINVAL;
        }
    } while (opt_code != -1);
    if (!output_path) {
        fputs("No output path\n", stderr);
        return EINVAL;
    }
    if (optind >= argc) {
        fputs("No input file\n", stderr);
        return EINVAL;
    }
    if (max_procs < 1) {
        fputs("Invalid --max-procs\n", stderr);
        return EINVAL;
    }
    uint8_t const threshold_maf = min_maf > 0.0;
    uint8_t const threshold_p = min_p > 0.0;
    uint8_t const filter_chromosome = chromosome != (uint8_t) (-1);
    if (filter_chromosome && chromosome > 22) {
        fputs("Invalid --chromosome\n", stderr);
        return EINVAL;
    }
    if (min_maf < 0.0 || min_maf > 0.5) {
        fputs("Invalid --min-maf\n", stderr);
        return EINVAL;
    }
    if (threshold_maf && !variants_file) {
        fputs("--min-maf requires --variants-file\n", stderr);
        return EINVAL;
    }
    if (table_1_mode && !variants_file) {
        fputs("--table-1 requires --variants-file\n", stderr);
        return EINVAL;
    }
    if (find && !variants_file) {
        fputs("--find requires --variants-file\n", stderr);
        return EINVAL;
    }
    if (min_p < 0.0) {
        fputs("Invalid --min-p\n", stderr);
        return EINVAL;
    }
    struct format * variants_format;
    if (variants_file) {
        parse_format_string(&variants_format, "crpAaf");
        read_format_file(variants_format, variants_file, 1, " ");
        fclose(variants_file);
        parse_format_chr(variants_format, pad ? '0' : '\0');
    }
    struct format * exclude_format;
    if (exclude_file) {
        parse_format_string(&exclude_format, "cpAa");
        read_format_file(exclude_format, exclude_file, 0, "_");
        fclose(exclude_file);
        parse_format_chr(exclude_format, pad ? '0' : '\0');
    }
    FILE * input_file = NULL;
    FILE * output_file = NULL;
    pid_t pid;
    for (int i = optind; i < argc; i += 1) {
        if (i - optind >= max_procs) {
            wait(NULL);
        }
        pid = fork();
        if (pid == -1) {
            errsv = errno;
            perror("fork");
            return errsv;
        }
        if (!pid) {
            input_file = fopen(argv[i], "r");
            if (!input_file) {
                errsv = errno;
                perror("input_file");
                fflush(stderr);
                _exit(errsv);
            }
            char * const b = basename(argv[i]);
            char * const s = alloca(strlen(output_path) +
                              strlen(b) + 2);
            if (stpcpy(s, output_path)[-1] != '/') {
                strcat(s, "/");
            }
            strcat(s, b);
            output_file = fopen(s, "w");
            if (!output_file) {
                errsv = errno;
                perror("output_file");
                fflush(stderr);
                _exit(errsv);
            }
            break;
        }
    }
    if (pid && variants_file) {
        destroy_format(variants_format);
    }
    if (pid && exclude_file) {
        destroy_format(exclude_format);
    }
    for (int i = 0; pid && i < max_procs; i += 1) {
        wait(NULL);
    }
    if (pid) {
        return EXIT_SUCCESS;
    }
    struct format * input_format;
    parse_format_string(&input_format,
            table_1_mode ? "crpAas" : "rtcpAas*****n");
    read_format_file(input_format, input_file, 1, " ");
    fclose(input_file);
    parse_format_chr(input_format, pad ? '0' : '\0');
    if (variants_file) {
        join_format(input_format, variants_format, "f", 1);
    }
    join_format(input_format, NULL, "d", 1);
    calc_gd(input_format);
    if (chromosome != (uint8_t) (-1)) {
        set_func_chr_chr(chromosome);
        mark_format_func(input_format, func_chr);
    }
    if (threshold_maf) {
        set_func_maf_maf(min_maf);
        mark_format_func(input_format, func_maf);
    }
    if (threshold_p) {
        set_func_p_p(min_p);
        mark_format_func(input_format, func_p);
    }
    if (exclude_file) {
        mark_format(input_format, exclude_format, 1);
    }
    init_heaps(input_format, chromosome != (uint8_t) (-1) ||
                             threshold_maf ||
                             threshold_p ||
                             exclude_file);
    make_heaps();
    if (table_1_mode) {
        fputs("chr rsid pos gd a1 a2 maf pval\n",
                output_file);
    } else {
        fputs("lead_rsid lead_pheno lead_p-value lead_chr lead_pos cluster_nominal cluster_rsids_phenos\n",
                output_file);
    }
    for (uint8_t chr = 0; chr < 23; chr += 1) {
        struct map const * const map = get_map_p(chr);
        struct heap * const heap = &heaps[chr];
        while (heap->len > 0) {
            size_t const lead = extract_heap(heap);
            double const gd = *(double *)get_format_field(input_format, lead, GD);
            if (table_1_mode) {
                double const af = *(double *)get_format_field(input_format, lead, AF);
                fprintf(output_file,
                        "%.2s %s %u %lf %s %s %lf %lf",
                        (char *)get_format_field(input_format, lead, CHR),
                        *(char **)get_format_field(input_format, lead, RSID),
                        *(uint32_t *)get_format_field(input_format, lead, POS),
                        gd,
                        *(char **)get_format_field(input_format, lead, A1),
                        *(char **)get_format_field(input_format, lead, A2),
                        af < 0.5 ? af : 1.0 - af,
                        *(double *)get_format_field(input_format, lead, P));
            } else {
                fprintf(output_file,
                        "%s %.5s %lf %.2s %u",
                        *(char **)get_format_field(input_format, lead, RSID),
                        (char *)get_format_field(input_format, lead, PHENO),
                        *(double *)get_format_field(input_format, lead, P),
                        (char *)get_format_field(input_format, lead, CHR),
                        *(uint32_t *)get_format_field(input_format, lead, POS));
            }
            set_func_gd_circle(gd, 0.25);
            mark_heap_func(heap, func_gd);
            if (!table_1_mode) {
                fprintf(output_file, " %u", *(uint32_t *)get_format_field(input_format, lead, NOM)
                                                   + acc_heap_nom(heap));
                print_heap_nonleads(heap, output_file);
            }
            batch_delete_heap(heap);
            fputc('\n', output_file);
        }
    }
    fclose(output_file);
    destroy_heaps();
    if (find) {
        fputs(find, stdout);
        size_t find_idx = 0;
        size_t const find_lines = variants_format->r;
        while (find_idx < find_lines &&
               strcmp(find, *(char **)get_format_field(variants_format, find_idx, RSID))) {
            find_idx += 1;
        }
        if (find_idx < find_lines) {
            double const gd = get_gen_map_cm(
                    get_map_p(get_format_chr(variants_format, find_idx)),
                    *(uint32_t *)get_format_field(variants_format, find_idx, POS));
            size_t const lines = input_format->r;
            for (size_t line_idx = 0; line_idx < lines; line_idx += 1) {
                if (!get_format_flag(input_format, line_idx) &&
                    get_format_chr(variants_format, find_idx) == get_format_chr(input_format, line_idx)) {
                    double const gd_lead = *(double *)get_format_field(input_format, line_idx, GD);
                    if ((gd_lead > gd ? gd_lead - gd : gd - gd_lead) < 0.5) {
                        fputc(' ', stdout);
                        fputs(*(char **)get_format_field(input_format, line_idx, RSID), stdout);
                    }
                }
            }
        } else {
            fputs(" NOT_IN_VARIANTS_FILE", stdout);
        }
        fputc('\n', stdout);
        fflush(stdout);
    }
    destroy_format(input_format);
    if (exclude_file) {
        destroy_format(exclude_format);
    }
    if (variants_file) {
        destroy_format(variants_format);
    }
    _exit(EXIT_SUCCESS);
}
