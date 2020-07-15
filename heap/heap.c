//
// Created by winfield on 2020-05-12.
//

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "heap.h"
#include "format.h"

struct heap heaps[23] = {
        [0].chr = 0,
        [1].chr = 1,
        [2].chr = 2,
        [3].chr = 3,
        [4].chr = 4,
        [5].chr = 5,
        [6].chr = 6,
        [7].chr = 7,
        [8].chr = 8,
        [9].chr = 9,
        [10].chr = 10,
        [11].chr = 11,
        [12].chr = 12,
        [13].chr = 13,
        [14].chr = 14,
        [15].chr = 15,
        [16].chr = 16,
        [17].chr = 17,
        [18].chr = 18,
        [19].chr = 19,
        [20].chr = 20,
        [21].chr = 21,
        [22].chr = 22
};

int init_heaps(
        struct format * const format,
        uint8_t const filter) {
    size_t const lines = format->r;
    size_t lens[23] = {0};
    for (size_t line_idx = 0; line_idx < lines; line_idx += 1) {
        uint8_t const chr = get_format_chr(format, line_idx);
        if (!filter ||
            !get_format_flag(format, line_idx)) {
            lens[chr] += 1;
        }
    }
    for (uint8_t chr = 0; chr < 23; chr += 1) {
        size_t const len = lens[chr];
        if (len) {
            heaps[chr].format = format;
            heaps[chr].array = calloc(len, sizeof(size_t));
            if (!heaps[chr].array) {
                int errsv = errno;
                perror("init_heaps");
                return -errsv;
            }
        }
    }
    for (size_t line_idx = 0; line_idx < lines; line_idx += 1) {
        uint8_t const chr = get_format_chr(format, line_idx);
        if (!filter ||
            !get_format_flag(format, line_idx)) {
            heaps[chr].array[heaps[chr].len++] = line_idx;
        }
    }
    return 0;
}

void max_heapify(struct heap * const heap, size_t const i) {
    size_t const n = heap->len;
    size_t const l = 2 * i + 1;
    size_t const r = 2 * i + 2;
    size_t m = i;
    if (l < n &&
        *(double *)get_format_field(heap->format, heap->array[l], P)
        >
        *(double *)get_format_field(heap->format, heap->array[m], P)) {
        m = l;
    }
    if (r < n &&
        *(double *)get_format_field(heap->format, heap->array[r], P)
        >
        *(double *)get_format_field(heap->format, heap->array[m], P)) {
        m = r;
    }
    if (m != i) {
        size_t const temp = heap->array[i];
        heap->array[i] = heap->array[m];
        heap->array[m] = temp;
        max_heapify(heap, m);
    }
}

void heapify(uint8_t const chr) {
    struct heap * const heap = &heaps[chr];
    size_t const n = heap->len;
    if (n > 1) {
        for (size_t i = (n - 2) / 2; i != (size_t) (-1); i -= 1) {
            max_heapify(heap, i);
        }
    }
}

void make_heaps() {
    for (uint8_t chr = 0; chr < 23; chr += 1) {
        heapify(chr);
    }
}

void filter_heap(struct heap * const heap, size_t const i) {
    size_t p = (i - 1) / 2;
    if (i > 0 &&
        *(double *)get_format_field(heap->format, heap->array[i], P)
        >
        *(double *)get_format_field(heap->format, heap->array[p], P)) {
        size_t const temp = heap->array[p];
        heap->array[p] = heap->array[i];
        heap->array[i] = temp;
        filter_heap(heap, p);
    }
}

void delete_heap(struct heap * const heap, size_t const i) {
    heap->len -= 1;
    if (i < heap->len) {
        heap->array[i] = heap->array[heap->len];
        if (heap->len > 1) {
            if (i == 0 ||
                *(double *)get_format_field(heap->format, heap->array[i], P)
                <
                *(double *)get_format_field(heap->format, heap->array[(i - 1) / 2], P)) {
                max_heapify(heap, i);
            } else {
                filter_heap(heap, i);
            }
        }
    }
}

size_t extract_heap(struct heap * const heap) {
    size_t const root = heap->array[0];
    delete_heap(heap, 0);
    return root;
}

void batch_delete_heap(struct heap * const heap) {
    for (size_t i = 0; i < heap->len; i += 1) {
        if (get_format_flag(heap->format, heap->array[i])) {
            while (i < heap->len - 1 &&
                   get_format_flag(
                           heap->format,
                           heap->array[heap->len - 1])) {
                heap->len -= 1;
            }
            if (i < heap->len - 1) {
                heap->array[i] = heap->array[heap->len - 1];
            }
            heap->len -= 1;
        }
    }
    size_t const n = heap->len;
    if (n > 1) {
        for (size_t i = (n - 2) / 2; i != (size_t) (-1); i -= 1) {
            max_heapify(heap, i);
        }
    }
}

int mark_heap_func(
        struct heap * const target,
        uint8_t func(struct format const *, size_t)) {
    size_t const lines = target->len;
    for (size_t line_idx = 0; line_idx < lines; line_idx += 1) {
        if (!get_format_flag(target->format, target->array[line_idx]) &&
            func(target->format, target->array[line_idx])) {
            set_format_flag(target->format, target->array[line_idx], 1);
        }
    }
    return 0;
}

uint32_t acc_heap_nom(struct heap const * const target) {
    uint32_t nominal_sum = 0;
    size_t const lines = target->len;
    for (size_t line_idx = 0; line_idx < lines; line_idx += 1) {
        if (get_format_flag(target->format, target->array[line_idx])) {
            nominal_sum += *(uint32_t *)get_format_field(target->format, target->array[line_idx], NOM);
        }
    }
    return nominal_sum;
}

void print_heap_nonleads(struct heap const * const target, FILE * const output_file) {
    size_t const lines = target->len;
    for (size_t line_idx = 0; line_idx < lines; line_idx += 1) {
        if (get_format_flag(target->format, target->array[line_idx])) {
            fprintf(output_file,
                    " %s %.5s",
                    *(char **)get_format_field(target->format, target->array[line_idx], RSID),
                    (char *)get_format_field(target->format, target->array[line_idx], PHENO));
        }
    }
}

void destroy_heaps() {
    for (uint8_t chr = 0; chr < 23; chr += 1) {
        size_t const len = heaps[chr].len;
        if (len) {
            free(heaps[chr].array);
        }
    }
}
