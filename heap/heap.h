#ifndef PEAKS_HEAP_H
#define PEAKS_HEAP_H
#include <stdint.h>
#include "format.h"

extern struct heap {
    struct format * format;
    size_t * array;
    size_t len;
    uint8_t const chr;
} heaps[];
int init_heaps(struct format * format, uint8_t filter);
void make_heaps();
size_t extract_heap(struct heap * heap);
void batch_delete_heap(struct heap * heap);
int mark_heap_func(struct heap * target, uint8_t func(struct format const *, size_t));
uint32_t acc_heap_nom(struct heap const * target);
void print_heap_nonleads(struct heap const * target, FILE * output_file);
void destroy_heaps();
#endif //PEAKS_HEAP_H
