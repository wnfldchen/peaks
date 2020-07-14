//
// Created by winfield on 2020-05-11.
//

#ifndef GREEDYSNPS_MAPS_H
#define GREEDYSNPS_MAPS_H
#include <stdint.h>
#include "map.h"
#include "format.h"
struct map const * get_map_p(uint8_t i);
double get_gen_map_cm(struct map const * map, uint32_t pos);
double get_gen_map_dist(struct map const * map, uint32_t a, uint32_t b);
int calc_gd(struct format const * format);
#endif //GREEDYSNPS_MAPS_H
