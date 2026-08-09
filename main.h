#ifndef CHESS_V2_MAIN_H
#define CHESS_V2_MAIN_H
#include <stdint.h>
#include <stdlib.h>

#define NUM_PIECE_TYPES 6
#define FILES 8
#define RANKS 8
#define NUM_SQUARES 64
#define BIT(n) (1ULL << (n))
#define THROW_EXCEPTION do {                                                                                     \
                            printf("\n\nException from %s, line %d in %s\n", __FILE_NAME__, __LINE__, __func__); \
                            exit(EXIT_FAILURE);                                                                  \
                        } while(0)

typedef enum: unsigned int { white, black } side; // unsigned, otherwise 1-bit would mean value = -1 given bit field width
typedef enum: int { false, true } bool;
typedef enum: int { pawn, knight, bishop, rook, queen, king, noPiece } piece;
typedef enum: int { invalid = -1,
                    A1, B1, C1, D1, E1, F1, G1, H1,
                    A2, B2, C2, D2, E2, F2, G2, H2,
                    A3, B3, C3, D3, E3, F3, G3, H3,
                    A4, B4, C4, D4, E4, F4, G4, H4,
                    A5, B5, C5, D5, E5, F5, G5, H5,
                    A6, B6, C6, D6, E6, F6, G6, H6,
                    A7, B7, C7, D7, E7, F7, G7, H7,
                    A8, B8, C8, D8, E8, F8, G8, H8 } square;

typedef struct {
    uint64_t bitboard[2][NUM_PIECE_TYPES];
    uint64_t piecesForSide[2]; // all pieces of a certain color. used to check blocking / capturing. Subject for removal
    uint64_t allPieces;
    piece pieceLookup[NUM_SQUARES];
    uint8_t kingIndex[2];

    side playerToMove : 1; // bit fields serve no purpose and are purely for semantics (also subject for removal due to "side" enum constraint)
    uint8_t castlingRights : 6; // inadequate on its own; helper function to compute castling rights needed
    square enPassantIndex; // square to which a pawn would be after capturing en passant (the one the mover jumped over)
    uint8_t fiftyMoveRule;
} gameState_s;

uint64_t clear (int index, uint64_t* bitboard);
uint64_t set(int index, uint64_t* bitboard);
bool read(int index, uint64_t bitboard);
square getIndexFromInput(char notation[]);
uint64_t getAllPiecesForSide(side side, gameState_s* state);



#endif //CHESS_V2_MAIN_H