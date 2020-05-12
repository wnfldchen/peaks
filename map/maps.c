//
// Created by winfield on 2020-05-11.
//

#include <stddef.h>
#include "map.h"
#include "map0X.h"
#include "map01.h"
#include "map02.h"
#include "map03.h"
#include "map04.h"
#include "map05.h"
#include "map06.h"
#include "map07.h"
#include "map08.h"
#include "map09.h"
#include "map10.h"
#include "map11.h"
#include "map12.h"
#include "map13.h"
#include "map14.h"
#include "map15.h"
#include "map16.h"
#include "map17.h"
#include "map18.h"
#include "map19.h"
#include "map20.h"
#include "map21.h"
#include "map22.h"
#include "maps.h"

Map const * get_map_p(Map_e map) {
    switch (map) {
        case MAP_0X:
            return &map0X;
        case MAP_01:
            return &map01;
        case MAP_02:
            return &map02;
        case MAP_03:
            return &map03;
        case MAP_04:
            return &map04;
        case MAP_05:
            return &map05;
        case MAP_06:
            return &map06;
        case MAP_07:
            return &map07;
        case MAP_08:
            return &map08;
        case MAP_09:
            return &map09;
        case MAP_10:
            return &map10;
        case MAP_11:
            return &map11;
        case MAP_12:
            return &map12;
        case MAP_13:
            return &map13;
        case MAP_14:
            return &map14;
        case MAP_15:
            return &map15;
        case MAP_16:
            return &map16;
        case MAP_17:
            return &map17;
        case MAP_18:
            return &map18;
        case MAP_19:
            return &map19;
        case MAP_20:
            return &map20;
        case MAP_21:
            return &map21;
        case MAP_22:
            return &map22;
        default:
            return NULL;
    }
}
