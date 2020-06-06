//
// Created by winfield on 2020-05-11.
//

#ifndef GREEDYSNPS_MAP_H
#define GREEDYSNPS_MAP_H
#include <stdint.h>
struct Map {
    uint32_t const * const position;
    double const * const comb_rate;
    double const * const gen_map;
    uint32_t const n;
};
#endif //GREEDYSNPS_MAP_H
