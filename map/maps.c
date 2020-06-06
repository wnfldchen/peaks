//
// Created by winfield on 2020-05-11.
//

#include <stddef.h>
#include <stdint.h>
#include "map.h"
#include "map0X.h"
#include "map01.h"
#include "map02.h"
#include "map03.h"
#include "map04.h"
#include "map05.h"
#include "map06.h"
#include "map07.h"
#include "map08.h"
#include "map09.h"
#include "map10.h"
#include "map11.h"
#include "map12.h"
#include "map13.h"
#include "map14.h"
#include "map15.h"
#include "map16.h"
#include "map17.h"
#include "map18.h"
#include "map19.h"
#include "map20.h"
#include "map21.h"
#include "map22.h"
#include "maps.h"

struct Map const * get_map_p(uint8_t const i) {
    static struct Map const * const maps[] = {
        &map0X, &map01, &map02, &map03, &map04,
        &map05, &map06, &map07, &map08, &map09,
        &map10, &map11, &map12, &map13, &map14,
        &map15, &map16, &map17, &map18, &map19,
        &map20, &map21, &map22
    };
    return maps[i];
}

uint32_t binary_search(struct Map const * const map, uint32_t const pos) {
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

double get_gen_map_cm(struct Map const * const map, uint32_t const pos) {
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

double get_gen_map_dist(struct Map const * const map, uint32_t const a, uint32_t const b) {
    if (b > a) {
        return get_gen_map_cm(map, b) - get_gen_map_cm(map, a);
    } else if (b < a) {
        return get_gen_map_cm(map, a) - get_gen_map_cm(map, b);
    } else {
        return 0.0;
    }
}
