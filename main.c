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
#include "map/maps.h"
#include "heap/heap.h"
#include "variant/variant.h"
#include "exclude/exclude.h"

int main(int argc, char ** argv) {
    enum {
        OUTPUT_PATH,
        CHROMOSOME,
        MIN_P,
        MIN_MAF,
        VARIANTS_FILE,
        TABLE_1,
        MAX_PROCS,
        EXCLUDE_FILE
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
        {0}
    };
    char * output_path = NULL;
    FILE * variants_file = NULL;
    FILE * exclude_file = NULL;
    uint8_t chromosome = (uint8_t) (-1);
    uint8_t table_1_mode = 0;
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
    uint8_t const require_variants = threshold_maf || table_1_mode;
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
    if (min_p < 0.0) {
        fputs("Invalid --min-p\n", stderr);
        return EINVAL;
    }
    char * line = NULL;
    size_t n = 0;
    uint8_t skip_header_lines = 1;
    uint32_t variants_sizes[23] = {0};
    while (require_variants &&
           getline(&line, &n, variants_file) != -1) {
        uint8_t i;
        if (skip_header_lines) {
            skip_header_lines -= 1;
        } else if (line[0] == 'X' || line[1] == 'X') {
            variants_sizes[0] += 1;
        } else if (sscanf(line, "%2hhu", &i) != 1 ||
                   i < 1 || i > 22) {
            fputs("Invalid variant chr\n", stderr);
        } else {
            variants_sizes[i] += 1;
        }
    }
    struct Variant * variants[23] = {0};
    for (uint8_t i = 0; require_variants && i < 23; i += 1) {
        variants[i] = calloc(variants_sizes[i], sizeof(struct Variant));
        if (!variants[i]) {
            errsv = errno;
            perror("variants");
            return errsv;
        }
    }
    if (require_variants) {
        rewind(variants_file);
        skip_header_lines = 1;
    }
    uint32_t variants_n[23] = {0};
    while (require_variants &&
           getline(&line, &n, variants_file) != -1) {
        struct Variant node = {0};
        if (skip_header_lines) {
            skip_header_lines -= 1;
        } else if (sscanf(line,
                          "%2c %ms %u %ms %ms %lf",
                          node.chr,
                          &node.rsid,
                          &node.pos,
                          &node.a1,
                          &node.a2,
                          &node.af) != 6) {
            fputs("Invalid variant line\n", stderr);
            free_variant(&node);
        } else {
            if (node.chr[0] == 'X' || node.chr[1] == 'X') {
                node.chr_id = 0;
            } else if (sscanf(node.chr, "%2hhu", &node.chr_id) != 1 ||
                       node.chr_id < 1 || node.chr_id > 22) {
                fputs("Invalid variant chr\n", stderr);
                free_variant(&node);
                continue;
            }
            uint8_t const i = node.chr_id;
            variants[i][variants_n[i]] = node;
            variants_n[i] += 1;
        }
    }
    for (uint8_t i = 0; require_variants && i < 23; i += 1) {
        if (variants_sizes[i] != variants_n[i]) {
            fputs("variants_size != variants_n\n", stderr);
        }
    }
    uint32_t exclude_sizes[23] = {0};
    while (exclude_file &&
           getline(&line, &n, exclude_file) != -1) {
        uint8_t i;
        if (line[0] == 'X' || line[1] == 'X') {
            exclude_sizes[0] += 1;
        } else if (sscanf(line, "%2hhu", &i) != 1 ||
                   i < 1 || i > 22) {
            fputs("Invalid exclude chr\n", stderr);
        } else {
            exclude_sizes[i] += 1;
        }
    }
    struct Exclude * exclude[23] = {0};
    for (uint8_t i = 0; exclude_file && i < 23; i += 1) {
        exclude[i] = calloc(exclude_sizes[i], sizeof(struct Exclude));
        if (!exclude[i]) {
            errsv = errno;
            perror("exclude");
            return errsv;
        }
    }
    if (exclude_file) {
        rewind(exclude_file);
    }
    uint32_t exclude_n[23] = {0};
    while (exclude_file &&
           getline(&line, &n, exclude_file) != -1) {
        struct Exclude node = {0};
        if (sscanf(line,
                   "%2c_%u_%m[^_]_%m[^_]",
                   node.chr,
                   &node.pos,
                   &node.a1,
                   &node.a2) != 4) {
            fputs("Invalid exclude entry\n", stderr);
            free_exclude(&node);
        } else {
            if (node.chr[0] == 'X' || node.chr[1] == 'X') {
                node.chr_id = 0;
            } else if (sscanf(node.chr, "%2hhu", &node.chr_id) != 1 ||
                       node.chr_id < 1 || node.chr_id > 22) {
                fputs("Invalid exclude chr\n", stderr);
                free_exclude(&node);
                continue;
            }
            uint8_t const i = node.chr_id;
            exclude[i][exclude_n[i]] = node;
            exclude_n[i] += 1;
        }
    }
    if (variants_file) {
        fclose(variants_file);
    }
    if (exclude_file) {
        fclose(exclude_file);
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
                _exit(errsv);
            }
            break;
        }
    }
    if (pid) {
        free(line);
    }
    for (uint8_t i = 0;
         pid && (require_variants || exclude_file) && i < 23;
         i += 1) {
        uint32_t const v_n = variants_n[i];
        uint32_t const e_n = exclude_n[i];
        if (require_variants) {
            for (uint32_t j = 0; j < v_n; j += 1) {
                free_variant(&variants[i][j]);
            }
            free(variants[i]);
        }
        if (exclude_file) {
            for (uint32_t j = 0; j < e_n; j += 1) {
                free_exclude(&exclude[i][j]);
            }
            free(exclude[i]);
        }
    }
    for (int i = 0; pid && i < max_procs; i += 1) {
        wait(NULL);
    }
    if (pid) {
        return EXIT_SUCCESS;
    }
    skip_header_lines = 1;
    uint32_t heap_sizes[23] = {0};
    while (getline(&line, &n, input_file) != -1) {
        char chr[2] = {0};
        uint8_t i;
        if (skip_header_lines) {
            skip_header_lines -= 1;
        } else if ((table_1_mode &&
                    sscanf(line, "%2c", chr) != 1)
                   ||
                   (!table_1_mode &&
                    sscanf(line, "%*s %*s %2c", chr) != 1)) {
            fputs("Invalid input line\n", stderr);
        } else if (chr[0] == 'X' || chr[1] == 'X') {
            heap_sizes[0] += 1;
        } else if (sscanf(chr, "%2hhu", &i) != 1 ||
                   i < 1 || i > 22) {
            fputs("Invalid input chr\n", stderr);
        } else {
            heap_sizes[i] += 1;
        }
    }
    for (uint8_t i = 0; i < 23; i += 1) {
        heaps[i].array = calloc(heap_sizes[i], sizeof(struct Node));
        if (!heaps[i].array) {
            errsv = errno;
            perror("heaps");
            _exit(errsv);
        }
    }
    rewind(input_file);
    skip_header_lines = 1;
    while (getline(&line, &n, input_file) != -1) {
        struct Node node = {0};
        if (skip_header_lines) {
            skip_header_lines -= 1;
        } else if ((table_1_mode &&
                    sscanf(line,
                           "%2c %ms %u %ms %ms %*s %*s %lf",
                           node.chr,
                           &node.rsid,
                           &node.pos,
                           &node.a1,
                           &node.a2,
                           &node.p) != 6)
                   ||
                   (!table_1_mode &&
                    sscanf(line,
                           "%ms %5c %2c %u %ms %ms %lf %*s %*s %*s %*s %*s %hhu",
                           &node.rsid,
                           node.pheno,
                           node.chr,
                           &node.pos,
                           &node.a1,
                           &node.a2,
                           &node.p,
                           &node.nom) != 8)) {
            fputs("Invalid input line\n", stderr);
            free_node(&node);
        } else {
            if (node.chr[1] == 'X') {
                node.chr_id = 0;
            } else if (node.chr[0] == 'X') {
                node.chr[1] = '\0';
                node.chr_id = 0;
            } else if (sscanf(node.chr, "%2hhu", &node.chr_id) != 1 ||
                       node.chr_id < 1 || node.chr_id > 22) {
                fputs("Invalid input chr\n", stderr);
                free_node(&node);
                continue;
            }
            if (exclude_file) {
                struct Exclude * const excl = exclude[node.chr_id];
                uint32_t const e_n = exclude_n[node.chr_id];
                uint32_t l = 0;
                uint32_t r = e_n;
                while (l < r) {
                    uint32_t const m = (l + r) / 2;
                    uint32_t const m_pos = excl[m].pos;
                    if (m_pos < node.pos) {
                        l = m + 1;
                    } else {
                        r = m;
                    }
                }
                while (l < e_n && excl[l].pos == node.pos &&
                       (strcmp(excl[l].a1, node.a1) ||
                        strcmp(excl[l].a2, node.a2))) {
                    l += 1;
                }
                if (l < e_n && excl[l].pos == node.pos) {
                    free_node(&node);
                    continue;
                }
            }
            if (require_variants) {
                struct Variant * const vars = variants[node.chr_id];
                uint32_t const v_n = variants_n[node.chr_id];
                uint32_t l = 0;
                uint32_t r = v_n;
                while (l < r) {
                    uint32_t const m = (l + r) / 2;
                    uint32_t const m_pos = vars[m].pos;
                    if (m_pos < node.pos) {
                        l = m + 1;
                    } else {
                        r = m;
                    }
                }
                while (l < v_n && vars[l].pos == node.pos &&
                       (strcmp(vars[l].rsid, node.rsid) ||
                        strcmp(vars[l].a1, node.a1) ||
                        strcmp(vars[l].a2, node.a2))) {
                    l += 1;
                }
                if (l >= v_n || vars[l].pos != node.pos) {
                    fputs("Variant not found\n", stderr);
                    free_node(&node);
                    continue;
                }
                node.af = vars[l].af;
            }
            if (table_1_mode) {
                node.gd = get_gen_map_cm(get_map_p(node.chr_id), node.pos);
            }
            if ((!threshold_p || node.p > min_p) &&
                (!threshold_maf || (node.af > min_maf && node.af < 1.0 - min_maf)) &&
                (chromosome == (uint8_t) (-1) || node.chr_id == chromosome)) {
                emplace_array(node);
            } else {
                free_node(&node);
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
        struct Map const * const map = get_map_p(chr);
        struct Heap * const heap = &heaps[chr];
        struct Node * non_leads = NULL;
        if (!table_1_mode) {
            non_leads = calloc(heap_sizes[chr], sizeof(struct Node));
            if (!non_leads) {
                fputs("Allocation of non_leads failed\n", stderr);
                continue;
            }
        }
        while (heap->n > 0) {
            struct Node const lead = extract_heap(heap);
            if (table_1_mode) {
                fprintf(output_file,
                        "%.2s %s %u %lf %s %s %lf %lf",
                        lead.chr,
                        lead.rsid,
                        lead.pos,
                        lead.gd,
                        lead.a1,
                        lead.a2,
                        lead.af < 0.5 ? lead.af : 1.0 - lead.af,
                        lead.p);
            } else {
                fprintf(output_file,
                        "%s %.5s %lf %.2s %u",
                        lead.rsid,
                        lead.pheno,
                        lead.p,
                        lead.chr,
                        lead.pos);
            }
            uint32_t nominal_sum = lead.nom;
            uint32_t non_leads_n = 0;
            for (uint32_t i = 0; i < heap->n; i += 1) {
                if (get_gen_map_dist(map, lead.pos, heap->array[i].pos) < 0.25) {
                    heap->array[i].flag = 1;
                    if (table_1_mode) {
                        free_node(&heap->array[i]);
                    } else {
                        nominal_sum += heap->array[i].nom;
                        non_leads[non_leads_n] = heap->array[i];
                        non_leads_n += 1;
                    }
                }
            }
            batch_delete_heap(heap);
            if (!table_1_mode) {
                fprintf(output_file, " %u", nominal_sum);
            }
            for (uint32_t i = 0; !table_1_mode && i < non_leads_n; i += 1) {
                fprintf(output_file,
                        " %s %.5s",
                        non_leads[i].rsid,
                        non_leads[i].pheno);
                free_node(&non_leads[i]);
            }
            fputc('\n', output_file);
            free_node(&lead);
        }
        if (!table_1_mode) {
            free(non_leads);
        }
    }
    for (uint8_t i = 0; i < 23; i += 1) {
        free(heaps[i].array);
        if (require_variants) {
            uint32_t const v_n = variants_n[i];
            for (uint32_t j = 0; j < v_n; j += 1) {
                free_variant(&variants[i][j]);
            }
            free(variants[i]);
        }
        if (exclude_file) {
            for (uint32_t j = 0; j < exclude_n[i]; j += 1) {
                free_exclude(&exclude[i][j]);
            }
            free(exclude[i]);
        }
    }
    free(line);
    fclose(input_file);
    fclose(output_file);
    _exit(EXIT_SUCCESS);
}
