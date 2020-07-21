cmake_minimum_required(VERSION 3.16)
project(peaks C)
set(CMAKE_C_STANDARD 11)
add_executable(peaks)

target_sources(
        peaks
        PRIVATE
        main.c
        format/format.c
        format/format.h
        heap/heap.c
        heap/heap.h
        map/map.h
        map/map0X.c
        map/map01.c
        map/map02.c
        map/map03.c
        map/map04.c
        map/map05.c
        map/map06.c
        map/map07.c
        map/map08.c
        map/map09.c
        map/map10.c
        map/map11.c
        map/map12.c
        map/map13.c
        map/map14.c
        map/map15.c
        map/map16.c
        map/map17.c
        map/map18.c
        map/map19.c
        map/map20.c
        map/map21.c
        map/map22.c
        map/maps.c
        map/maps.h
        options/options.c
        options/options.h
)

target_include_directories(
        peaks
        PRIVATE
        format
        heap
        map
        options
)
