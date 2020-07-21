//
// Created by winfield on 2020-06-23.
//

#ifndef PEAKS_FORMAT_H
#define PEAKS_FORMAT_H
#include <stdint.h>
#include <stdio.h>
#include "map.h"
struct format {
    void * v[256];
    enum {
        CHR = 1,
        POS,
        RSID,
        PHENO,
        A1,
        A2,
        AF,
        GD,
        P,
        BETA,
        SE,
        P_REPRO,
        BETA_REPRO,
        SE_REPRO,
        NOM
    } t[256];
    uint8_t s[256];
    uint8_t u[16];
    size_t r;
    uint8_t c;
};
char const * format_t(int t);
size_t sizeof_t(int t);
int map_t(char c);
int parse_format_string(struct format ** format, char const * string);
int read_format_file(struct format * format, FILE * file, uint8_t skip, char const * delim);
void destroy_format(struct format * format);
size_t match_format(struct format const * a, struct format const * b, size_t a_idx, uint8_t sorted);
void set_func_chr_chr(uint8_t chr);
uint8_t func_chr(struct format const * format, size_t line_idx);
void set_func_p_p(double p);
uint8_t func_p(struct format const * format, size_t line_idx);
void set_func_maf_maf(double maf);
uint8_t func_maf(struct format const * format, size_t line_idx);
void set_func_gd_circle(double center, double radius);
uint8_t func_gd(struct format const * format, size_t line_idx);
int mark_format_func(struct format * target, uint8_t func(struct format const *, size_t));
int mark_format(struct format * target, struct format const * source, uint8_t sorted);
int join_format(struct format * subset, struct format const * superset, char const * string, uint8_t sorted);
void parse_format_chr(struct format * format, char pad);
void set_format_flag(struct format * format, size_t line_idx, uint8_t flag);
uint8_t get_format_flag(struct format const * format, size_t line_idx);
uint8_t get_format_col(struct format const * format, int t);
int get_format_type(struct format const * format, uint8_t i);
void * get_format_field(struct format const * format, size_t line_idx, int t);
uint8_t get_format_chr(struct format const * format, size_t line_idx);
char const * print_t(int t);
#endif //PEAKS_FORMAT_H
