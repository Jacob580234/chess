cmake_minimum_required(VERSION 4.0)
project(chess_v2 C)

set(CMAKE_C_STANDARD 17)

add_executable(chess_v2 game/main.c
        game/attackMaps.c
        game/attackMaps.h
        game/print.c
        game/print.h
        game/FEN.c
        game/FEN.h
        game/main.h
        game/move.c
        game/move.h
        engine/moveGen.c
        engine/moveGen.h
        game/events.c
        game/events.h)
