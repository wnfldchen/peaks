//
// Created by winfield on 2020-06-23.
//

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include "format.h"
#include "maps.h"

char const * format_t(int const t) {
    static char const * const map[256] = {
            [0] = "%*s",
            [CHR] = "%2c",
            [POS] = "%u",
            [RSID] = "%ms",
            [PHENO] = "%5c",
            [A1] = "%ms",
            [A2] = "%ms",
            [AF] = "%lf",
            [GD] = "%lf",
            [P] = "%lf",
            [BETA] = "%lf",
            [SE] = "%lf",
            [P_REPRO] = "%lf",
            [BETA_REPRO] = "%lf",
            [SE_REPRO] = "%lf",
            [NOM] = "%u"
    };
    return map[t];
}

size_t sizeof_t(int const t) {
    static size_t const map[256] = {
            [CHR] = sizeof(char[2]),
            [POS] = sizeof(uint32_t),
            [RSID] = sizeof(char *),
            [PHENO] = sizeof(char[5]),
            [A1] = sizeof(char *),
            [A2] = sizeof(char *),
            [AF] = sizeof(double),
            [GD] = sizeof(double),
            [P] = sizeof(double),
            [BETA] = sizeof(double),
            [SE] = sizeof(double),
            [P_REPRO] = sizeof(double),
            [BETA_REPRO] = sizeof(double),
            [SE_REPRO] = sizeof(double),
            [NOM] = sizeof(uint32_t)
    };
    return map[t];
}

int map_t(char const c) {
    static int const map[256] = {
            ['c'] = CHR, ['p'] = POS, ['r'] = RSID,
            ['t'] = PHENO, ['A'] = A1, ['a'] = A2,
            ['f'] = AF, ['d'] = GD, ['s'] = P,
            ['b'] = BETA, ['e'] = SE, ['S'] = P_REPRO,
            ['B'] = BETA_REPRO, ['E'] = SE_REPRO,
            ['n'] = NOM
    };
    return map[c];
}

int parse_format_string(struct format ** const format, char const * const string) {
    struct format * const p = calloc(1, sizeof(struct format));
    if (!p) {
        int errsv = errno;
        perror("parse_format_string");
        return -errsv;
    }
    *format = p;
    size_t len = strlen(string);
    if (len > 255) {
        len = 255;
    }
    p->c = len;
    for (size_t i = 0; i < len; i += 1) {
        int const t = map_t(string[i]);
        p->t[i] = t;
        p->s[i] = 1;
        p->u[t] = i;
    }
    return len;
}

int read_format_file(struct format * const format, FILE * const file, uint8_t const skip, char const * const delim) {
    uint8_t const c = format->c;
    char * line = NULL;
    size_t n = 0;
    size_t lines = 0;
    uint8_t skip_lines = skip;
    while (getline(&line, &n, file) != -1) {
        if (skip_lines) {
            skip_lines -= 1;
        } else {
            lines += 1;
        }
    }
    format->r = lines;
    for (uint8_t i = 0; i < c; i += 1) {
        int const t = format->t[i];
        if (t) {
            format->v[i] = calloc(lines, sizeof_t(t));
            if (!format->v[i]) {
                int errsv = errno;
                perror("read_format_file");
                return -errsv;
            }
        }
    }
    format->v[255] = calloc(lines, sizeof(uint8_t));
    if (!format->v[255]) {
        int errsv = errno;
        perror("read_format_file");
        return -errsv;
    }
    rewind(file);
    skip_lines = skip;
    size_t line_idx = 0;
    while (getline(&line, &n, file) != -1) {
        if (skip_lines) {
            skip_lines -= 1;
        } else {
            char * tok = strtok(line, delim);
            for (uint8_t i = 0; i < c; i += 1) {
                if (tok) {
                    int const t = format->t[i];
                    void * const v = format->v[i];
                    if (t && v) {
                        void * p = v + (line_idx * sizeof_t(t));
                        if (sscanf(tok, format_t(t), p) != 1) {
                            fputs("Invalid token\n", stderr);
                        }
                    }
                    tok = strtok(NULL, delim);
                } else {
                    break;
                }
            }
            line_idx += 1;
        }
    }
    if (line) {
        free(line);
    }
    return line_idx;
}

