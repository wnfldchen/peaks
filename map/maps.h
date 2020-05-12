//
// Created by winfield on 2020-05-11.
//

#ifndef GREEDYSNPS_MAPS_H
#define GREEDYSNPS_MAPS_H
#include "map.h"
typedef enum {
    MAP_0X,
    MAP_01,
    MAP_02,
    MAP_03,
    MAP_04,
    MAP_05,
    MAP_06,
    MAP_07,
    MAP_08,
    MAP_09,
    MAP_10,
    MAP_11,
    MAP_12,
    MAP_13,
    MAP_14,
    MAP_15,
    MAP_16,
    MAP_17,
    MAP_18,
    MAP_19,
    MAP_20,
    MAP_21,
    MAP_22
} Map_e;
Map const * get_map_p(Map_e map);
double get_gen_map_dist(Map const * map, uint32_t a, uint32_t b);
#endif //GREEDYSNPS_MAPS_H
