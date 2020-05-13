//
// Created by winfield on 2020-05-12.
//

#ifndef GREEDYSNPS_HEAP_H
#define GREEDYSNPS_HEAP_H
#include <stdint.h>
typedef struct {
    char rsid_255s[256];
    char pheno_5c[5];
    char chr_2c[2];
    uint8_t chr_2hhu;
    uint32_t pos_u;
    char a1_255s[256];
    char a2_255s[256];
    double p_lf;
    double beta_lf;
    double se_lf;
    double p_repro_lf;
    double beta_repro_lf;
    double se_repro_lf;
    uint8_t nominal_hhu;
} Association;
typedef struct {
    uint16_t n;
    Association array[16384];
} Heap;
extern Heap heaps[23];
void emplace_array(Association association);
void make_heaps();
Association extract_heap(Heap * heap);
void delete_heap(Heap * heap, uint8_t i);
#endif //GREEDYSNPS_HEAP_H
