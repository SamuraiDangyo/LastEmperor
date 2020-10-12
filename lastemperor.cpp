/*
LastEmperor, a Chess960 move generator tool (Derived from Sapeli 1.67)
Copyright (C) 2019-2020 Toni Helminen

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

// Headers

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <immintrin.h>

// Constants

#define NAME         "LastEmperor 1.11"
#define MAX_MOVES    218 // Legal moves
#define MAX_TOKENS   30
#define STARTPOS     "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -"

// Macros

#define WHITE()      (BRD->white[0] | BRD->white[1] | BRD->white[2] | BRD->white[3] | BRD->white[4] | BRD->white[5])
#define BLACK()      (BRD->black[0] | BRD->black[1] | BRD->black[2] | BRD->black[3] | BRD->black[4] | BRD->black[5])
#define BOTH()       (WHITE() | BLACK())
#define X(bb)        ((bb) & 7)
#define Y(bb)        ((bb) >> 3)
#define RESET(memo)  memset((memo), 0, sizeof((memo)))

// Structs

typedef struct {
  uint64_t
    white[6],  // White bitboards
    black[6];  // Black bitboards
  char
    board[64], // Pieces black and white
    epsq;      // En passant square
  uint8_t
    castle;    // Castling rights
} BOARD_T;

typedef struct {
  uint64_t
    hash,
    nodes;
  int
    depth;
} HASH_T;

// Consts

static const int
  ROOK_VECTORS[8]        = {1,0,0,1,0,-1,-1,0},
  BISHOP_VECTORS[8]      = {1,1,-1,-1,1,-1,-1,1},
  KING_VECTORS[2 * 8]    = {1,0,0,1,0,-1,-1,0,1,1,-1,-1,1,-1,-1,1},
  KNIGHT_VECTORS[2 * 8]  = {2,1,-2,1,2,-1,-2,-1,1,2,-1,2,1,-2,-1,-2};

static const uint64_t
  ROOK_MASK[64] =
    {0x101010101017eULL,0x202020202027cULL,0x404040404047aULL,0x8080808080876ULL,0x1010101010106eULL,0x2020202020205eULL,0x4040404040403eULL,0x8080808080807eULL,
     0x1010101017e00ULL,0x2020202027c00ULL,0x4040404047a00ULL,0x8080808087600ULL,0x10101010106e00ULL,0x20202020205e00ULL,0x40404040403e00ULL,0x80808080807e00ULL,
     0x10101017e0100ULL,0x20202027c0200ULL,0x40404047a0400ULL,0x8080808760800ULL,0x101010106e1000ULL,0x202020205e2000ULL,0x404040403e4000ULL,0x808080807e8000ULL,
     0x101017e010100ULL,0x202027c020200ULL,0x404047a040400ULL,0x8080876080800ULL,0x1010106e101000ULL,0x2020205e202000ULL,0x4040403e404000ULL,0x8080807e808000ULL,
     0x1017e01010100ULL,0x2027c02020200ULL,0x4047a04040400ULL,0x8087608080800ULL,0x10106e10101000ULL,0x20205e20202000ULL,0x40403e40404000ULL,0x80807e80808000ULL,
     0x17e0101010100ULL,0x27c0202020200ULL,0x47a0404040400ULL,0x8760808080800ULL,0x106e1010101000ULL,0x205e2020202000ULL,0x403e4040404000ULL,0x807e8080808000ULL,
     0x7e010101010100ULL,0x7c020202020200ULL,0x7a040404040400ULL,0x76080808080800ULL,0x6e101010101000ULL,0x5e202020202000ULL,0x3e404040404000ULL,0x7e808080808000ULL,
     0x7e01010101010100ULL,0x7c02020202020200ULL,0x7a04040404040400ULL,0x7608080808080800ULL,0x6e10101010101000ULL,0x5e20202020202000ULL,0x3e40404040404000ULL,0x7e80808080808000ULL},
  ROOK_MOVE_MAGICS[64] =
    {0x101010101017eULL,0x202020202027cULL,0x404040404047aULL,0x8080808080876ULL,0x1010101010106eULL,0x2020202020205eULL,0x4040404040403eULL,0x8080808080807eULL,
     0x1010101017e00ULL,0x2020202027c00ULL,0x4040404047a00ULL,0x8080808087600ULL,0x10101010106e00ULL,0x20202020205e00ULL,0x40404040403e00ULL,0x80808080807e00ULL,
     0x10101017e0100ULL,0x20202027c0200ULL,0x40404047a0400ULL,0x8080808760800ULL,0x101010106e1000ULL,0x202020205e2000ULL,0x404040403e4000ULL,0x808080807e8000ULL,
     0x101017e010100ULL,0x202027c020200ULL,0x404047a040400ULL,0x8080876080800ULL,0x1010106e101000ULL,0x2020205e202000ULL,0x4040403e404000ULL,0x8080807e808000ULL,
     0x1017e01010100ULL,0x2027c02020200ULL,0x4047a04040400ULL,0x8087608080800ULL,0x10106e10101000ULL,0x20205e20202000ULL,0x40403e40404000ULL,0x80807e80808000ULL,
     0x17e0101010100ULL,0x27c0202020200ULL,0x47a0404040400ULL,0x8760808080800ULL,0x106e1010101000ULL,0x205e2020202000ULL,0x403e4040404000ULL,0x807e8080808000ULL,
     0x7e010101010100ULL,0x7c020202020200ULL,0x7a040404040400ULL,0x76080808080800ULL,0x6e101010101000ULL,0x5e202020202000ULL,0x3e404040404000ULL,0x7e808080808000ULL,
     0x7e01010101010100ULL,0x7c02020202020200ULL,0x7a04040404040400ULL,0x7608080808080800ULL,0x6e10101010101000ULL,0x5e20202020202000ULL,0x3e40404040404000ULL,0x7e80808080808000ULL},
  BISHOP_MASK[64] =
    {0x40201008040200ULL,0x402010080400ULL,0x4020100a00ULL,0x40221400ULL,0x2442800ULL,0x204085000ULL,0x20408102000ULL,0x2040810204000ULL,
     0x20100804020000ULL,0x40201008040000ULL,0x4020100a0000ULL,0x4022140000ULL,0x244280000ULL,0x20408500000ULL,0x2040810200000ULL,0x4081020400000ULL,
     0x10080402000200ULL,0x20100804000400ULL,0x4020100a000a00ULL,0x402214001400ULL,0x24428002800ULL,0x2040850005000ULL,0x4081020002000ULL,0x8102040004000ULL,
     0x8040200020400ULL,0x10080400040800ULL,0x20100a000a1000ULL,0x40221400142200ULL,0x2442800284400ULL,0x4085000500800ULL,0x8102000201000ULL,0x10204000402000ULL,
     0x4020002040800ULL,0x8040004081000ULL,0x100a000a102000ULL,0x22140014224000ULL,0x44280028440200ULL,0x8500050080400ULL,0x10200020100800ULL,0x20400040201000ULL,
     0x2000204081000ULL,0x4000408102000ULL,0xa000a10204000ULL,0x14001422400000ULL,0x28002844020000ULL,0x50005008040200ULL,0x20002010080400ULL,0x40004020100800ULL,
     0x20408102000ULL,0x40810204000ULL,0xa1020400000ULL,0x142240000000ULL,0x284402000000ULL,0x500804020000ULL,0x201008040200ULL,0x402010080400ULL,
     0x2040810204000ULL,0x4081020400000ULL,0xa102040000000ULL,0x14224000000000ULL,0x28440200000000ULL,0x50080402000000ULL,0x20100804020000ULL,0x40201008040200ULL},
  BISHOP_MOVE_MAGICS[64] =
    {0x40201008040200ULL,0x402010080400ULL,0x4020100a00ULL,0x40221400ULL,0x2442800ULL,0x204085000ULL,0x20408102000ULL,0x2040810204000ULL,
     0x20100804020000ULL,0x40201008040000ULL,0x4020100a0000ULL,0x4022140000ULL,0x244280000ULL,0x20408500000ULL,0x2040810200000ULL,0x4081020400000ULL,
     0x10080402000200ULL,0x20100804000400ULL,0x4020100a000a00ULL,0x402214001400ULL,0x24428002800ULL,0x2040850005000ULL,0x4081020002000ULL,0x8102040004000ULL,
     0x8040200020400ULL,0x10080400040800ULL,0x20100a000a1000ULL,0x40221400142200ULL,0x2442800284400ULL,0x4085000500800ULL,0x8102000201000ULL,0x10204000402000ULL,
     0x4020002040800ULL,0x8040004081000ULL,0x100a000a102000ULL,0x22140014224000ULL,0x44280028440200ULL,0x8500050080400ULL,0x10200020100800ULL,0x20400040201000ULL,
     0x2000204081000ULL,0x4000408102000ULL,0xa000a10204000ULL,0x14001422400000ULL,0x28002844020000ULL,0x50005008040200ULL,0x20002010080400ULL,0x40004020100800ULL,
     0x20408102000ULL,0x40810204000ULL,0xa1020400000ULL,0x142240000000ULL,0x284402000000ULL,0x500804020000ULL,0x201008040200ULL,0x402010080400ULL,
     0x2040810204000ULL,0x4081020400000ULL,0xa102040000000ULL,0x14224000000000ULL,0x28440200000000ULL,0x50080402000000ULL,0x20100804020000ULL,0x40201008040200ULL};

// Variables

static BOARD_T
  BRD_TMP = {{0},{0},{0},0,0}, *BRD = &BRD_TMP, *MGEN_MOVES = 0, *BRD_ORIGINAL = 0, BRD_EMPTY = {{0},{0},{0},0,0};

static int
  TOKENS_N = 0, TOKENS_I = 0, KING_W = 0, KING_B = 0, MGEN_MOVES_N = 0, ROOK_W[2] = {0}, ROOK_B[2] = {0},
  HASH_COUNT = 0, HASH_KEY = 0;

static char
  POSITION_FEN[90] = STARTPOS, FEN_STR[5][90] = {{0}}, TOKENS[MAX_TOKENS][90] = {{0}};

static uint64_t
  MGEN_BLACK = 0, MGEN_BOTH = 0, MGEN_EMPTY = 0, MGEN_GOOD = 0, MGEN_PAWN_SQ = 0, MGEN_WHITE = 0, PAWN_1_MOVES_W[64] = {0},
  PAWN_1_MOVES_B[64] = {0}, PAWN_2_MOVES_W[64] = {0}, PAWN_2_MOVES_B[64] = {0}, ZOBRIST_EP[64]= {0}, ZOBRIST_CASTLE[16] = {0},
  ZOBRIST_WTM[2] = {0}, ZOBRIST_BOARD[13][64] = {{0}}, CASTLE_W[2] = {0}, CASTLE_B[2] = {0}, CASTLE_EMPTY_W[2] = {0},
  CASTLE_EMPTY_B[2] = {0}, BISHOP_MOVES[64] = {0}, ROOK_MOVES[64] = {0}, QUEEN_MOVES[64] = {0}, KNIGHT_MOVES[64] = {0},
  KING_MOVES[64] = {0}, PAWN_CHECKS_W[64] = {0}, PAWN_CHECKS_B[64] = {0}, RANDOM_SEED = 131783,
  BISHOP_MAGIC_MOVES[64][512] = {{0}}, ROOK_MAGIC_MOVES[64][4096] = {{0}}, HASH_SIZE = 0;

static bool
  WTM = 0;

static HASH_T
  *HASH = 0;

// Prototypes

static uint64_t PerftB(const int);
static uint64_t RookMagicMoves(const int, const uint64_t);
static uint64_t BishopMagicMoves(const int, const uint64_t);

// Utils

static inline int Max(const int a, const int b) {
  return a > b ? a : b;
}

static uint64_t Nps(const uint64_t nodes, const uint64_t ms) {
  return (1000 * nodes) / (ms + 1);
}

static inline int Lsb(const uint64_t bb) {
  return __builtin_ctzll(bb);
}

static inline int PopCount(const uint64_t bb) {
  return __builtin_popcountll(bb);
}

static inline uint64_t ClearBit(const uint64_t bb) {
  return bb & (bb - 1);
}

static inline uint64_t Bit(const int nbits) {
  return 0x1ULL << nbits;
}

static void Print(const char *format, ...) {
  va_list va;
  va_start(va, format);
  vfprintf(stdout, format, va);
  va_end(va);
  fprintf(stdout, "\n");
  fflush(stdout);
}

static void Assert(const bool test, const char *msg) {
  if (test) return;
  Print(msg);
  exit(EXIT_FAILURE);
}

static uint64_t Now(void) {
  struct timeval tv;
  if (gettimeofday(&tv, NULL)) return 0;
  return (uint64_t) (1000 * tv.tv_sec + tv.tv_usec / 1000);
}

static uint64_t RandomBB(void) { // Deterministic
  static uint64_t a = 0X12311227ULL, b = 0X1931311ULL, c = 0X13138141ULL;
  a ^= b + c;
  b ^= b * c + 0x1717711ULL;
  c  = (3 * c) + 1;
#define MIXER(val) (((val) << 7) ^ ((val) >> 5))
  return MIXER(a) ^ MIXER(b) ^ MIXER(c);
}

static uint64_t Random8x64(void) {
  uint64_t val = 0;
  for (int i = 0; i < 8; i++) val ^= RandomBB() << (8 * i);
  return val;
}

static bool OnBoard(const int x, const int y) {
  return x >= 0 && x <= 7 && y >= 0 && y <= 7;
}

static inline uint64_t Hash(const int wtm) {
  uint64_t hash = ZOBRIST_EP[BRD->epsq + 1] ^ ZOBRIST_WTM[wtm] ^ ZOBRIST_CASTLE[BRD->castle], both = BOTH();
  for (int sq; both; both = ClearBit(both)) {sq = Lsb(both); hash ^= ZOBRIST_BOARD[BRD->board[sq] + 6][sq];}
  return hash;
}

// Token stuff

static void TokenAdd(const char *token) {if (TOKENS_N >= MAX_TOKENS) return; strcpy(TOKENS[TOKENS_N], token); TOKENS_N++;}
static bool TokenOk(void) {return TOKENS_I < TOKENS_N;}
static const char *TokenCurrent(void) {return TokenOk() ? TOKENS[TOKENS_I] : "";}
static void TokenPop(void) {TOKENS_I++;}
static bool TokenIs(const char *token) {return TokenOk() && !strcmp(token, TokenCurrent());}
static bool Token(const char *token) {if (!TokenIs(token)) return 0; TokenPop(); return 1;}
static int TokenInt(void) {int val = 0; if (TokenOk()) {val = atoi(TOKENS[TOKENS_I]); TokenPop();} return val;}

// Board stuff

static void BuildBitboards(void) {
  RESET(BRD->white);
  RESET(BRD->black);
  for (int i = 0; i < 64; i++)
    if (     BRD->board[i] > 0) BRD->white[ BRD->board[i] - 1] |= Bit(i);
    else if (BRD->board[i] < 0) BRD->black[-BRD->board[i] - 1] |= Bit(i);
}

static uint64_t Fill(int from, const int to) {
  uint64_t ret   = Bit(from);
  const int diff = from > to ? -1 : 1;
  if (from < 0 || to < 0 || from > 63 || to > 63) return 0;
  if (from == to) return ret;
  do {
    from += diff;
    ret  |= Bit(from);
  } while (from != to);
  return ret;
}

static void FindKings(void) {
  for (int i = 0; i < 64; i++)
    if (     BRD->board[i] ==  6) KING_W = i;
    else if (BRD->board[i] == -6) KING_B = i;
}

static void FindRank1Rank8Rooks(void) {
  int i;
  for (i = KING_W + 1; i <  8; i++) if (BRD->board[i] == 4) ROOK_W[0] = i;
  for (i = KING_W - 1; i > -1; i--) if (BRD->board[i] == 4) ROOK_W[1] = i;
  for (i = KING_B + 1; i < 64;         i++) if (BRD->board[i] == -4) ROOK_B[0] = i;
  for (i = KING_B - 1; i > 64 - 8 - 1; i--) if (BRD->board[i] == -4) ROOK_B[1] = i;
}

static void FindCastlingRooksAndKings(void) {
  KING_W = KING_B = 0;
  FindKings();
  RESET(ROOK_W);
  RESET(ROOK_B);
  FindRank1Rank8Rooks();
}

static void BuildCastlingBitboards(void) {
  CASTLE_W[0] = Fill(KING_W, 6);
  CASTLE_W[1] = Fill(KING_W, 2);
  CASTLE_B[0] = Fill(KING_B, 56 + 6);
  CASTLE_B[1] = Fill(KING_B, 56 + 2);
  CASTLE_EMPTY_W[0] = (CASTLE_W[0] | Fill(ROOK_W[0], 5     )) ^ (Bit(KING_W) | Bit(ROOK_W[0]));
  CASTLE_EMPTY_B[0] = (CASTLE_B[0] | Fill(ROOK_B[0], 56 + 5)) ^ (Bit(KING_B) | Bit(ROOK_B[0]));
  CASTLE_EMPTY_W[1] = (CASTLE_W[1] | Fill(ROOK_W[1], 3     )) ^ (Bit(KING_W) | Bit(ROOK_W[1]));
  CASTLE_EMPTY_B[1] = (CASTLE_B[1] | Fill(ROOK_B[1], 56 + 3)) ^ (Bit(KING_B) | Bit(ROOK_B[1]));
  for (int i = 0; i < 2; i++) {
    CASTLE_EMPTY_W[i] &= 0xFFULL;
    CASTLE_W[i]       &= 0xFFULL;
    CASTLE_EMPTY_B[i] &= 0xFF00000000000000ULL;
    CASTLE_B[i]       &= 0xFF00000000000000ULL;
  }
}

static int Piece(const char piece) {
  for (int i = 0; i < 6; i++)
    if (     piece == "pnbrqk"[i]) return -i - 1;
    else if (piece == "PNBRQK"[i]) return  i + 1;
  return 0;
}

static void FenBoard(const char *fen) {
  for (int sq = 56; *fen != '\0' && sq >= 0; fen++) if (*fen == '/') sq -= 16; else if (*fen >= '0' && *fen <= '9') sq += *fen - '0'; else BRD->board[sq++] = Piece(*fen);
}

static void FenKQkq(const char *fen) {
  for (; *fen != '\0'; fen++)
    if (*fen == 'K') {
      BRD->castle |= 1;
    } else if (*fen == 'Q') {
      BRD->castle |= 2;
    } else if (*fen == 'k') {
      BRD->castle |= 4;
    } else if (*fen == 'q') {
      BRD->castle |= 8;
    } else if (*fen >= 'A' && *fen <= 'H') {
      const int tmp = *fen - 'A';
      if (     tmp > KING_W) {ROOK_W[0] = tmp; BRD->castle |= 1;}
      else if (tmp < KING_W) {ROOK_W[1] = tmp; BRD->castle |= 2;}
    } else if (*fen >= 'a' && *fen <= 'h') {
      const int tmp = *fen - 'a';
      if (     tmp > X(KING_B)) {ROOK_B[0] = 56 + tmp; BRD->castle |= 4;}
      else if (tmp < X(KING_B)) {ROOK_B[1] = 56 + tmp; BRD->castle |= 8;}
    }
}

static void FenEp(const char *fen) {
  if (*fen == '-' || *fen == '\0' || *(fen + 1) == '\0') return;
  BRD->epsq = (*fen - 'a') + (8 * (*(fen + 1) - '1'));
}

static void FenSplit(const char *fen) {
  for (int i = 0; i < 5; i++) FEN_STR[i][0] = '\0';
  for (int split = 0; split < 5; split++) {
    while (*fen == ' ') fen++;
    if (*fen == '\0') return;
    for (int len = 0; *fen != ' ' && *fen != '\0'; len++, fen++) {
      FEN_STR[split][len]     = *fen;
      FEN_STR[split][len + 1] = '\0';
    }
  }
}

static void FenCreate(const char *fen) {
  FenSplit(fen);
  if (FEN_STR[0][0] == '\0') return;
  FenBoard(FEN_STR[0]);
  if (FEN_STR[1][0] == '\0') return;
  WTM = FEN_STR[1][0] == 'w';
  if (FEN_STR[2][0] == '\0') return;
  FindCastlingRooksAndKings();
  FenKQkq(FEN_STR[2]);
  BuildCastlingBitboards();
  if (FEN_STR[3][0] == '\0') return;
  FenEp(FEN_STR[3]);
}

static void FenReset(void) {
  BRD_TMP   = BRD_EMPTY;
  BRD       = &BRD_TMP;
  WTM       = 1;
  BRD->epsq = -1;
  KING_W = KING_B = 0;
  RESET(ROOK_W);
  RESET(ROOK_B);
}

static void Fen(const char *fen) {
  FenReset();
  FenCreate(fen);
  BuildBitboards();
  Assert(PopCount(BRD->white[5]) == 1 && PopCount(BRD->black[5]) == 1, "Error #2: Bad fen");
}

// Checks

static inline bool ChecksHereW(const int square) {
  const uint64_t both = BOTH();
  return ((PAWN_CHECKS_B[square]          &  BRD->white[0])                  |
          (KNIGHT_MOVES[square]           &  BRD->white[1])                  |
          (BishopMagicMoves(square, both) & (BRD->white[2] | BRD->white[4])) |
          (RookMagicMoves(square, both)   & (BRD->white[3] | BRD->white[4])) |
          (KING_MOVES[square]             &  BRD->white[5]));
}

static inline bool ChecksHereB(const int square) {
  const uint64_t both = BOTH();
  return ((PAWN_CHECKS_W[square]          &  BRD->black[0])                  |
          (KNIGHT_MOVES[square]           &  BRD->black[1])                  |
          (BishopMagicMoves(square, both) & (BRD->black[2] | BRD->black[4])) |
          (RookMagicMoves(square, both)   & (BRD->black[3] | BRD->black[4])) |
          (KING_MOVES[square]             &  BRD->black[5]));
}

static bool ChecksCastleW(uint64_t squares) {for (; squares; squares = ClearBit(squares)) if (ChecksHereW(Lsb(squares))) return 1; return 0;}
static bool ChecksCastleB(uint64_t squares) {for (; squares; squares = ClearBit(squares)) if (ChecksHereB(Lsb(squares))) return 1; return 0;}
static bool ChecksW(void) {return ChecksHereW(Lsb(BRD->black[5]));}
static bool ChecksB(void) {return ChecksHereB(Lsb(BRD->white[5]));}

// Move generator

#if defined PEXT
static inline uint32_t Pext(const uint64_t occupied, const uint64_t mask) {return _pext_u64(occupied, mask);}
static inline uint64_t BishopMagicMoves(const int square, const uint64_t occupied) {return BISHOP_MAGIC_MOVES[square][Pext(occupied, BISHOP_MASK[square])];}
static inline uint64_t RookMagicMoves(const int square, const uint64_t occupied)   {return ROOK_MAGIC_MOVES[  square][Pext(occupied, ROOK_MASK[square])];}
#else
static const uint64_t
  ROOK_MAGIC[64] = {
    0x548001400080106cULL,0x900184000110820ULL,0x428004200a81080ULL,0x140088082000c40ULL,0x1480020800011400ULL,0x100008804085201ULL,0x2a40220001048140ULL,0x50000810000482aULL,
    0x250020100020a004ULL,0x3101880100900a00ULL,0x200a040a00082002ULL,0x1004300044032084ULL,0x2100408001013ULL,0x21f00440122083ULL,0xa204280406023040ULL,0x2241801020800041ULL,
    0xe10100800208004ULL,0x2010401410080ULL,0x181482000208805ULL,0x4080101000021c00ULL,0xa250210012080022ULL,0x4210641044000827ULL,0x8081a02300d4010ULL,0x8008012000410001ULL,
    0x28c0822120108100ULL,0x500160020aa005ULL,0xc11050088c1000ULL,0x48c00101000a288ULL,0x494a184408028200ULL,0x20880100240006ULL,0x10b4010200081ULL,0x40a200260000490cULL,
    0x22384003800050ULL,0x7102001a008010ULL,0x80020c8010900c0ULL,0x100204082a001060ULL,0x8000118188800428ULL,0x58e0020009140244ULL,0x100145040040188dULL,0x44120220400980ULL,
    0x114001007a00800ULL,0x80a0100516304000ULL,0x7200301488001000ULL,0x1000151040808018ULL,0x3000a200010e0020ULL,0x1000849180802810ULL,0x829100210208080ULL,0x1004050021528004ULL,
    0x61482000c41820b0ULL,0x241001018a401a4ULL,0x45020c009cc04040ULL,0x308210c020081200ULL,0xa000215040040ULL,0x10a6024001928700ULL,0x42c204800c804408ULL,0x30441a28614200ULL,
    0x40100229080420aULL,0x9801084000201103ULL,0x8408622090484202ULL,0x4022001048a0e2ULL,0x280120020049902ULL,0x1200412602009402ULL,0x914900048020884ULL,0x104824281002402ULL},
  BISHOP_MAGIC[64] = {
    0x2890208600480830ULL,0x324148050f087ULL,0x1402488a86402004ULL,0xc2210a1100044bULL,0x88450040b021110cULL,0xc0407240011ULL,0xd0246940cc101681ULL,0x1022840c2e410060ULL,
    0x4a1804309028d00bULL,0x821880304a2c0ULL,0x134088090100280ULL,0x8102183814c0208ULL,0x518598604083202ULL,0x67104040408690ULL,0x1010040020d000ULL,0x600001028911902ULL,
    0x8810183800c504c4ULL,0x2628200121054640ULL,0x28003000102006ULL,0x4100c204842244ULL,0x1221c50102421430ULL,0x80109046e0844002ULL,0xc128600019010400ULL,0x812218030404c38ULL,
    0x1224152461091c00ULL,0x1c820008124000aULL,0xa004868015010400ULL,0x34c080004202040ULL,0x200100312100c001ULL,0x4030048118314100ULL,0x410000090018ULL,0x142c010480801ULL,
    0x8080841c1d004262ULL,0x81440f004060406ULL,0x400a090008202ULL,0x2204020084280080ULL,0xb820060400008028ULL,0x110041840112010ULL,0x8002080a1c84400ULL,0x212100111040204aULL,
    0x9412118200481012ULL,0x804105002001444cULL,0x103001280823000ULL,0x40088e028080300ULL,0x51020d8080246601ULL,0x4a0a100e0804502aULL,0x5042028328010ULL,0xe000808180020200ULL,
    0x1002020620608101ULL,0x1108300804090c00ULL,0x180404848840841ULL,0x100180040ac80040ULL,0x20840000c1424001ULL,0x82c00400108800ULL,0x28c0493811082aULL,0x214980910400080cULL,
    0x8d1a0210b0c000ULL,0x164c500ca0410cULL,0xc6040804283004ULL,0x14808001a040400ULL,0x180450800222a011ULL,0x600014600490202ULL,0x21040100d903ULL,0x10404821000420ULL};
static inline uint64_t BishopMagicIndex(const int square, const uint64_t mask) {return ((mask & BISHOP_MASK[square]) * BISHOP_MAGIC[square]) >> 55;}
static inline uint64_t RookMagicIndex(const int square, const uint64_t mask)   {return ((mask & ROOK_MASK[square]) * ROOK_MAGIC[square]) >> 52;}
static inline uint64_t BishopMagicMoves(const int square, const uint64_t mask) {return BISHOP_MAGIC_MOVES[square][BishopMagicIndex(square, mask)];}
static inline uint64_t RookMagicMoves(const int square, const uint64_t mask)   {return ROOK_MAGIC_MOVES[square][RookMagicIndex(square, mask)];}
#endif

static void HandleCastlingW() {
  MGEN_MOVES[MGEN_MOVES_N] = *BRD;
  BRD          = &MGEN_MOVES[MGEN_MOVES_N];
  BRD->epsq    = -1;
  BRD->castle &= 4 | 8;
}

static void AddCastleOOW(void) {
  if (ChecksCastleB(CASTLE_W[0])) return;
  HandleCastlingW();
  BRD->board[ROOK_W[0]] = 0;
  BRD->board[KING_W]    = 0;
  BRD->board[5]         = 4;
  BRD->board[6]         = 6;
  BRD->white[3]         = (BRD->white[3] ^ Bit(ROOK_W[0])) | Bit(5);
  BRD->white[5]         = (BRD->white[5] ^ Bit(KING_W))    | Bit(6);
  if (ChecksB()) return;
  MGEN_MOVES_N++;
}

static void AddCastleOOOW(void) {
  if (ChecksCastleB(CASTLE_W[1])) return;
  HandleCastlingW();
  BRD->board[ROOK_W[1]] = 0;
  BRD->board[KING_W]    = 0;
  BRD->board[3]         = 4;
  BRD->board[2]         = 6;
  BRD->white[3]         = (BRD->white[3] ^ Bit(ROOK_W[1])) | Bit(3);
  BRD->white[5]         = (BRD->white[5] ^ Bit(KING_W))    | Bit(2);
  if (ChecksB()) return;
  MGEN_MOVES_N++;
}

static void MgenCastlingMovesW(void) {
  if ((BRD->castle & 1) && !(CASTLE_EMPTY_W[0] & MGEN_BOTH)) {AddCastleOOW();  BRD = BRD_ORIGINAL;}
  if ((BRD->castle & 2) && !(CASTLE_EMPTY_W[1] & MGEN_BOTH)) {AddCastleOOOW(); BRD = BRD_ORIGINAL;}
}

static void HandleCastlingB() {
  MGEN_MOVES[MGEN_MOVES_N] = *BRD;
  BRD          = &MGEN_MOVES[MGEN_MOVES_N];
  BRD->epsq    = -1;
  BRD->castle &= 1 | 2;
}

static void AddCastleOOB(void) {
  if (ChecksCastleW(CASTLE_B[0])) return;
  HandleCastlingB();
  BRD->board[ROOK_B[0]] = 0;
  BRD->board[KING_B]    = 0;
  BRD->board[56 + 5]    = -4;
  BRD->board[56 + 6]    = -6;
  BRD->black[3]         = (BRD->black[3] ^ Bit(ROOK_B[0])) | Bit(56 + 5);
  BRD->black[5]         = (BRD->black[5] ^ Bit(KING_B))    | Bit(56 + 6);
  if (ChecksW()) return;
  MGEN_MOVES_N++;
}

static void AddCastleOOOB(void) {
  if (ChecksCastleW(CASTLE_B[1])) return;
  HandleCastlingB();
  BRD->board[ROOK_B[1]] = 0;
  BRD->board[KING_B]    = 0;
  BRD->board[56 + 3]    = -4;
  BRD->board[56 + 2]    = -6;
  BRD->black[3]         = (BRD->black[3] ^ Bit(ROOK_B[1])) | Bit(56 + 3);
  BRD->black[5]         = (BRD->black[5] ^ Bit(KING_B))    | Bit(56 + 2);
  if (ChecksW()) return;
  MGEN_MOVES_N++;
}

static void MgenCastlingMovesB(void) {
  if ((BRD->castle & 4) && !(CASTLE_EMPTY_B[0] & MGEN_BOTH)) {AddCastleOOB();  BRD = BRD_ORIGINAL;}
  if ((BRD->castle & 8) && !(CASTLE_EMPTY_B[1] & MGEN_BOTH)) {AddCastleOOOB(); BRD = BRD_ORIGINAL;}
}

static void CheckCastlingRightsW(void) {
  if (BRD->board[KING_W]    != 6) {BRD->castle &= 4 | 8; return;}
  if (BRD->board[ROOK_W[0]] != 4)  BRD->castle &= 2 | 4 | 8;
  if (BRD->board[ROOK_W[1]] != 4)  BRD->castle &= 1 | 4 | 8;
}

static void CheckCastlingRightsB(void) {
  if (BRD->board[KING_B]    != -6) {BRD->castle &= 1 | 2; return;}
  if (BRD->board[ROOK_B[0]] != -4)  BRD->castle &= 1 | 2 | 8;
  if (BRD->board[ROOK_B[1]] != -4)  BRD->castle &= 1 | 2 | 4;
}

static void HandleCastlingRights(void) {
  if (!BRD->castle) return;
  CheckCastlingRightsW();
  CheckCastlingRightsB();
}

static void ModifyPawnStuffW(const int from, const int to) {
  if (to == BRD_ORIGINAL->epsq) {
    BRD->board[to - 8] = 0;
    BRD->black[0]     ^= Bit(to - 8);
  } else if (Y(to) - Y(from) == 2) {
    BRD->epsq = to - 8;
  }
}

static void AddPromotionW(const int from, const int to, const int piece) {
  const int eat = BRD->board[to];
  MGEN_MOVES[MGEN_MOVES_N] = *BRD;
  BRD = &MGEN_MOVES[MGEN_MOVES_N];
  BRD->epsq        = -1;
  BRD->board[to]   = piece;
  BRD->board[from] = 0;
  BRD->white[0]   ^= Bit(from);
  BRD->white[piece - 1] |= Bit(to);
  if (eat <= -1) BRD->black[-eat - 1] ^= Bit(to);
  if (ChecksB()) return;
  HandleCastlingRights();
  MGEN_MOVES_N++;
}

static void AddPromotionStuffW(const int from, const int to) {
  BOARD_T *tmp = BRD;
  for (int piece = 2; piece <= 5; piece++) { // =qnrb
    AddPromotionW(from, to, piece);
    BRD = tmp;
  }
}

static void AddNormalStuffW(const int from, const int to) {
  const int me = BRD->board[from], eat = BRD->board[to];
  if (me <= 0) return;
  MGEN_MOVES[MGEN_MOVES_N] = *BRD;
  BRD = &MGEN_MOVES[MGEN_MOVES_N];
  BRD->epsq          = -1;
  BRD->board[from]   = 0;
  BRD->board[to]     = me;
  BRD->white[me - 1] = (BRD->white[me - 1] ^ Bit(from)) | Bit(to);
  if (eat <= -1) BRD->black[-eat - 1] ^= Bit(to);
  if (BRD->board[to] == 1) ModifyPawnStuffW(from, to);
  if (ChecksB()) return;
  HandleCastlingRights();
  MGEN_MOVES_N++;
}

static void AddW(const int from, const int to) {
  if (BRD->board[from] == 1 && Y(from) == 6)
    AddPromotionStuffW(from, to);
  else
    AddNormalStuffW(from, to);
}

static void ModifyPawnStuffB(const int from, const int to) {
  if (to == BRD_ORIGINAL->epsq) {
    BRD->board[to + 8] = 0;
    BRD->white[0]     ^= Bit(to + 8);
  } else if (Y(to) - Y(from) == -2) {
    BRD->epsq = to + 8;
  }
}

static void AddNormalStuffB(const int from, const int to) {
  const int me = BRD->board[from], eat = BRD->board[to];
  if (me >= 0) return;
  MGEN_MOVES[MGEN_MOVES_N] = *BRD;
  BRD                 = &MGEN_MOVES[MGEN_MOVES_N];
  BRD->epsq           = -1;
  BRD->board[to]      = me;
  BRD->board[from]    = 0;
  BRD->black[-me - 1] = (BRD->black[-me - 1] ^ Bit(from)) | Bit(to);
  if (eat >= 1) BRD->white[eat - 1] ^= Bit(to);
  if (BRD->board[to] == -1) ModifyPawnStuffB(from, to);
  if (ChecksW()) return;
  HandleCastlingRights();
  MGEN_MOVES_N++;
}

static void AddPromotionB(const int from, const int to, const int piece) {
  const int eat = BRD->board[to];
  MGEN_MOVES[MGEN_MOVES_N] = *BRD;
  BRD = &MGEN_MOVES[MGEN_MOVES_N];
  BRD->epsq        = -1;
  BRD->board[from] = 0;
  BRD->board[to]   = piece;
  BRD->black[0]   ^= Bit(from);
  BRD->black[-piece - 1] |= Bit(to);
  if (eat >= 1) BRD->white[eat - 1] ^= Bit(to);
  if (ChecksW()) return;
  HandleCastlingRights();
  MGEN_MOVES_N++;
}

static void AddPromotionStuffB(const int from, const int to) {
  BOARD_T *tmp = BRD;
  for (int piece = 2; piece <= 5; piece++) {
    AddPromotionB(from, to, -piece);
    BRD = tmp;
  }
}

static void AddB(const int from, const int to) {
  if (BRD->board[from] == -1 && Y(from) == 1)
    AddPromotionStuffB(from, to);
  else
    AddNormalStuffB(from, to);
}

static void AddMovesW(const int from, uint64_t moves) {
  for (; moves; moves = ClearBit(moves)) {
    AddW(from, Lsb(moves));
    BRD = BRD_ORIGINAL;
  }
}

static void AddMovesB(const int from, uint64_t moves) {
  for (; moves; moves = ClearBit(moves)) {
    AddB(from, Lsb(moves));
    BRD = BRD_ORIGINAL;
  }
}

static void MgenSetupW(void) {
  MGEN_WHITE   = WHITE();
  MGEN_BLACK   = BLACK();
  MGEN_BOTH    = MGEN_WHITE | MGEN_BLACK;
  MGEN_EMPTY   = ~MGEN_BOTH;
  MGEN_PAWN_SQ = BRD->epsq > 0 ? MGEN_BLACK | (Bit(BRD->epsq) & 0x0000FF0000000000ULL) : MGEN_BLACK;
}

static void MgenSetupB(void) {
  MGEN_WHITE   = WHITE();
  MGEN_BLACK   = BLACK();
  MGEN_BOTH    = MGEN_WHITE | MGEN_BLACK;
  MGEN_EMPTY   = ~MGEN_BOTH;
  MGEN_PAWN_SQ = BRD->epsq > 0 ? MGEN_WHITE | (Bit(BRD->epsq) & 0x0000000000FF0000ULL) : MGEN_WHITE;
}

static void MgenPawnsW(void) {
  for (uint64_t pieces = BRD->white[0]; pieces; pieces = ClearBit(pieces)) {
    const int sq = Lsb(pieces);
    AddMovesW(sq, PAWN_CHECKS_W[sq] & MGEN_PAWN_SQ);
    if (Y(sq) == 1) {
      if (PAWN_1_MOVES_W[sq] & MGEN_EMPTY) AddMovesW(sq, PAWN_2_MOVES_W[sq] & MGEN_EMPTY);
    } else {
      AddMovesW(sq, PAWN_1_MOVES_W[sq] & MGEN_EMPTY);
    }
  }
}

static void MgenPawnsB(void) {
  for (uint64_t pieces = BRD->black[0]; pieces; pieces = ClearBit(pieces)) {
    const int sq = Lsb(pieces);
    AddMovesB(sq, PAWN_CHECKS_B[sq] & MGEN_PAWN_SQ);
    if (Y(sq) == 6) {
      if (PAWN_1_MOVES_B[sq] & MGEN_EMPTY) AddMovesB(sq, PAWN_2_MOVES_B[sq] & MGEN_EMPTY);
    } else {
      AddMovesB(sq, PAWN_1_MOVES_B[sq] & MGEN_EMPTY);
    }
  }
}

static void MgenKnightsW(void) {
  for (uint64_t pieces = BRD->white[1]; pieces; pieces = ClearBit(pieces)) {
    const int sq = Lsb(pieces);
    AddMovesW(sq, KNIGHT_MOVES[sq] & MGEN_GOOD);
  }
}

static void MgenKnightsB(void) {
  for (uint64_t pieces = BRD->black[1]; pieces; pieces = ClearBit(pieces)) {
    const int sq = Lsb(pieces);
    AddMovesB(sq, KNIGHT_MOVES[sq] & MGEN_GOOD);
  }
}

static void MgenBishopsPlusQueensW(void) {
  for (uint64_t pieces = BRD->white[2] | BRD->white[4]; pieces; pieces = ClearBit(pieces)) {
    const int sq = Lsb(pieces);
    AddMovesW(sq, BishopMagicMoves(sq, MGEN_BOTH) & MGEN_GOOD);
  }
}

static void MgenBishopsPlusQueensB(void) {
  for (uint64_t pieces = BRD->black[2] | BRD->black[4]; pieces; pieces = ClearBit(pieces)) {
    const int sq = Lsb(pieces);
    AddMovesB(sq, BishopMagicMoves(sq, MGEN_BOTH) & MGEN_GOOD);
  }
}

static void MgenRooksPlusQueensW(void) {
  for (uint64_t pieces = BRD->white[3] | BRD->white[4]; pieces; pieces = ClearBit(pieces)) {
    const int sq = Lsb(pieces);
    AddMovesW(sq, RookMagicMoves(sq, MGEN_BOTH) & MGEN_GOOD);
  }
}

static void MgenRooksPlusQueensB(void) {
  for (uint64_t pieces = BRD->black[3] | BRD->black[4]; pieces; pieces = ClearBit(pieces)) {
    const int sq = Lsb(pieces);
    AddMovesB(sq, RookMagicMoves(sq, MGEN_BOTH) & MGEN_GOOD);
  }
}

static void MgenKingW(void) {
  const int sq = Lsb(BRD->white[5]);
  AddMovesW(sq, KING_MOVES[sq] & MGEN_GOOD);
}

static void MgenKingB(void) {
  const int sq = Lsb(BRD->black[5]);
  AddMovesB(sq, KING_MOVES[sq] & MGEN_GOOD);
}

static void MgenAllW(void) {
  MgenSetupW();
  MGEN_GOOD = ~MGEN_WHITE;
  MgenPawnsW();
  MgenKnightsW();
  MgenBishopsPlusQueensW();
  MgenRooksPlusQueensW();
  MgenKingW();
  MgenCastlingMovesW();
}

static void MgenAllB(void) {
  MgenSetupB();
  MGEN_GOOD = ~MGEN_BLACK;
  MgenPawnsB();
  MgenKnightsB();
  MgenBishopsPlusQueensB();
  MgenRooksPlusQueensB();
  MgenKingB();
  MgenCastlingMovesB();
}

static int MgenW(BOARD_T *moves) {
  MGEN_MOVES_N = 0;
  MGEN_MOVES   = moves;
  BRD_ORIGINAL = BRD;
  MgenAllW();
  return MGEN_MOVES_N;
}

static int MgenB(BOARD_T *moves) {
  MGEN_MOVES_N = 0;
  MGEN_MOVES   = moves;
  BRD_ORIGINAL = BRD;
  MgenAllB();
  return MGEN_MOVES_N;
}

// Hash

static void HashtableFreeMemory() {
  if (!HASH_SIZE) return;
  free(HASH);
  HASH      = 0;
  HASH_SIZE = 0;
}

static void HashtableSetSize(const int usize) {
  uint64_t size = (uint64_t) usize;
  HashtableFreeMemory();
  if (size < 1) size = 1;
  if (size > 1024 * 1024) size = 1024 * 1024;
  size = (1 << 20) * size;
  HASH_SIZE = 1;
  while (HASH_SIZE <= size) HASH_SIZE <<= 1;
  HASH_SIZE  >>= 1;
  HASH_COUNT   = (int) (HASH_SIZE / sizeof(HASH_T));
  HASH_KEY     = 1;
  while (HASH_KEY <= HASH_COUNT) HASH_KEY <<= 1;
  HASH_KEY >>= 1;
  HASH_KEY  -= 1; // 1000b = 8d / - 1d / 0111b = 7d
  HASH       = (HASH_T*) calloc(HASH_COUNT, sizeof(HASH_T));
  Assert(HASH != NULL, "Error # 7: Couldn't allocate space for the hashtable");
}

// Perft

static uint64_t GetPerft(const uint64_t hash, const int depth) {
  const HASH_T *entry = &HASH[(uint32_t)(hash & (uint64_t) HASH_KEY)];
  return entry->hash == hash && entry->depth == depth ? entry->nodes : 0;
}

static void AddPerft(const uint64_t hash, const uint64_t nodes, const int depth) {
  HASH_T *entry = &HASH[(uint32_t)(hash & (uint64_t) HASH_KEY)];
  if (!nodes || (entry->hash == hash && entry->nodes > nodes)) return;
  entry->depth = depth;
  entry->hash  = hash;
  entry->nodes = nodes;
}

// Bulk counting + hashing
static uint64_t PerftW(const int depth) {
  BOARD_T moves[MAX_MOVES];
  const uint64_t hash = Hash(1);
  uint64_t nodes      = GetPerft(hash, depth);
  if (nodes) return nodes;
  int len = MgenW(moves);
  if (depth <= 0) return (uint64_t) len;
  nodes = 0;
  for (int i = 0; i < len; i++) {
    BRD    = moves + i;
    nodes += PerftB(depth - 1);
  }
  AddPerft(hash, nodes, depth);
  return nodes;
}

static uint64_t PerftB(const int depth) {
  BOARD_T moves[MAX_MOVES];
  const uint64_t hash = Hash(0);
  uint64_t nodes      = GetPerft(hash, depth);
  if (nodes) return nodes;
  int len = MgenB(moves);
  if (depth <= 0) return (uint64_t) len;
  nodes = 0;
  for (int i = 0; i < len; i++) {
    BRD    = moves + i;
    nodes += PerftW(depth - 1);
  }
  AddPerft(hash, nodes, depth);
  return nodes;
}

static uint64_t Perft(const int depth) {
  Fen(POSITION_FEN);
  if (depth > 0) return WTM ? PerftW(depth - 1) : PerftB(depth - 1);
  return 1;
}

// 561735852 -> 561,735,852
static const char *BigNumber(uint64_t number) {
  char str[256] = "";
  static char ret[256];
  int counter = 0, three = 2, i, len;
  if (!number) {ret[0] = '0'; ret[1] = '\0'; return ret;}
  for (i = 0; i < 100; i++) {if (!number) break; str[counter] = '0' + (number % 10); counter++; number /= 10; if (!three && number) {str[counter] = ','; counter++; three = 3;} three--; str[counter + 1] = '\0';}
  for (counter = 0, len = (int) strlen(str), i = len - 1; i >= 0; i--) {ret[counter] = str[i]; counter++;}
  ret[counter] = '\0';
  return ret;
}

static void PerftPrint(const int depth, const uint64_t nodes, const uint64_t ms) {
  const char *big_num = BigNumber(nodes);
  if (depth < 0) printf("Total"); else printf("%5i", depth);
  printf("%20s %11.3f %11.3f\n", big_num, 0.000001 * (double) Nps(nodes, ms), 0.001 * (double) ms);
}

static void PerftRun(const int depth) {
  uint64_t nodes, start_time, diff_time, totaltime = 0, allnodes = 0;
  Print("[ %s ]", POSITION_FEN);
  Print("Depth               Nodes        Mnps        Time");
  for (int i = 0; i < depth + 1; i++) {
    start_time = Now();
    nodes      = Perft(i);
    diff_time  = Now() - start_time;
    totaltime  += diff_time;
    allnodes   += nodes;
    PerftPrint(i, nodes, diff_time);
  }
  Print("=================================================");
  Print("                    Nodes        Mnps        Time");
  PerftPrint(-1, allnodes, totaltime);
  exit(EXIT_SUCCESS);
}

static uint64_t SuiteRun(const int depth) {
  uint64_t start, nodes = 0, allnodes = 0;
  Print("Depth               Nodes        Mnps        Time");
  for (int i = 0; i <= depth; i++) {
    start = Now();
    nodes = Perft(i);
    allnodes += nodes;
    PerftPrint(i, nodes, Now() - start);
  }
  return allnodes;
}

static void Bench(const bool fullsuite) {
  uint64_t nodes = 0;
  const char fens[12][90] = {
    // Normal : https://www.chessprogramming.org/Perft_Results
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq",
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq",
    "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w -",
    "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq",
    "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ",
    "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w -",
    // Chess960 : https://www.chessprogramming.org/Chess960_Perft_Results
    "bqnb1rkr/pp3ppp/3ppn2/2p5/5P2/P2P4/NPP1P1PP/BQ1BNRKR w HFhf",
    "bnqbnr1r/p1p1ppkp/3p4/1p4p1/P7/3NP2P/1PPP1PP1/BNQB1RKR w HF",
    "nrbq2kr/ppppppb1/5n1p/5Pp1/8/P5P1/1PPPP2P/NRBQNBKR w HBhb",
    "1r1bkqbr/pppp1ppp/2nnp3/8/2P5/N4P2/PP1PP1PP/1RNBKQBR w Hh",
    "rkqnbbnr/ppppppp1/8/7p/3N4/6PP/PPPPPP2/RKQNBB1R w HAa",
    "rbqkr1bn/pp1ppp2/2p1n2p/6p1/8/4BPNP/PPPPP1P1/RBQKRN2 w EAea"
  };
  uint64_t start = Now();
  for (int i = 0; i < 12; i++) {
    strcpy(POSITION_FEN, fens[i]);
    Print("%s[ #%i: %s ]", i ? "\n" : "", i + 1, fens[i]);
    nodes += SuiteRun(fullsuite ? 6 : 5);
  }
  Print("\n=================================================\n");
  Print("                    Nodes        Mnps        Time");
  PerftPrint(-1, nodes, Now() - start);
  Assert((nodes == (fullsuite ? 21799671196 : 561735852)), "Error #7: Broken move generator");
}

// CLI

static void PrintHelp() {
  Print("# %s\nGNU General Public License version 3; for details see 'LICENSE'\n", NAME);
  Print("> lastemperor -hash 512 -perft 6 # Set 512 MB hash and run perft\n");
  Print("## Commands");
  Print("--help        This help");
  Print("--version     Show Version");
  Print("-hash [N]     Set hash in N MB");
  Print("-fen [FEN]    Set fen");
  Print("-perft [1..]  Run perft position");
  Print("-bench [01]   Benchmark (0 = normal, 1 = full)\n");
  Print("Full source code here: <https://github.com/SamuraiDangyo/LastEmperor/>");
}

static void Commands(void) {
  while (TokenOk()) {
    if (     Token("--help"))    {PrintHelp(); break;}
    else if (Token("--version")) {Print(NAME); break;}
    else if (Token("-hash"))     {HashtableSetSize(TokenInt());}
    else if (Token("-fen"))      {strcpy(POSITION_FEN, TokenCurrent()); Fen(POSITION_FEN); TokenPop();}
    else if (Token("-perft"))    {PerftRun(Max(1, TokenInt())); break;}
    else if (Token("-bench"))    {Bench(TokenInt()); break;}
    else                         {Print("Unknown Command: '%s'", TokenCurrent()); break;}
  }
}

// Init

static uint64_t PermutateBb(const uint64_t moves, const int index) {
  int i, total = 0, good[64] = {0};
  uint64_t permutations = 0;
  for (i = 0; i < 64; i++)
    if (moves & Bit(i)) {
      good[total] = i;
      total++;
    }
  const int popn = PopCount(moves);
  for (i = 0; i < popn; i++)
    if ((1 << i) & index)
      permutations |= Bit(good[i]);
  return permutations & moves;
}

static uint64_t MakeSliderMagicMoves(const int *slider_vectors, const int square, const uint64_t moves) {
  uint64_t tmp, possible_moves = 0;
  const int x_square = X(square), y_square = Y(square);
  for (int i = 0; i < 4; i++)
    for (int j = 1; j < 8; j++) {
      const int x = x_square + j * slider_vectors[2 * i], y = y_square + j * slider_vectors[2 * i + 1];
      if (!OnBoard(x, y)) break;
      tmp             = Bit(8 * y + x);
      possible_moves |= tmp;
      if (tmp & moves) break;
    }
  return possible_moves & (~Bit(square));
}

static void InitBishopMagics(void) {
  for (int i = 0; i < 64; i++) {
    const uint64_t magics = BISHOP_MOVE_MAGICS[i] & (~Bit(i));
    for (int j = 0; j < 512; j++) {
      const uint64_t allmoves = PermutateBb(magics, j);
#if defined PEXT
      BISHOP_MAGIC_MOVES[i][Pext(allmoves, BISHOP_MASK[i])] = MakeSliderMagicMoves(BISHOP_VECTORS, i, allmoves);
#else
      BISHOP_MAGIC_MOVES[i][BishopMagicIndex(i, allmoves)] = MakeSliderMagicMoves(BISHOP_VECTORS, i, allmoves);
#endif
    }
  }
}

static void InitRookMagics(void) {
  for (int i = 0; i < 64; i++) {
    const uint64_t magics = ROOK_MOVE_MAGICS[i] & (~Bit(i));
    for (int j = 0; j < 4096; j++) {
      const uint64_t allmoves = PermutateBb(magics, j);
#if defined PEXT
      ROOK_MAGIC_MOVES[i][Pext(allmoves, ROOK_MASK[i])] = MakeSliderMagicMoves(ROOK_VECTORS, i, allmoves);
#else
      ROOK_MAGIC_MOVES[i][RookMagicIndex(i, allmoves)] = MakeSliderMagicMoves(ROOK_VECTORS, i, allmoves);
#endif
    }
  }
}

static uint64_t MakeSliderMoves(const int square, const int *slider_vectors) {
  uint64_t moves = 0;
  const int x_square = X(square), y_square = Y(square);
  for (int i = 0; i < 4; i++) {
    const int dx = slider_vectors[2 * i], dy = slider_vectors[2 * i + 1];
    uint64_t tmp = 0;
    for (int j = 1; j < 8; j++) {
      const int x = x_square + j * dx, y = y_square + j * dy;
      if (!OnBoard(x, y)) break;
      tmp |= Bit(8 * y + x);
    }
    moves |= tmp;
  }
  return moves;
}

static void InitSliderMoves(void) {
  for (int i = 0; i < 64; i++) {
    ROOK_MOVES[i]   = MakeSliderMoves(i, ROOK_VECTORS);
    BISHOP_MOVES[i] = MakeSliderMoves(i, BISHOP_VECTORS);
    QUEEN_MOVES[i]  = ROOK_MOVES[i] | BISHOP_MOVES[i];
  }
}

static uint64_t MakeJumpMoves(const int square, const int len, const int dy, const int *jump_vectors) {
  uint64_t moves = 0;
  const int x_square = X(square), y_square = Y(square);
  for (int i = 0; i < len; i++) {
    const int x = x_square + jump_vectors[2 * i], y = y_square + dy * jump_vectors[2 * i + 1];
    if (OnBoard(x, y)) moves |= Bit(8 * y + x);
  }
  return moves;
}

static void InitJumpMoves(void) {
  const int pawn_check_vectors[2 * 2] = {-1,1,1,1}, pawn_1_vectors[1 * 2] = {0,1};
  for (int i = 0; i < 64; i++) {
    KING_MOVES[i]     = MakeJumpMoves(i, 8,  1, KING_VECTORS);
    KNIGHT_MOVES[i]   = MakeJumpMoves(i, 8,  1, KNIGHT_VECTORS);
    PAWN_CHECKS_W[i]  = MakeJumpMoves(i, 2,  1, pawn_check_vectors);
    PAWN_CHECKS_B[i]  = MakeJumpMoves(i, 2, -1, pawn_check_vectors);
    PAWN_1_MOVES_W[i] = MakeJumpMoves(i, 1,  1, pawn_1_vectors);
    PAWN_1_MOVES_B[i] = MakeJumpMoves(i, 1, -1, pawn_1_vectors);
  }
  for (int i = 0; i < 8; i++) {
    PAWN_2_MOVES_W[ 8 + i] = MakeJumpMoves( 8 + i, 1,  1, pawn_1_vectors) | MakeJumpMoves( 8 + i, 1,  2, pawn_1_vectors);
    PAWN_2_MOVES_B[48 + i] = MakeJumpMoves(48 + i, 1, -1, pawn_1_vectors) | MakeJumpMoves(48 + i, 1, -2, pawn_1_vectors);
  }
}

static void InitZobrist(void) {
  int i, j;
  for (i = 0; i < 13; i++) for (j = 0; j < 64; j++) ZOBRIST_BOARD[i][j] = Random8x64();
  for (i = 0; i < 64; i++) ZOBRIST_EP[i]     = Random8x64();
  for (i = 0; i < 16; i++) ZOBRIST_CASTLE[i] = Random8x64();
  for (i = 0; i <  2; i++) ZOBRIST_WTM[i]    = Random8x64();
}

static void Init(void) {
  RANDOM_SEED += (uint64_t) time(NULL);
  InitZobrist();
  InitBishopMagics();
  InitRookMagics();
  InitSliderMoves();
  InitJumpMoves();
  HashtableSetSize(256);
  Fen(STARTPOS);
}

// "Wisdom begins in wonder." -- Socrates
int main(int argc, char **argv) {
  for (int i = 1; i < argc; i++) TokenAdd(argv[i]);
  Init();
  atexit(HashtableFreeMemory);
  Commands();
  return EXIT_SUCCESS;
}
