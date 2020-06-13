//
// Created by winfield on 2020-06-12.
//

#ifndef GREEDYSNPS_EXCLUDE_H
#define GREEDYSNPS_EXCLUDE_H
#include <stdint.h>
struct Exclude {
    char * a1;
    char * a2;
    uint32_t pos;
    char chr[2];
    uint8_t chr_id;
};
void free_exclude(struct Exclude * exclude);
#endif //GREEDYSNPS_EXCLUDE_H
