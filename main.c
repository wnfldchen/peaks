#include <stdio.h>
#include "map/maps.h"
int main() {
    puts("greedysnps");
    puts("Maps:");
    for (Map_e map = MAP_0X; map <= MAP_22; map += 1) {
        printf("%d %p\n", map, get_map_p(map));
    }
    return 0;
}
