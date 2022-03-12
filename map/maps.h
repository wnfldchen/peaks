// peaks. Copyright (c) 2020. Winfield Chen.

#ifndef PEAKS_MAPS_H
#define PEAKS_MAPS_H
#include <stdint.h>
#include "map.h"
#include "format.h"
void destroy_cust_maps();
void set_cust_maps(struct format const * format);
struct map const * get_map_p(uint8_t i);
double get_gen_map_cm(struct map const * map, uint32_t pos);
double get_gen_map_dist(struct map const * map, uint32_t a, uint32_t b);
int calc_gd(struct format const * format);
#endif //PEAKS_MAPS_H
