//
// Created by winfield on 2020-05-12.
//

#include "heap.h"
Heap heaps[23]; // Zero initialzed by default
void emplace_array(Association association) {
    Heap * heap = &heaps[association.chr_2hhu];
    heap->array[heap->n] = association;
    heap->n += 1;
}
