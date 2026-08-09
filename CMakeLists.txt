cmake_minimum_required(VERSION 4.0)
project(chess_v2 C)

set(CMAKE_C_STANDARD 17)

add_executable(chess_v2 main.c
        bitboardDatabase.c
        bitboardDatabase.h
        print.c
        print.h
        FEN.c
        FEN.h
        main.h
        move.c
        move.h)
