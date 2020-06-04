#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include "map/maps.h"
#include "heap/heap.h"
double maf(FILE * variants_file, Association association) {
    // TODO
    return 0.5;
}

int main(int argc, char ** argv) {
    enum {
        INPUT_FILE,
        OUTPUT_FILE,
        CHROMOSOME,
        MIN_P,
        MIN_MAF,
        VARIANTS_FILE,
        TABLE_1
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
        {0}
    };
    FILE * input_file = NULL;
    FILE * output_file = NULL;
    FILE * variants_file = NULL;
    uint8_t chromosome = (uint8_t) (-1);
    uint8_t table_1_mode = 0;
    double min_p = 0.0;
    double min_maf = 0.0;
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
            case CHROMOSOME: {
                char chromosome_opt[2] = {0};
                strncpy(chromosome_opt, optarg, 2);
                if (chromosome_opt[0] == 'X') {
                    chromosome_opt[1] = '\0';
                    chromosome = 0;
                } else {
                    sscanf(chromosome_opt,
                           "%2hhu",
                           &chromosome);
                }
                break;
            }
            case MIN_P:
                sscanf(optarg, "%lf", &min_p);
                break;
            case MIN_MAF:
                sscanf(optarg, "%lf", &min_maf);
                break;
            case VARIANTS_FILE:
                variants_file = fopen(optarg, "r");
                if (variants_file == NULL) {
                    fputs("Error opening variants file\n", stderr);
                    opt_code = '?';
                }
            case TABLE_1:
                table_1_mode = 1;
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
            if (variants_file != NULL) {
                fclose(variants_file);
            }
            fputs("Error parsing arguments\n", stderr);
            return EINVAL;
        }
    } while (opt_code != -1);
    uint8_t threshold_maf = min_maf > 0.0;
    uint8_t threshold_p = min_p > 0.0;
    if (input_file == NULL) {
        fputs("No such input file\n", stderr);
        return ENOENT;
    }
    if (output_file == NULL) {
        fputs("Error opening output file\n", stderr);
        return EBADF;
    }
    if (chromosome != (uint8_t) (-1) && chromosome > 22) {
        fputs("Invalid chromosome\n", stderr);
        return EINVAL;
    }
    if (min_maf < 0.0 || min_maf > 0.5) {
        fputs("Invalid MAF threshold\n", stderr);
        return EINVAL;
    }
    if (threshold_maf && variants_file == NULL) {
        fputs("MAF threshold requires variants file\n", stderr);
        return EINVAL;
    }
    if (table_1_mode && variants_file == NULL) {
        fputs("Table 1 mode requires variants file\n", stderr);
        return EINVAL;
    }
    if (min_p < 0.0) {
        fputs("-log(p) threshold less than 0\n", stderr);
        return EINVAL;
    }
    char line[256] = {0};
    uint8_t skip_header_lines = 1;
    while (fgets(line, 256, input_file) != NULL) {
        if (skip_header_lines) {
            skip_header_lines -= 1;
        } else {
            Association association = {0};
            if (table_1_mode) {
                sscanf(line,
                       "%2c %255s %u %255s %255s %lf %lf %lf",
                       association.chr_2c,
                       association.rsid_255s,
                       &association.pos_u,
                       association.a1_255s,
                       association.a2_255s,
                       &association.beta_lf,
                       &association.se_lf,
                       &association.p_lf);
            } else {
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
            }
            if (association.chr_2c[0] == 'X') {
                association.chr_2c[1] = '\0';
                association.chr_2hhu = 0;
            } else {
                sscanf(association.chr_2c,
                       "%2hhu",
                       &association.chr_2hhu);
            }
            if ((!threshold_p || association.p_lf > min_p) &&
                (!threshold_maf || maf(variants_file, association) > min_maf) &&
                (chromosome == (uint8_t) (-1) || association.chr_2hhu == chromosome)) {
                emplace_array(association);
            }
        }
    }
    make_heaps();
    if (table_1_mode) {
        fputs("chr rsid pos gd a1 a2 maf pval\n", output_file);
    } else {
        fputs("lead_rsid lead_pheno lead_p-value lead_chr lead_pos cluster_nominal cluster_rsids_phenos\n", output_file);
    }
    for (uint8_t chr = 0; chr < 23; chr += 1) {
        Map const * const map = get_map_p(chr);
        Heap * const heap = &heaps[chr];
        while (heap->n > 0) {
            Association const lead = extract_heap(heap);
            if (table_1_mode) {
                fprintf(output_file,
                        "%.2s %.255s %u %lf %.255s %.255s %lf %lf",
                        lead.chr_2c,
                        lead.rsid_255s,
                        lead.pos_u,
                        get_gen_map_cm(map, lead.pos_u),
                        lead.a1_255s,
                        lead.a2_255s,
                        maf(variants_file, lead),
                        lead.p_lf);
            } else {
                fprintf(output_file,
                        "%.255s %.5s %lf %.2s %u",
                        lead.rsid_255s,
                        lead.pheno_5c,
                        lead.p_lf,
                        lead.chr_2c,
                        lead.pos_u);
            }
            uint16_t nominal_sum = lead.nominal_hhu;
            char non_leads[4194304] = {0}; // 4194304 = 256 (one line) * 16384 (hits)
            uint32_t cursor = 0;
            uint8_t cluster_flag[16384] = {0};
            for (uint16_t i = 0; i < heap->n; i += 1) {
                if (get_gen_map_dist(map, lead.pos_u, heap->array[i].pos_u) < 0.25) {
                    cluster_flag[i] = 1;
                    if (!table_1_mode) {
                        Association const non_lead = heap->array[i];
                        nominal_sum += non_lead.nominal_hhu;
                        cursor += sprintf(&non_leads[cursor],
                                          " %.255s %.5s",
                                          non_lead.rsid_255s,
                                          non_lead.pheno_5c);
                    }
                }
            }
            batch_delete_heap(heap, cluster_flag);
            if (!table_1_mode) {
                fprintf(output_file, " %hu", nominal_sum);
                fputs(non_leads, output_file);
            }
            fputc('\n', output_file);
        }
    }
    fclose(input_file);
    fclose(output_file);
    if (variants_file != NULL) {
        fclose(variants_file);
    }
    return 0;
}
