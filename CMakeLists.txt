cmake_minimum_required(VERSION 4.0)
project(chess C)

set(CMAKE_C_STANDARD 17)

add_executable(chess game/main.c
        game/maps.c
        game/maps.h
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
        game/events.h
        test/performance/perf.c
        test/logic/test.c
        test/performance/perf.h
        test/logic/test.h
        engine/moveMaking.c
        engine/moveMaking.h
        asm.h
        test/logic/moveGeneration/pawnMoveGenTest.c
        test/logic/moveGeneration/pawnMoveGenTest.h
        engine/search.c
        engine/search.h
        engine/evaluation.c
        engine/evaluation.h
        test/logic/moveGeneration/kingMoveGenTest.c
        test/logic/moveGeneration/kingMoveGenTest.h)
