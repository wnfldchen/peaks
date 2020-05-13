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

Map const * get_map_p(Map_e const map) {
    switch (map) {
        case MAP_0X:
            return &map0X;
        case MAP_01:
            return &map01;
        case MAP_02:
            return &map02;
        case MAP_03:
            return &map03;
        case MAP_04:
            return &map04;
        case MAP_05:
            return &map05;
        case MAP_06:
            return &map06;
        case MAP_07:
            return &map07;
        case MAP_08:
            return &map08;
        case MAP_09:
            return &map09;
        case MAP_10:
            return &map10;
        case MAP_11:
            return &map11;
        case MAP_12:
            return &map12;
        case MAP_13:
            return &map13;
        case MAP_14:
            return &map14;
        case MAP_15:
            return &map15;
        case MAP_16:
            return &map16;
        case MAP_17:
            return &map17;
        case MAP_18:
            return &map18;
        case MAP_19:
            return &map19;
        case MAP_20:
            return &map20;
        case MAP_21:
            return &map21;
        case MAP_22:
            return &map22;
        default:
            return NULL;
    }
}

uint32_t binary_search(Map const * const map, uint32_t const pos) {
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

double get_gen_map_cm(Map const * const map, uint32_t const pos) {
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

double get_gen_map_dist(Map const * const map, uint32_t const a, uint32_t const b) {
    if (b > a) {
        return get_gen_map_cm(map, b) - get_gen_map_cm(map, a);
    } else if (b < a) {
        return get_gen_map_cm(map, a) - get_gen_map_cm(map, b);
    } else {
        return 0.0;
    }
}