void destroy_format(struct format * const format) {
    uint8_t const c = format->c;
    for (uint8_t i = 0; i < c; i += 1) {
        int const t = format->t[i];
        void * const v = format->v[i];
        if (t && v) {
            uint8_t const s = format->s[i];
            if (!strcmp(format_t(t), "%ms") && s) {
                size_t const lines = format->r;
                char * const * const p = v;
                for (size_t line_idx = 0;
                     line_idx < lines;
                     line_idx += 1) {
                    free(p[line_idx]);
                }
            }
            free(v);
        }
    }
    free(format->v[255]);
    free(format);
}

size_t match_format(
        struct format const * const a,
        struct format const * const b,
        size_t const a_idx,
        uint8_t const sorted) {
    uint8_t const chr = get_format_chr(a, a_idx);
    uint32_t const pos = *(uint32_t *)get_format_field(a, a_idx, POS);
    char const * const a1 = *(char **)get_format_field(a, a_idx, A1);
    char const * const a2 = *(char **)get_format_field(a, a_idx, A2);
    size_t const b_lines = b->r;
    size_t b_idx = 0;
    if (sorted) {
        while (b_idx < b_lines &&
               get_format_chr(b, b_idx) != chr) {
            b_idx += 1;
        }
        while (b_idx < b_lines &&
               get_format_chr(b, b_idx) == chr &&
               *(uint32_t *)get_format_field(b, b_idx, POS) < pos) {
            b_idx += 1;
        }
        while (b_idx < b_lines &&
               get_format_chr(b, b_idx) == chr &&
               *(uint32_t *)get_format_field(b, b_idx, POS) == pos &&
               (strcmp(a1, *(char **)get_format_field(b, b_idx, A1)) ||
                strcmp(a2, *(char **)get_format_field(b, b_idx, A2)))) {
            b_idx += 1;
        }
        if (b_idx >= b_lines ||
            get_format_chr(b, b_idx) != chr ||
            *(uint32_t *)get_format_field(b, b_idx, POS) != pos) {
            b_idx = (size_t) (-1);
        }
    } else {
        while (b_idx < b_lines &&
               (get_format_chr(b, b_idx) != chr ||
                *(uint32_t *)get_format_field(b, b_idx, POS) != pos ||
                strcmp(a1, *(char **)get_format_field(b, b_idx, A1)) ||
                strcmp(a2, *(char **)get_format_field(b, b_idx, A2)))) {
            b_idx += 1;
        }
        if (b_idx >= b_lines) {
            b_idx = (size_t) (-1);
        }
    }
    return b_idx;
}

uint8_t func_chr_chr = (uint8_t) (-1);

void set_func_chr_chr(uint8_t const chr) {
    func_chr_chr = chr;
}

uint8_t func_chr(struct format const * const format, size_t const line_idx) {
    return func_chr_chr != (uint8_t) (-1) &&
           func_chr_chr != get_format_chr(format, line_idx);
}

double func_p_p = 0.0;

void set_func_p_p(double const p) {
    func_p_p = p;
}

uint8_t func_p(struct format const * const format, size_t const line_idx) {
    return *(double *)get_format_field(format, line_idx, P) < func_p_p;
}

double func_maf_maf = 0.0;

void set_func_maf_maf(double const maf) {
    func_maf_maf = maf;
}

uint8_t func_maf(struct format const * const format, size_t const line_idx) {
    double const af = *(double *)get_format_field(format, line_idx, AF);
    double const maf = af > 0.5 ? 1.0 - af : af;
    return maf < func_maf_maf;
}

double func_gd_center = 0.0;
double func_gd_radius = 0.0;

void set_func_gd_circle(double const center, double const radius) {
    func_gd_center = center;
    func_gd_radius = radius;
}

uint8_t func_gd(struct format const * const format, size_t const line_idx) {
    double const gd = *(double *)get_format_field(format, line_idx, GD);
    if (gd > func_gd_center) {
        return gd - func_gd_center < func_gd_radius;
    } else if (gd < func_gd_center) {
        return func_gd_center - gd < func_gd_radius;
    } else {
        return 1;
    }
}

int mark_format_func(
        struct format * const target,
        uint8_t func(struct format const *, size_t)) {
    size_t const lines = target->r;
    for (size_t line_idx = 0; line_idx < lines; line_idx += 1) {
        if (!get_format_flag(target, line_idx) &&
            func(target, line_idx)) {
            set_format_flag(target, line_idx, 1);
        }
    }
    return 0;
}

int mark_format(
        struct format * const target,
        struct format const * const source,
        uint8_t const sorted) {
    size_t const lines = target->r;
    for (size_t line_idx = 0; line_idx < lines; line_idx += 1) {
        if (!get_format_flag(target, line_idx) &&
            match_format(target, source, line_idx, sorted) != (size_t) (-1)) {
            set_format_flag(target, line_idx, 1);
        }
    }
    return 0;
}

