//
// Created by winfield on 2020-06-12.
//

#include <stdlib.h>
#include "variant.h"

void free_variant(struct Variant *variant) {
    free(variant->rsid);
    free(variant->a1);
    free(variant->a2);
}
