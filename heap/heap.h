//
// Created by winfield on 2020-05-12.
//

#ifndef GREEDYSNPS_HEAP_H
#define GREEDYSNPS_HEAP_H
#include <stdint.h>
struct Node {
    char * rsid;
    char * a1;
    char * a2;
    double p;
    double af;
    double gd;
    uint32_t pos;
    uint8_t chr_id;
    uint8_t nom;
    char chr[2];
    char pheno[5];
    uint8_t flag;
};
struct Heap {
    struct Node * array;
    uint32_t n;
};
extern struct Heap heaps[23];
void emplace_array(struct Node node);
void make_heaps();
struct Node extract_heap(struct Heap * heap);
void batch_delete_heap(struct Heap * heap);
void free_node(struct Node const * node);
#endif //GREEDYSNPS_HEAP_H
