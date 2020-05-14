//
// Created by winfield on 2020-05-12.
//

#include <stdint.h>
#include "heap.h"
Heap heaps[23]; // Zero initialized by default
void emplace_array(Association const association) {
    Heap * heap = &heaps[association.chr_2hhu];
    heap->array[heap->n] = association;
    heap->n += 1;
}

void max_heapify(Heap * const heap, uint16_t const i) {
    uint16_t const n = heap->n;
    uint16_t const l = 2 * i + 1;
    uint16_t const r = 2 * i + 2;
    uint16_t m = i;
    if (l < n &&
        heap->array[l].p_lf > heap->array[m].p_lf) {
        m = l;
    }
    if (r < n &&
        heap->array[r].p_lf > heap->array[m].p_lf) {
        m = r;
    }
    if (m != i) {
        Association const temp = heap->array[i];
        heap->array[i] = heap->array[m];
        heap->array[m] = temp;
        max_heapify(heap, m);
    }
}

void heapify(uint8_t const chr) {
    Heap * const heap = &heaps[chr];
    uint16_t const n = heap->n;
    if (n > 1) {
        for (uint16_t i = (n - 2) / 2; i != (uint16_t) (-1); i -= 1) {
            max_heapify(heap, i);
        }
    }
}

void make_heaps() {
    for (uint8_t chr = 0; chr < 23; chr += 1) {
        heapify(chr);
    }
}

void filter_heap(Heap * const heap, uint16_t const i) {
    uint16_t p = (i - 1) / 2;
    if (i > 0 &&
        heap->array[i].p_lf > heap->array[p].p_lf) {
        Association temp = heap->array[p];
        heap->array[p] = heap->array[i];
        heap->array[i] = temp;
        filter_heap(heap, p);
    }
}

void delete_heap(Heap * const heap, uint16_t const i) {
    heap->n -= 1;
    if (i < heap->n) {
        heap->array[i] = heap->array[heap->n];
        if (heap->n > 1) {
            if (i == 0 ||
                heap->array[i].p_lf < heap->array[(i - 1) / 2].p_lf) {
                max_heapify(heap, i);
            } else {
                filter_heap(heap, i);
            }
        }
    }
}

Association extract_heap(Heap * const heap) {
    Association root = heap->array[0];
    delete_heap(heap, 0);
    return root;
}

void batch_delete_heap(Heap * const heap, uint8_t * const delete) {
    for (uint16_t i = 0; i < heap->n; i += 1) {
        if (delete[i]) {
            heap->n -= 1;
            if (i < heap->n) {
                heap->array[i] = heap->array[heap->n];
                delete[i] = delete[heap->n];
                i -= 1;
            }
        }
    }
    uint16_t const n = heap->n;
    if (n > 1) {
        for (uint16_t i = (n - 2) / 2; i != (uint16_t) (-1); i -= 1) {
            max_heapify(heap, i);
        }
    }
}
