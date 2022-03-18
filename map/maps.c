// peaks. Copyright (c) 2020. Winfield Chen.

#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include "format.h"
#include "map.h"
#include "maps.h"

struct map cust_maps[23];

void destroy_cust_maps() {
    for (uint8_t chr = 0; chr < 23; chr += 1) {
        free(cust_maps[chr].position);
        free(cust_maps[chr].gen_map);
    }
}

void set_cust_maps(struct format const * const format) {
    for (uint8_t chr = 0; chr < 23; chr += 1) {
        uint32_t n = 0;
        size_t const lines = format->r;
        size_t start_idx = 0;
        for (size_t line_idx = start_idx; line_idx < lines; line_idx += 1) {
            if (get_format_chr(format, line_idx) == chr) {
                if (n == 0) {
                    start_idx = line_idx;
                }
                n += 1;
            }
        }
        uint32_t * const position = calloc(n, sizeof(uint32_t));
        if (!position) {
            int errsv = errno;
            perror("position");
            exit(errsv);
        }
        double * const gen_map = calloc(n, sizeof(double));
        if (!gen_map) {
            int errsv = errno;
            perror("gen_map");
            exit(errsv);
        }
        for (size_t line_idx = start_idx; line_idx < start_idx + n; line_idx += 1) {
            size_t i = line_idx - start_idx;
            position[i] = *(uint32_t *)get_format_field(format, line_idx, POS);
            gen_map[i] = *(double *)get_format_field(format, line_idx, GD);
        }
        cust_maps[chr] = (struct map){
                .position = position,
                .comb_rate = NULL,
                .gen_map = gen_map,
                .n = n
        };
    }
}

uint8_t map_rotate;

void set_map_rotate() {
    map_rotate = 1;
}

uint32_t rotate_gen_map_pos(struct map const * const map, uint32_t const pos) {
    uint32_t start = map->position[0];
    uint32_t end = map->position[map->n - 1];
    return (pos + ((end - start) / 2)) % end;
}

struct map const * get_map_p(uint8_t const i) {
    static struct map const * const maps[] = {
        &map0X, &map01, &map02, &map03, &map04,
        &map05, &map06, &map07, &map08, &map09,
        &map10, &map11, &map12, &map13, &map14,
        &map15, &map16, &map17, &map18, &map19,
        &map20, &map21, &map22
    };
    return cust_maps[i].n ? &cust_maps[i] : maps[i];
}

uint32_t binary_search(struct map const * const map, uint32_t const pos) {
    uint32_t l = 0;
    uint32_t r = map->n;
    while (l < r) {
        uint32_t m = (l + r) / 2;
        uint32_t m_pos = map->position[m];
        if (m_pos > pos) {
            r = m;
        } else {
            l = m + 1;
        }
    }
    return r ? r - 1 : 0;
}

double get_gen_map_cm(struct map const * const map, uint32_t pos) {
    if (map_rotate) {
        pos = rotate_gen_map_pos(map, pos);
    }
    uint32_t r = binary_search(map, pos);
    uint32_t r_pos = map->position[r];
    double r_map = map->gen_map[r];
    if (r_pos < pos && r < map->n - 1) {
        uint32_t s = r + 1;
        uint32_t s_pos = map->position[s];
        double s_map = map->gen_map[s];
        uint32_t res_pos = pos - r_pos;
        uint32_t delta_pos = s_pos - r_pos;
        double delta_map = s_map - r_map;
        r_map += (res_pos * delta_map) / delta_pos;
    }
    return r_map;
}

double get_gen_map_dist(struct map const * const map, uint32_t const a, uint32_t const b) {
    if (b > a) {
        return get_gen_map_cm(map, b) - get_gen_map_cm(map, a);
    } else if (b < a) {
        return get_gen_map_cm(map, a) - get_gen_map_cm(map, b);
    } else {
        return 0.0;
    }
}

int calc_gd(struct format const * const format) {
    size_t const lines = format->r;
    for (size_t line_idx = 0; line_idx < lines; line_idx += 1) {
        *(double *)get_format_field(format, line_idx, GD) =
                get_gen_map_cm(
                        get_map_p(
                                get_format_chr(format, line_idx)),
                        *(uint32_t *)get_format_field(
                                format,
                                line_idx,
                                POS));
    }
    return 0;
}
