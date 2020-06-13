//
// Created by winfield on 2020-06-12.
//

#include <stdlib.h>
#include "exclude.h"

void free_exclude(struct Exclude *exclude) {
    free(exclude->a1);
    free(exclude->a2);
}