int join_format(
        struct format * const subset,
        struct format const * const superset,
        char const * const string,
        uint8_t const sorted) {
    size_t len = strlen(string);
    uint8_t const c = subset->c;
    if (len > 255 - c) {
        len = 255 - c;
    }
    subset->c = len + c;
    size_t const lines = subset->r;
    for (size_t i = 0; i < len; i += 1) {
        int const t = map_t(string[i]);
        subset->t[i + c] = t;
        subset->u[t] = i + c;
        if (t) {
            size_t const size = sizeof_t(t);
            subset->v[i + c] = calloc(lines, size);
            if (!subset->v[i + c]) {
                int errsv = errno;
                perror("join_format");
                return -errsv;
            }
            for (size_t line_idx = 0; superset && line_idx < lines; line_idx += 1) {
                size_t const sup_idx = match_format(subset, superset, line_idx, sorted);
                if (sup_idx == (size_t) (-1)) {
                    continue;
                }
                void * const dst = get_format_field(subset, line_idx, t);
                void const * const src = get_format_field(superset, sup_idx, t);
                memcpy(dst, src, size);
            }
        }
    }
    return subset->c;
}

void parse_format_chr(struct format * const format, char const pad) {
    uint8_t const i = format->u[CHR];
    uint8_t * const flags = format->v[255];
    if (format->t[i] == CHR && flags) {
        size_t const lines = format->r;
        char * const s = format->v[i];
        for (size_t line_idx = 0;
             line_idx < lines;
             line_idx += 1) {
            char * const chr = &s[line_idx * 2];
            uint8_t chr_id;
            if (chr[1] == 'X' && pad) {
                chr_id = 0;
                chr[0] = pad;
            } else if (chr[1] == 'X' && !pad) {
                chr_id = 0;
                chr[0] = 'X';
                chr[1] = '\0';
            } else if (chr[0] == 'X' && pad) {
                chr_id = 0;
                chr[0] = pad;
                chr[1] = 'X';
            } else if (chr[0] == 'X' && !pad) {
                chr_id = 0;
                chr[1] = '\0';
            } else if (sscanf(chr, "%2hhu", &chr_id) != 1 ||
                       chr_id < 1 || chr_id > 22) {
                chr_id = (uint8_t) (-1) >> 1;
            }
            flags[line_idx] = chr_id;
        }
    }
}

void set_format_flag(
        struct format * const format,
        size_t const line_idx,
        uint8_t const flag) {
    uint8_t * const flags = format->v[255];
    if (flags) {
        flags[line_idx] &= 0x7F;
        uint8_t const mask = flag << 7;
        flags[line_idx] |= mask;
    }
}

uint8_t get_format_flag(
        struct format const * const format,
        size_t const line_idx) {
    uint8_t * const flags = format->v[255];
    return flags ? flags[line_idx] >> 7 : (uint8_t) (-1);
}

uint8_t get_format_col(
        struct format const * const format,
        int const t) {
    uint8_t const i = format->u[t];
    return format->t[i] == t ? i : (uint8_t) (-1);
}

int get_format_type(
        struct format const * const format,
        uint8_t const i) {
    return format->t[i];
}

void * get_format_field(
        struct format const * const format,
        size_t const line_idx,
        int const t) {
    uint8_t const i = format->u[t];
    size_t const lines = format->r;
    if (!t || format->t[i] != t || line_idx >= lines) {
        return NULL;
    }
    void * const v = format->v[i];
    if (!v) {
        return NULL;
    }
    return v + (line_idx * sizeof_t(t));
}

uint8_t get_format_chr(
        struct format const * format,
        size_t line_idx) {
    size_t const lines = format->r;
    if (line_idx >= lines) {
        return (uint8_t) (-1);
    }
    uint8_t * const flags = format->v[255];
    return flags ? flags[line_idx] & 0x7F : (uint8_t) (-1);
}

char const * print_t(int const t) {
    static char const * const map[256] = {
            [CHR] = "%.2s",
            [POS] = "%u",
            [RSID] = "%s",
            [PHENO] = "%.5s",
            [A1] = "%s",
            [A2] = "%s",
            [AF] = "%lf",
            [GD] = "%lf",
            [P] = "%lf",
            [BETA] = "%lf",
            [SE] = "%lf",
            [P_REPRO] = "%lf",
            [BETA_REPRO] = "%lf",
            [SE_REPRO] = "%lf",
            [NOM] = "%u"
    };
    return map[t];
}
