//
// Created by winfield on 2020-05-12.
//

#include <stdint.h>
#include <stdlib.h>
#include "heap.h"
struct Heap heaps[23]; // Zero initialized by default
void emplace_array(struct Node const node) {
    struct Heap * heap = &heaps[node.chr_id];
    heap->array[heap->n] = node;
    heap->n += 1;
}

void max_heapify(struct Heap * const heap, uint32_t const i) {
    uint32_t const n = heap->n;
    uint32_t const l = 2 * i + 1;
    uint32_t const r = 2 * i + 2;
    uint32_t m = i;
    if (l < n &&
        heap->array[l].p > heap->array[m].p) {
        m = l;
    }
    if (r < n &&
        heap->array[r].p > heap->array[m].p) {
        m = r;
    }
    if (m != i) {
        struct Node const temp = heap->array[i];
        heap->array[i] = heap->array[m];
        heap->array[m] = temp;
        max_heapify(heap, m);
    }
}

void heapify(uint8_t const chr) {
    struct Heap * const heap = &heaps[chr];
    uint32_t const n = heap->n;
    if (n > 1) {
        for (uint32_t i = (n - 2) / 2; i != (uint32_t) (-1); i -= 1) {
            max_heapify(heap, i);
        }
    }
}

void make_heaps() {
    for (uint8_t chr = 0; chr < 23; chr += 1) {
        heapify(chr);
    }
}

void filter_heap(struct Heap * const heap, uint32_t const i) {
    uint32_t p = (i - 1) / 2;
    if (i > 0 &&
        heap->array[i].p > heap->array[p].p) {
        struct Node temp = heap->array[p];
        heap->array[p] = heap->array[i];
        heap->array[i] = temp;
        filter_heap(heap, p);
    }
}

void delete_heap(struct Heap * const heap, uint32_t const i) {
    heap->n -= 1;
    if (i < heap->n) {
        heap->array[i] = heap->array[heap->n];
        if (heap->n > 1) {
            if (i == 0 ||
                heap->array[i].p < heap->array[(i - 1) / 2].p) {
                max_heapify(heap, i);
            } else {
                filter_heap(heap, i);
            }
        }
    }
}

struct Node extract_heap(struct Heap * const heap) {
    struct Node root = heap->array[0];
    delete_heap(heap, 0);
    return root;
}

void batch_delete_heap(struct Heap * const heap) {
    for (uint32_t i = 0; i < heap->n; i += 1) {
        if (heap->array[i].flag) {
            while (i < heap->n - 1 && heap->array[heap->n - 1].flag) {
                heap->n -= 1;
            }
            if (i < heap->n - 1) {
                heap->array[i] = heap->array[heap->n - 1];
            }
            heap->n -= 1;
        }
    }
    uint32_t const n = heap->n;
    if (n > 1) {
        for (uint32_t i = (n - 2) / 2; i != (uint32_t) (-1); i -= 1) {
            max_heapify(heap, i);
        }
    }
}

void free_node(struct Node const * const node) {
    free(node->rsid);
    free(node->a1);
    free(node->a2);
}
