//
// Created by winfield on 2020-06-12.
//

#ifndef GREEDYSNPS_VARIANT_H
#define GREEDYSNPS_VARIANT_H
#include <stdint.h>
struct Variant {
    char * rsid;
    char * a1;
    char * a2;
    double af;
    uint32_t pos;
    char chr[2];
    uint8_t chr_id;
};
void free_variant(struct Variant * variant);
#endif //GREEDYSNPS_VARIANT_H
