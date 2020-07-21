#include <stdint.h>
#include "format.h"
#include "map.h"
#include "maps.h"

struct map const * get_map_p(uint8_t const i) {
    static struct map const * const maps[] = {
        &map0X, &map01, &map02, &map03, &map04,
        &map05, &map06, &map07, &map08, &map09,
        &map10, &map11, &map12, &map13, &map14,
        &map15, &map16, &map17, &map18, &map19,
        &map20, &map21, &map22
    };
    return maps[i];
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

double get_gen_map_cm(struct map const * const map, uint32_t const pos) {
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
        return (res_pos * delta_map) / delta_pos + r_map;
    } else {
        return r_map;
    }
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
