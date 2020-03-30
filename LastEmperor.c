/*
LastEmperor, a Chess960 move generator (Derived from Sapeli 1.67)
Copyright (C) 2019 Toni Helminen

LastEmperor is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

LastEmperor is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

//
// Headers
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/time.h>
#include <limits.h>
#include <stdbool.h>

//
// Constants
//

#define NAME            "LastEmperor"
#define VERSION         "1.05"
#define AUTHOR          "Toni Helminen"

#define STARTPOS        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define MAX_MOVES       218
#define MAX_TOKENS      32
#define BITBOARD        unsigned long long int
#define MEGABYTE        (1 << 20)
#define WHITE()         (BRD->white[0] | BRD->white[1] | BRD->white[2] | BRD->white[3] | BRD->white[4] | BRD->white[5])
#define BLACK()         (BRD->black[0] | BRD->black[1] | BRD->black[2] | BRD->black[3] | BRD->black[4] | BRD->black[5])
#define BOTH()          (WHITE() | BLACK())

//
// Macros
//

#define X(a)            ((a) & 7)
#define Y(a)            ((a) >> 3)
#define INT(a)          ((int) (a))
#define DOUBLE(f)       ((double) (f))
#define MAX(a, b)       (((a) > (b)) ? (a) : (b))
#define MIN(a, b)       (((a) < (b)) ? (a) : (b))
#define ULL(a)          ((BITBOARD) (a))
#define RESET(a)        memset((a), 0, sizeof((a)))
#define BIT(a)          (0x1ULL << (a))
#define LSB(b)          (__builtin_ctzll(b))
#define POPCOUNT(b)     (__builtin_popcountll(b))
#define MYASSERT(test)  Assert((test), (__LINE__))

//
// Structs
//

typedef struct {
  BITBOARD white[6];    // White bitboards
  BITBOARD black[6];    // Black bitboards
  char board[64];       // Pieces
  BITBOARD hash;        // Hash of the position
  char epsq;            // En passant square
  unsigned char castle; // Castling rights
} BOARD_T;

typedef struct {
  BITBOARD hash;
  BITBOARD nodes;
  int depth;
} HASH_ENTRY_T;

typedef struct {
  HASH_ENTRY_T *array;
  BITBOARD size; // Size in bytes
  int count;     // Number of entries
  int key;       // Hash key
} HASH_T;

typedef struct {
  const char fen[100];
  const BITBOARD nodes[7];
} PERFT_T;

//
// LastEmperor headers
//

#include "fprotos.h"
#include "consts.h"
#include "all960pos.h"

//
// Static ( global ) variables
//

// Board

static const BOARD_T BRD_EMPTY        = {0};
static BOARD_T BRD_2                  = {0};
static BOARD_T *BRD                   = &BRD_2;
static BOARD_T *BRD_ORIGINAL          = 0;
static bool WTM                       = 0;

// Move generator

static HASH_T MYHASH                  = {0};
static BOARD_T *MOVES                 = 0;
static int MOVES_N                    = 0;
static int KING_W                     = 0;
static int KING_B                     = 0;
static int ROOK_W[2]                  = {0};
static int ROOK_B[2]                  = {0};
static BITBOARD CASTLE_W[2]           = {0};
static BITBOARD CASTLE_B[2]           = {0};
static BITBOARD CASTLE_EMPTY_W[2]     = {0};
static BITBOARD CASTLE_EMPTY_B[2]     = {0};
static BITBOARD BISHOP_MAGIC_MOVES[64][512]  = {{0}};
static BITBOARD ROOK_MAGIC_MOVES[  64][4096] = {{0}};

// Zobrist

static BITBOARD ZOBRIST_BOARD[13][64] = {{0}};
static BITBOARD ZOBRIST_EP[64]        = {0};
static BITBOARD ZOBRIST_CASTLE[16]    = {0};
static BITBOARD ZOBRIST_WTM[2]        = {0};

// Misc

static BITBOARD RANDOM_SEED            = 131783;
static BITBOARD PERFT_SUITE_TOTAL_TIME = 0;
static int TOKENS_N                    = 0;
static int TOKENS_I                    = 0;
static char POSITION_FEN[128]          = {0};
static char TOKENS[MAX_TOKENS][128]    = {{0}};
static char FEN_STR[4][128]            = {{0}};

//
// Utils
//

static inline /* <- make me faster! */ int Max(const int a, const int b)
{
  return a > b ? a : b;
}

static inline int Min(const int a, const int b)
{
  return a < b ? a : b;
}

static bool Equal_strings(const char *str1, const char *str2)
{
  return strcmp(str1, str2) ? 0 : 1;
}

static BITBOARD Nps(const BITBOARD nodes, const BITBOARD ms)
{
  return ms ? (1000 * nodes) / ms : 0;
}

static void Print(const char *format, ...)
{
  va_list va;
  va_start(va, format);
  vfprintf(stdout, format, va);
  va_end(va);
  fprintf(stdout, "\n");
  fflush(stdout);
}

static const char *Int_to_string(const int x)
{
  static char str[32];
  sprintf(str, "%d", x);
  return str;
}

static BITBOARD Now(void)
{
  struct timeval tv;
  MYASSERT(gettimeofday(&tv, NULL) == 0);
  return 1000 * tv.tv_sec + tv.tv_usec / 1000;
}

static void String_join(char *s1, const char *s2)
{
  strcpy(s1 + strlen(s1), s2);
}

static bool Is_number(const char ch)
{
  return ch >= '0' && ch <= '9';
}

// Deterministic to get same zobrist numbers every time
static BITBOARD Random_bb(void)
{
  static BITBOARD a = 0x12311227ULL;
  static BITBOARD b = 0x1931311ULL;
  static BITBOARD c = 0x13138141ULL;

  a ^= b + c;
  b ^= b * c + 0x1717711ULL;
  c *= 3;
  c += 1;

#define MIXER(val) (((val) << 7) ^ ((val) >> 5))
  return MIXER(a) ^ MIXER(b) ^ MIXER(c);
}

static BITBOARD Random_u64(void)
{
  int i;
  BITBOARD ret = 0;
  for (i = 0; i < 8; i++) ret ^= Random_bb() << (8 * i);
  return ret;
}

static bool On_board(const int x, const int y)
{
  return x >= 0 && y >= 0 && x <= 7 && y <= 7;
}

static const char *Get_time_string(void)
{
  static char str[64] = {0};
  time_t tmr;
  struct tm *tmem;
  time(&tmr);
  tmem = localtime(&tmr);
  strftime(str, 64, "%d.%m.%Y %X", tmem);
  return str;
}

//
// Debug
//

static void Debug_tokens(void)
{
  int i;
  Print("TOKENS ( %i ) :", TOKENS_N);
  for (i = 0; i < TOKENS_N; i++)
    Print("%i. %s", i, TOKENS[i]);
}

static void Debug_log(const char *str)
{
  FILE *file = fopen("LastEmperor-log.txt", "a+");
  fprintf(file, "%s\n%s\n:::\n", Get_time_string(), str);
  fclose(file);
}

static void Assert(const bool test, const int line_number)
{
  if ( ! test) {
    Print("LastEmperor error: Line: %i", line_number);
    exit(EXIT_FAILURE);
  }
}

//
// Token stuff
//

static void Token_add(const char *token)
{
  MYASSERT(TOKENS_N + 2 < MAX_TOKENS);
  strcpy(TOKENS[TOKENS_N], token);
  TOKENS_N++;
}

static void Token_reset(void)
{
  TOKENS_I = 0;
  TOKENS_N = 0;
}

static const char *Token_current(void)
{
  return Token_ok() ? TOKENS[TOKENS_I] : "\0";
}

static void Token_pop(void)
{
  TOKENS_I++;
}

static bool Token_ok(void)
{
  return TOKENS_I < TOKENS_N;
}

static bool Token_is(const char *token)
{
  return Token_ok() && Equal_strings(token, Token_current());
}

static void Token_expect(const char *token)
{
  if ( ! Token_is(token)) {
    Print("LastEmperor error ( Unexpected token ) : '%s'", Token_current());
    exit(EXIT_FAILURE);
  }
  Token_pop();
}

static bool Token_next(const char *token)
{
  if ( ! Token_is(token)) return 0;
  Token_pop();
  return 1;
}

static int Token_next_int(void)
{
  int ret = 0;

  if (Token_ok() && ! Token_is(";")) { // Assume number
    ret = atoi(TOKENS[TOKENS_I]);
    Token_pop();
  }

  return ret;
}

//
// Board stuff
//

static void Build_bitboards(void)
{
  int i;

  RESET(BRD->white);
  RESET(BRD->black);
  for (i = 0; i < 64; i++)
    if (BRD->board[i] > 0)      BRD->white[ BRD->board[i] - 1] |= BIT(i);
    else if (BRD->board[i] < 0) BRD->black[-BRD->board[i] - 1] |= BIT(i);
}

static BITBOARD Fill(int from, const int to)
{
  BITBOARD ret   = BIT(from);
  const int diff = from > to ? -1 : 1;

  if (from < 0 || to < 0 || from > 63 || to > 63) return 0;
  if (from == to) return ret;

  while (from != to) {
    from += diff;
    ret  |= BIT(from);
  }

  return ret;
}

static void Find_castling_rooks_and_kings(void)
{
  int i;

  RESET(ROOK_W);
  RESET(ROOK_B);
  KING_W = 0;
  KING_B = 0;

  for (i = 0; i < 64; i++)                  if (BRD->board[i] ==  6) KING_W    = i;
  for (i = KING_W + 1; i < 8; i++)          if (BRD->board[i] ==  4) ROOK_W[0] = i;
  for (i = KING_W - 1; i > -1; i--)         if (BRD->board[i] ==  4) ROOK_W[1] = i;
  for (i = 0; i < 64; i++)                  if (BRD->board[i] == -6) KING_B    = i;
  for (i = KING_B + 1; i < 64; i++)         if (BRD->board[i] == -4) ROOK_B[0] = i;
  for (i = KING_B - 1; i > 64 - 8 - 1; i--) if (BRD->board[i] == -4) ROOK_B[1] = i;
}

static int Piece(const char piece)
{
  int i;
  for (i = 0; i < 6; i++)
    if (     piece == "pnbrqk"[i]) return -i - 1;
    else if (piece == "PNBRQK"[i]) return i + 1;
  return 0;
}

static void Fen_board(const char *fen)
{
  int pos = 56;

  while (*fen != '\0' && pos >= 0) {
    if (*fen == '/') {
      pos -= 16;
    } else if (Is_number(*fen)) {
      pos += *fen - '0';
    } else {
      BRD->board[pos] = Piece(*fen);
      pos++;
    }
    fen++;
  }
}

static void Fen_KQkq(const char *fen)
{
  int tmp;

  while (*fen != '\0') {
    if (*fen == 'K') {
      BRD->castle |= 1;
    } else if (*fen == 'Q') {
      BRD->castle |= 2;
    } else if (*fen == 'k') {
      BRD->castle |= 4;
    } else if (*fen == 'q') {
      BRD->castle |= 8;
    } else if (*fen >= 'A' && *fen <= 'H') {
      tmp = *fen - 'A';
      if (tmp > KING_W) {
        ROOK_W[0]    = tmp;
        BRD->castle |= 1;
      } else if (tmp < KING_W) {
        ROOK_W[1]    = tmp;
        BRD->castle |= 2;
      }
    } else if (*fen >= 'a' && *fen <= 'h') {
      tmp = *fen - 'a';
      if (tmp > X(KING_B)) {
        ROOK_B[0]    = 56 + tmp;
        BRD->castle |= 4;
      } else if (tmp < X(KING_B)) {
        ROOK_B[1]    = 56 + tmp;
        BRD->castle |= 8;
      }
    }
    fen++;
  }
}

static void Fen_ep(const char *fen)
{
  if (*fen == '-' || *fen == '\0' || *(fen + 1) == '\0') return;
  BRD->epsq = *fen - 'a';
  fen++;
  BRD->epsq += 8 * (*fen - '1');
}

static void Fen_split(const char *fen)
{
  int len, i;
  int piece = 0;

  for (i = 0; i < 4; i++) FEN_STR[i][0] = '\0';

  while (piece < 4) {
    while (*fen == ' ') fen++;
    if (*fen == '\0') return;

    len = 0;
    while (*fen != ' ' && *fen != '\0') {
      MYASSERT((len < 128));
      FEN_STR[piece][len] = *fen;
      len++;
      FEN_STR[piece][len] = '\0';
      fen++;
    }

    piece++;
  }
}

static void Build_castling_bitboards(void)
{
  int i;
  const BITBOARD rank1 = 0xFFULL;
  const BITBOARD rank8 = 0xFF00000000000000ULL;

  CASTLE_W[0] = Fill(KING_W, 6);
  CASTLE_W[1] = Fill(KING_W, 2);
  CASTLE_B[0] = Fill(KING_B, 56 + 6);
  CASTLE_B[1] = Fill(KING_B, 56 + 2);

  CASTLE_EMPTY_W[0] = (CASTLE_W[0] | Fill(ROOK_W[0], 5     )) ^ (BIT(KING_W) | BIT(ROOK_W[0]));
  CASTLE_EMPTY_B[0] = (CASTLE_B[0] | Fill(ROOK_B[0], 56 + 5)) ^ (BIT(KING_B) | BIT(ROOK_B[0]));
  CASTLE_EMPTY_W[1] = (CASTLE_W[1] | Fill(ROOK_W[1], 3     )) ^ (BIT(KING_W) | BIT(ROOK_W[1]));
  CASTLE_EMPTY_B[1] = (CASTLE_B[1] | Fill(ROOK_B[1], 56 + 3)) ^ (BIT(KING_B) | BIT(ROOK_B[1]));

  for (i = 0; i < 2; i++) {
    CASTLE_EMPTY_W[i] &= rank1;
    CASTLE_W[i]       &= rank1;
    CASTLE_EMPTY_B[i] &= rank8;
    CASTLE_B[i]       &= rank8;
  }
}

// https://en.wikipedia.org/wiki/Forsyth-Edwards_Notation
static void Fen_create(const char *fen)
{
  Fen_split(fen);

  if (FEN_STR[0][0] == '\0') return;
  Fen_board(FEN_STR[0]);

  if (FEN_STR[1][0] == '\0') return;
  WTM = FEN_STR[1][0] == 'w';

  if (FEN_STR[2][0] == '\0') return;
  Find_castling_rooks_and_kings();
  Fen_KQkq(FEN_STR[2]);
  Build_castling_bitboards();

  if (FEN_STR[3][0] == '\0') return;
  Fen_ep(FEN_STR[3]);
}

static void Fen(const char *fen)
{
  BRD_2     = BRD_EMPTY;
  BRD       = &BRD_2;
  WTM       = 1;
  BRD->epsq = -1;
  KING_W    = 0;
  KING_B    = 0;

  RESET(ROOK_W);
  RESET(ROOK_B);
  Fen_create(fen);
  Build_bitboards();
  BRD->hash = Hash(WTM);
  Assume_legal_position();
}

static void Assume_legal_position(void)
{
  int i;

  // Only 1 king per side
  MYASSERT((POPCOUNT(BRD->white[5]) == 1 && POPCOUNT(BRD->black[5]) == 1));

  // King can't be captured
  if (WTM) MYASSERT(( ! Checks_w())); else MYASSERT(( ! Checks_b()));

  // Kings on correct places
  MYASSERT((BRD->board[LSB(BRD->white[5])] == 6 && BRD->board[LSB(BRD->black[5])] == -6));

  // Pieces where they should be
  for (i = 0; i < 64; i++) {
    if (BRD->board[i] > 0)
      MYASSERT(((BRD->board[i] <=  6) && (BIT(i) & BRD->white[ BRD->board[i] - 1])));
    else if (BRD->board[i] < 0)
      MYASSERT(((BRD->board[i] >= -6) && (BIT(i) & BRD->black[-BRD->board[i] - 1])));
    else
      MYASSERT(( ! (BIT(i) & BOTH())));
  }

  // Check en passant validity
  MYASSERT((BRD->epsq >= -1 && BRD->epsq <= 63));
  if (BRD->epsq != -1) {
    if (WTM) MYASSERT((Y(BRD->epsq) == 5)); else MYASSERT((Y(BRD->epsq) == 2));
  }

  // Make sure castling rooks are in correct places
  if (BRD->castle & 1) MYASSERT((X(ROOK_W[0]) >= X(KING_W) && BRD->board[ROOK_W[0]] ==  4));
  if (BRD->castle & 2) MYASSERT((X(ROOK_W[1]) <= X(KING_W) && BRD->board[ROOK_W[1]] ==  4));
  if (BRD->castle & 4) MYASSERT((X(ROOK_B[0]) >= X(KING_B) && BRD->board[ROOK_B[0]] == -4));
  if (BRD->castle & 8) MYASSERT((X(ROOK_B[1]) <= X(KING_B) && BRD->board[ROOK_B[1]] == -4));
}

//
// Checks
//

static bool Checks_castle_w(BITBOARD squares)
{
  int pos;
  const BITBOARD both = BOTH();
  const BITBOARD bishop_plus_queen = BRD->white[2] | BRD->white[4];
  const BITBOARD rook_plus_queen = BRD->white[3] | BRD->white[4];

  while (squares) {
    pos      = LSB(squares);
    squares &= squares - 1;
    if ((PAWN_CHECKS_B[pos] & BRD->white[0])                |
        (KNIGHT_MOVES[pos] & BRD->white[1])                 |
        (Bishop_magic_moves(pos, both) & bishop_plus_queen) |
        (Rook_magic_moves(pos, both) & rook_plus_queen)     |
        (KING_MOVES[pos] & BRD->white[5]))
      return 1;
  }
  return 0;
}

static bool Checks_castle_b(BITBOARD squares)
{
  int pos;
  const BITBOARD both              = BOTH();
  const BITBOARD bishop_plus_queen = BRD->black[2] | BRD->black[4];
  const BITBOARD rook_plus_queen   = BRD->black[3] | BRD->black[4];

  while (squares) {
    pos      = LSB(squares);
    squares &= squares - 1;
    if ((PAWN_CHECKS_W[pos] & BRD->black[0])                |
        (KNIGHT_MOVES[pos] & BRD->black[1])                 |
        (Bishop_magic_moves(pos, both) & bishop_plus_queen) |
        (Rook_magic_moves(pos, both) & rook_plus_queen)     |
        (KING_MOVES[pos] & BRD->black[5]))
      return 1;
  }
  return 0;
}

static bool Checks_w(void)
{
  const BITBOARD both = BOTH();
  const int king_i    = LSB(BRD->black[5]);
  return ((PAWN_CHECKS_B[king_i] & BRD->white[0])                              |
          (KNIGHT_MOVES[king_i] & BRD->white[1])                               |
          (Bishop_magic_moves(king_i, both) & (BRD->white[2] | BRD->white[4])) |
          (Rook_magic_moves(king_i, both) & (BRD->white[3] | BRD->white[4]))   |
          (KING_MOVES[king_i] & BRD->white[5]));
}

static bool Checks_b(void)
{
  const BITBOARD both = BOTH();
  const int king_i    = LSB(BRD->white[5]);
  return ((PAWN_CHECKS_W[king_i] & BRD->black[0])                              |
          (KNIGHT_MOVES[king_i] & BRD->black[1])                               |
          (Bishop_magic_moves(king_i, both) & (BRD->black[2] | BRD->black[4])) |
          (Rook_magic_moves(king_i, both) & (BRD->black[3] | BRD->black[4]))   |
          (KING_MOVES[king_i] & BRD->black[5]));
}

//
// Mgen
//

static BITBOARD Mgen_slider_real(const int *slider_moves, const int pos, const BITBOARD moves)
{
  BITBOARD tmp;
  int j, k, x2, y2;
  const int x             = X(pos);
  const int y             = Y(pos);
  BITBOARD possible_moves = 0;

  for (j = 0; j < 4; j++)
    for (k = 1; k < 8; k++) {
      x2 = x + k * slider_moves[2 * j];
      y2 = y + k * slider_moves[2 * j + 1];
      if ( ! On_board(x2, y2)) break;
      tmp             = BIT(x2 + 8 * y2);
      possible_moves |= tmp;
      if (tmp & moves) break;
    }

  return possible_moves & (~BIT(pos));
}

static BITBOARD Mgen_bishop_real(const int pos, const BITBOARD moves)
{
  const int bishop_moves[8] = {1,1, -1,1, 1,-1, -1,-1};
  return Mgen_slider_real(bishop_moves, pos, moves);
}

static BITBOARD Mgen_rook_real(const int pos, const BITBOARD moves)
{
  const int rook_moves[8] = {1,0, -1,0, 0,1, 0,-1};
  return Mgen_slider_real(rook_moves, pos, moves);
}

static void Handle_castling_w(void)
{
  MOVES[MOVES_N]  = *BRD;
  BRD             = &MOVES[MOVES_N];
  BRD->epsq       = -1;
  BRD->castle    &= 4 | 8;
}

static void Add_castle_O_O_w(void)
{
  if (Checks_castle_b(CASTLE_W[0])) return;

  Handle_castling_w();

  BRD->board[ROOK_W[0]] = 0;
  BRD->board[KING_W]    = 0;
  BRD->board[5]         = 4;
  BRD->board[6]         = 6;
  BRD->white[3]        ^= BIT(ROOK_W[0]);
  BRD->white[5]        ^= BIT(KING_W);
  BRD->white[3]        |= BIT(5);
  BRD->white[5]        |= BIT(6);

  if (Checks_b()) return;

  BRD->hash ^= ZOBRIST_WTM[0]
               ^ ZOBRIST_WTM[1]
               ^ ZOBRIST_EP[BRD_ORIGINAL->epsq + 1]
               ^ ZOBRIST_EP[BRD->epsq + 1]
               ^ ZOBRIST_CASTLE[BRD_ORIGINAL->castle]
               ^ ZOBRIST_CASTLE[BRD->castle]
               ^ ZOBRIST_BOARD[6 + 6][KING_W]
               ^ ZOBRIST_BOARD[4 + 6][ROOK_W[0]]
               ^ ZOBRIST_BOARD[6 + 6][6]
               ^ ZOBRIST_BOARD[4 + 6][5];
  MOVES_N++;
}

static void Add_castle_O_O_O_w(void)
{
  if (Checks_castle_b(CASTLE_W[1])) return;

  Handle_castling_w();

  BRD->board[ROOK_W[1]] = 0;
  BRD->board[KING_W]    = 0;
  BRD->board[3]         = 4;
  BRD->board[2]         = 6;
  BRD->white[3]        ^= BIT(ROOK_W[1]);
  BRD->white[5]        ^= BIT(KING_W);
  BRD->white[3]        |= BIT(3);
  BRD->white[5]        |= BIT(2);

  if (Checks_b()) return;

  BRD->hash ^= ZOBRIST_WTM[0]
               ^ ZOBRIST_WTM[1]
               ^ ZOBRIST_EP[BRD_ORIGINAL->epsq + 1]
               ^ ZOBRIST_EP[BRD->epsq + 1]
               ^ ZOBRIST_CASTLE[BRD_ORIGINAL->castle]
               ^ ZOBRIST_CASTLE[BRD->castle]
               ^ ZOBRIST_BOARD[6 + 6][KING_W]
               ^ ZOBRIST_BOARD[4 + 6][ROOK_W[1]]
               ^ ZOBRIST_BOARD[6 + 6][2]
               ^ ZOBRIST_BOARD[4 + 6][3];
  MOVES_N++;
}

static void Castling_moves_w(const BITBOARD both)
{
  if ((BRD->castle & 1) && ! (CASTLE_EMPTY_W[0] & both)) {Add_castle_O_O_w();   BRD = BRD_ORIGINAL;} // O-O
  if ((BRD->castle & 2) && ! (CASTLE_EMPTY_W[1] & both)) {Add_castle_O_O_O_w(); BRD = BRD_ORIGINAL;} // O-O-O
}

static void Handle_castling_b(void)
{
  MOVES[MOVES_N] = *BRD;
  BRD            = &MOVES[MOVES_N];
  BRD->epsq      = -1;
  BRD->castle   &= 1 | 2;
}

static void Add_castle_O_O_b(void)
{
  if (Checks_castle_w(CASTLE_B[0])) return;

  Handle_castling_b();

  BRD->board[ROOK_B[0]] = 0;
  BRD->board[KING_B]    = 0;
  BRD->board[56 + 5]    = -4;
  BRD->board[56 + 6]    = -6;
  BRD->black[3]        ^= BIT(ROOK_B[0]);
  BRD->black[5]        ^= BIT(KING_B);
  BRD->black[3]        |= BIT(56 + 5);
  BRD->black[5]        |= BIT(56 + 6);

  if (Checks_w()) return;

  BRD->hash ^= ZOBRIST_WTM[0]
               ^ ZOBRIST_WTM[1]
               ^ ZOBRIST_EP[BRD_ORIGINAL->epsq + 1]
               ^ ZOBRIST_EP[BRD->epsq + 1]
               ^ ZOBRIST_CASTLE[BRD_ORIGINAL->castle]
               ^ ZOBRIST_CASTLE[BRD->castle]
               ^ ZOBRIST_BOARD[-6 + 6][KING_B]
               ^ ZOBRIST_BOARD[-4 + 6][ROOK_B[0]]
               ^ ZOBRIST_BOARD[-6 + 6][56 + 6]
               ^ ZOBRIST_BOARD[-4 + 6][56 + 5];
  MOVES_N++;
}

static void Add_castle_O_O_O_b(void)
{
  if (Checks_castle_w(CASTLE_B[1])) return;

  Handle_castling_b();

  BRD->board[ROOK_B[1]] = 0;
  BRD->board[KING_B]    = 0;
  BRD->board[56 + 3]    = -4;
  BRD->board[56 + 2]    = -6;
  BRD->black[3]        ^= BIT(ROOK_B[1]);
  BRD->black[5]        ^= BIT(KING_B);
  BRD->black[3]        |= BIT(56 + 3);
  BRD->black[5]        |= BIT(56 + 2);

  if (Checks_w()) return;

  BRD->hash ^= ZOBRIST_WTM[0]
               ^ ZOBRIST_WTM[1]
               ^ ZOBRIST_EP[BRD_ORIGINAL->epsq + 1]
               ^ ZOBRIST_EP[BRD->epsq + 1]
               ^ ZOBRIST_CASTLE[BRD_ORIGINAL->castle]
               ^ ZOBRIST_CASTLE[BRD->castle]
               ^ ZOBRIST_BOARD[-6 + 6][KING_B]
               ^ ZOBRIST_BOARD[-4 + 6][ROOK_B[1]]
               ^ ZOBRIST_BOARD[-6 + 6][56 + 2]
               ^ ZOBRIST_BOARD[-4 + 6][56 + 3];
  MOVES_N++;
}

static void Castling_moves_b(const BITBOARD both)
{
  if ((BRD->castle & 4) && ! (CASTLE_EMPTY_B[0] & both)) {Add_castle_O_O_b();   BRD = BRD_ORIGINAL;} // O-O
  if ((BRD->castle & 8) && ! (CASTLE_EMPTY_B[1] & both)) {Add_castle_O_O_O_b(); BRD = BRD_ORIGINAL;} // O-O-O
}

static void Check_castle_rights_w(void)
{
  if (BRD->board[KING_W]    != 6) {BRD->castle &= 4 | 8;     return;}
  if (BRD->board[ROOK_W[0]] != 4)  BRD->castle &= 2 | 4 | 8;
  if (BRD->board[ROOK_W[1]] != 4)  BRD->castle &= 1 | 4 | 8;
}

static void Check_castle_rights_b(void)
{
  if (BRD->board[KING_B]    != -6) {BRD->castle  &= 1 | 2;    return;}
  if (BRD->board[ROOK_B[0]] != -4)  BRD->castle  &= 1 | 2 | 8;
  if (BRD->board[ROOK_B[1]] != -4)  BRD->castle  &= 1 | 2 | 4;
}

static void Handle_castle_rights(void)
{
  if ( ! BRD->castle) return;
  Check_castle_rights_w();
  Check_castle_rights_b();
}

static void Add_underpromotion_w(const BOARD_T *board, const int piece, const int to)
{
  MOVES[MOVES_N]     = *board;
  BRD                = &MOVES[MOVES_N];
  BRD->board[to]     = piece + 1;
  BRD->hash         ^= ZOBRIST_BOARD[5 + 6][to];
  BRD->hash         ^= ZOBRIST_BOARD[piece + 1 + 6][to];
  BRD->white[4]     ^= BIT(to);
  BRD->white[piece] |= BIT(to);
  MOVES_N++;
}

static bool Add_pawn_w(const int from, const int to)
{
  BOARD_T *board;

  if (to > 55) {
    BRD->white[0]  ^= BIT(to);
    BRD->board[to]  = 5;
    BRD->white[4]  |= BIT(to);
    BRD->hash      ^= ZOBRIST_EP[BRD->epsq + 1] ^ ZOBRIST_BOARD[5 + 6][to];

    if (Checks_b()) return 1;

    Handle_castle_rights();
    BRD->hash ^= ZOBRIST_CASTLE[BRD->castle];
    MOVES_N++;

    board = BRD;
    Add_underpromotion_w(board, 1, to);
    Add_underpromotion_w(board, 2, to);
    Add_underpromotion_w(board, 3, to);

    return 1;
  } else if (to == BRD_ORIGINAL->epsq) {
    BRD->board[to - 8]  = 0;
    BRD->hash          ^= ZOBRIST_BOARD[-1 + 6][to - 8];
    BRD->black[0]      ^= BIT(to - 8);
  } else if (Y(from) == 1 && Y(to) == 3) {
    BRD->epsq = to - 8;
  }
  return 0;
}

static void Add_w(const int from, const int to)
{
  const int me  = BRD->board[from];
  const int eat = BRD->board[to];

  MOVES[MOVES_N]     = *BRD;
  BRD                = &MOVES[MOVES_N];
  BRD->epsq          = -1;
  BRD->board[to]     = me;
  BRD->board[from]   = 0;
  BRD->white[me - 1] = (BRD->white[me - 1] ^ BIT(from)) | BIT(to);

  BRD->hash ^= ZOBRIST_WTM[0]
               ^ ZOBRIST_WTM[1]
               ^ ZOBRIST_EP[BRD_ORIGINAL->epsq + 1]
               ^ ZOBRIST_CASTLE[BRD_ORIGINAL->castle]
               ^ ZOBRIST_BOARD[me + 6][from];

  if (eat) {
    BRD->hash            ^= ZOBRIST_BOARD[eat + 6][to];
    BRD->black[-eat - 1] ^= BIT(to);
  }

  if ((BRD->board[to] == 1 && Add_pawn_w(from, to)) || Checks_b()) return;

  Handle_castle_rights();
  BRD->hash ^= ZOBRIST_BOARD[me + 6][to]
               ^ ZOBRIST_CASTLE[BRD->castle]
               ^ ZOBRIST_EP[BRD->epsq + 1];
  MOVES_N++;
}

static void Add_underpromotion_b(const BOARD_T *board, const int piece, const int to)
{
  MOVES[MOVES_N]       = *board;
  BRD                  = &MOVES[MOVES_N];
  BRD->board[to]       = piece - 1;
  BRD->hash            ^= ZOBRIST_BOARD[-5 + 6][to];
  BRD->hash            ^= ZOBRIST_BOARD[piece - 1 + 6][to];
  BRD->black[4]        ^= BIT(to);
  BRD->black[-(piece)] |= BIT(to);
  MOVES_N++;
}

static bool Add_pawn_b(const int from, const int to)
{
  BOARD_T *board;

  if (to < 8) {
    BRD->black[0]  ^= BIT(to);
    BRD->board[to]  = -5;
    BRD->black[4]  |= BIT(to);
    BRD->hash      ^= ZOBRIST_EP[BRD->epsq + 1] ^ ZOBRIST_BOARD[-5 + 6][to];

    if (Checks_w()) return 1;

    Handle_castle_rights();
    BRD->hash ^= ZOBRIST_CASTLE[BRD->castle];
    MOVES_N++;
    board = BRD;

    Add_underpromotion_b(board, -1, to);
    Add_underpromotion_b(board, -2, to);
    Add_underpromotion_b(board, -3, to);
    return 1;
  } else if (to == BRD_ORIGINAL->epsq) {
    BRD->board[to + 8]  = 0;
    BRD->hash          ^= ZOBRIST_BOARD[1 + 6][to + 8];
    BRD->white[0]      ^= BIT(to + 8);
  } else if (Y(from) == 6 && Y(to) == 4) {
    BRD->epsq = to + 8;
  }
  return 0;
}

static void Add_b(const int from, const int to)
{
  const int me  = BRD->board[from];
  const int eat = BRD->board[to];

  MOVES[MOVES_N]       = *BRD;
  BRD                  = &MOVES[MOVES_N];
  BRD->epsq            = -1;
  BRD->board[to]       = me;
  BRD->board[from]     = 0;
  BRD->black[-me - 1]  = (BRD->black[-me - 1] ^ BIT(from)) | BIT(to);
  BRD->hash           ^= ZOBRIST_WTM[0]
                         ^ ZOBRIST_WTM[1]
                         ^ ZOBRIST_EP[BRD_ORIGINAL->epsq + 1]
                         ^ ZOBRIST_CASTLE[BRD_ORIGINAL->castle]
                         ^ ZOBRIST_BOARD[me + 6][from];

  if (eat) {
    BRD->hash           ^= ZOBRIST_BOARD[eat + 6][to];
    BRD->white[eat - 1] ^= BIT(to);
  }

  if ((BRD->board[to] == -1 && Add_pawn_b(from, to)) || Checks_w()) return;

  Handle_castle_rights();
  BRD->hash ^= ZOBRIST_BOARD[me + 6][to] ^ ZOBRIST_CASTLE[BRD->castle] ^ ZOBRIST_EP[BRD->epsq + 1];
  MOVES_N++;
}

static void Add_moves_w(const int from, BITBOARD moves)
{
  while (moves) {
    Add_w(from, LSB(moves));
    moves &= moves - 1;
    BRD    = BRD_ORIGINAL;
  }
}

static void Add_moves_b(const int from, BITBOARD moves)
{
  while (moves) {
    Add_b(from, LSB(moves));
    moves &= moves - 1;
    BRD    = BRD_ORIGINAL;
  }
}

static inline BITBOARD Bishop_magic_index(const int position, const BITBOARD mask)
{
  return ((mask & BISHOP_MASK[position]) * BISHOP_MAGIC[position]) >> 55;
}

static inline BITBOARD Rook_magic_index(const int position, const BITBOARD mask)
{
  return ((mask & ROOK_MASK[position]) * ROOK_MAGIC[position]) >> 52;
}

static inline BITBOARD Bishop_magic_moves(const int position, const BITBOARD mask)
{
  return BISHOP_MAGIC_MOVES[position][Bishop_magic_index(position, mask)];
}

static inline BITBOARD Rook_magic_moves(const int position, const BITBOARD mask)
{
  return ROOK_MAGIC_MOVES[position][Rook_magic_index(position, mask)];
}

#define POP() pos = LSB(pieces); pieces &= pieces - 1

static void Mgen_all_w(void)
{
  int pos;
  BITBOARD pieces;
  const BITBOARD white   = WHITE();
  const BITBOARD black   = BLACK();
  const BITBOARD both    = white | black;
  const BITBOARD empty   = ~both;
  const BITBOARD good    = ~white;
  const BITBOARD pawn_sq = BRD->epsq > 0 ? (black | (BIT(BRD->epsq) & 0x0000FF0000000000ULL)) : black;

  // Pawns
  pieces = BRD->white[0];
  while (pieces) {
    POP();
    Add_moves_w(pos, PAWN_CHECKS_W[pos] & pawn_sq);
    if (Y(pos) == 1) {if (PAWN_MOVES_1_W[pos] & empty) Add_moves_w(pos, PAWN_MOVES_2_W[pos] & empty);}
    else {                                             Add_moves_w(pos, PAWN_MOVES_1_W[pos] & empty);}
  }

  // Knights
  pieces = BRD->white[1];
  while (pieces) {POP(); Add_moves_w(pos, KNIGHT_MOVES[pos] & good);}

  // Bishops + Queens
  pieces = BRD->white[2] | BRD->white[4];
  while (pieces) {POP(); Add_moves_w(pos, Bishop_magic_moves(pos, both) & good);}

  // Rooks + Queens
  pieces = BRD->white[3] | BRD->white[4];
  while (pieces) {POP(); Add_moves_w(pos, Rook_magic_moves(pos, both) & good);}

  // King
  pos = LSB(BRD->white[5]);
  Add_moves_w(pos, KING_MOVES[pos] & good);
  Castling_moves_w(both);
}

static void Mgen_all_b(void)
{
  int pos;
  BITBOARD pieces;
  const BITBOARD white   = WHITE();
  const BITBOARD black   = BLACK();
  const BITBOARD both    = white | black;
  const BITBOARD empty   = ~both;
  const BITBOARD good    = ~black;
  const BITBOARD pawn_sq = BRD->epsq > 0 ? white | (BIT(BRD->epsq) & 0x0000000000FF0000ULL) : white;

  // Pawns
  pieces = BRD->black[0];
  while (pieces) {
    POP();
    Add_moves_b(pos, PAWN_CHECKS_B[pos] & pawn_sq);
    if (Y(pos) == 6) {if (PAWN_MOVES_1_B[pos] & empty) Add_moves_b(pos, PAWN_MOVES_2_B[pos] & empty);}
    else {                                             Add_moves_b(pos, PAWN_MOVES_1_B[pos] & empty);}
  }

  // Knights
  pieces = BRD->black[1];
  while (pieces) {POP(); Add_moves_b(pos, KNIGHT_MOVES[pos] & good);}

  // Bishops + Queens
  pieces = BRD->black[2] | BRD->black[4];
  while (pieces) {POP(); Add_moves_b(pos, Bishop_magic_moves(pos, both) & good);}

  // Rooks + Queens
  pieces = BRD->black[3] | BRD->black[4];
  while (pieces) {POP(); Add_moves_b(pos, Rook_magic_moves(pos, both) & good);}

  // King
  pos = LSB(BRD->black[5]);
  Add_moves_b(pos, KING_MOVES[pos] & good);
  Castling_moves_b(both);
}

static inline void Swap(BOARD_T *board_a, BOARD_T *board_b)
{
  BOARD_T tmp = *board_a;
  *board_a    = *board_b;
  *board_b    = tmp;
}

static int Mgen_w(BOARD_T *moves)
{
  MOVES_N      = 0;
  MOVES        = moves;
  BRD_ORIGINAL = BRD;
  Mgen_all_w();
  return MOVES_N;
}

static int Mgen_b(BOARD_T *moves)
{
  MOVES_N      = 0;
  MOVES        = moves;
  BRD_ORIGINAL = BRD;
  Mgen_all_b();
  return MOVES_N;
}

//
// Hash
//

static BITBOARD Hash(const int wtm)
{
  int pos;
  BITBOARD hash = ZOBRIST_EP[BRD->epsq + 1] ^ ZOBRIST_WTM[wtm] ^ ZOBRIST_CASTLE[BRD->castle];
  BITBOARD both = BOTH();

  while (both) {
    pos  = LSB(both);
    both &= both - 1;
    hash ^= ZOBRIST_BOARD[BRD->board[pos] + 6][pos];
  }

  return hash;
}

static void Hashtable_free_memory(void)
{
  if ( ! MYHASH.size) return;
  free(MYHASH.array);
  MYHASH.array = 0;
  MYHASH.size = 0;
}

// Allocate memory for the Hashtable
// -- The More The Better --
// [1 MB, 1 PB]
static void Hashtable_set_size(const int usize /* MB */)
{
  BITBOARD size = ULL(usize);

  Hashtable_free_memory();
  size = MAX(size, 1);
  size = MIN(size, 1024 * 1024); // 1 PB
  size = MEGABYTE * size; // To MB

  MYHASH.size = 1;
  while (MYHASH.size <= size) MYHASH.size <<= 1; // Calculate needed memory in bytes

  MYHASH.size  >>= 1;
  MYHASH.count   = INT(MYHASH.size / sizeof(HASH_ENTRY_T));
  MYHASH.key     = 1;
  while (MYHASH.key <= MYHASH.count) MYHASH.key <<= 1; // Create key according to count

  MYHASH.key   >>= 1;
  MYHASH.key    -= 1; // 1000b = 8d / - 1d / 0111b = 7d
  MYHASH.array   = (HASH_ENTRY_T*) calloc(MYHASH.count, sizeof(HASH_ENTRY_T)); // <- Cast for g++

  MYASSERT(MYHASH.array != NULL); // Make sure there is enough space
}

//
// Perft
//

static BITBOARD Get_perft(const BITBOARD hash, const int depth)
{
  const HASH_ENTRY_T *entry = &MYHASH.array[INT(hash & MYHASH.key)];
  return entry->hash == hash && entry->depth == depth ? entry->nodes : 0;
}

static void Add_perft(const BITBOARD hash, const BITBOARD nodes, const int depth)
{
  HASH_ENTRY_T *entry = &MYHASH.array[INT(hash & MYHASH.key)];
  if ( ! nodes || (entry->hash == hash && entry->nodes > nodes)) return;
  entry->depth = depth;
  entry->hash  = hash;
  entry->nodes = nodes;
}

// Bulk counting + hashing
static BITBOARD Perft_w(const int depth)
{
  int i, len;
  BOARD_T moves[MAX_MOVES];
  const BITBOARD hash = BRD->hash;
  BITBOARD nodes      = Get_perft(hash, depth);

  if (nodes) return nodes;

  len = Mgen_w(moves);
  if (depth <= 0) return len;

  nodes = 0;
  for (i = 0; i < len; i++) {
    BRD    = moves + i;
    nodes += Perft_b(depth - 1);
  }

  Add_perft(hash, nodes, depth);
  return nodes;
}

static BITBOARD Perft_b(const int depth)
{
  int i, len;
  BOARD_T moves[MAX_MOVES];
  const BITBOARD hash = BRD->hash;
  BITBOARD nodes      = Get_perft(hash, depth);

  if (nodes) return nodes;

  len = Mgen_b(moves);
  if (depth <= 0) return len;

  nodes = 0;
  for (i = 0; i < len; i++) {
    BRD    = moves + i;
    nodes += Perft_w(depth - 1);
  }

  Add_perft(hash, nodes, depth);
  return nodes;
}

static BITBOARD Perft(const int depth)
{
  BOARD_T *board = BRD;
  BITBOARD nodes = 1;

  if (depth > 0) nodes = WTM ? Perft_w(depth - 1) : Perft_b(depth - 1);
  BRD = board;

  return nodes;
}

static void Padding(const char *str, const int space)
{
  int i;
  const int len = space - strlen(str);
  for (i = 0; i < len; i++) printf(" ");
  printf("%s", str);
}

#define PRINTHEADER() Print("depth           nodes            mnps            time")

static void Perft_final_print(const BITBOARD nodes, const BITBOARD ms)
{
  Print("\n===");
  Perft_print(-1, nodes, ms);
}

static void Perft_print(const int depth, const BITBOARD nodes, const BITBOARD ms)
{
  static char str[32];
  if (depth < 0) {printf("total");} else {sprintf(str, "%i", depth); Padding(str, 5);}
  sprintf(str, "%llu", nodes); Padding(str, 16);
  sprintf(str, "%.3f", 0.000001f * DOUBLE(Nps(nodes, ms))); Padding(str, 16);
  sprintf(str, "%.3f", 0.001f * DOUBLE(ms)); Padding(str, 16);
  printf("\n");
}

static void Perft_run(const int depth)
{
  int i;
  BITBOARD nodes, start_time, diff_time;
  BITBOARD totaltime = 0;
  BITBOARD allnodes = 0;

  Print("### Perft ( %i MB ) ###", MYHASH.size / MEGABYTE);
  if (POSITION_FEN[0] == '\0') Print("\n[ %s ]", STARTPOS); else Print("\n[ %s ]", POSITION_FEN);
  PRINTHEADER();

  for (i = 0; i < depth + 1; i++) {
    start_time = Now();
    nodes      = Perft(i);
    diff_time  = Now() - start_time;
    totaltime  += diff_time;
    allnodes   += nodes;
    Perft_print(i, nodes, diff_time);
  }

  Perft_final_print(allnodes, totaltime);
}

static BITBOARD Suite_run(const int suite_i, const int depth)
{
  int i;
  BITBOARD start, diff, nodes;
  BITBOARD all_nodes = 0;
  const BITBOARD *counts = PERFT_SUITE[suite_i].nodes;

  PRINTHEADER();
  for (i = 0; i < depth; i++) {
    start = Now();
    nodes = Perft(i);
    diff  = Now() - start;

    PERFT_SUITE_TOTAL_TIME += diff;
    all_nodes              += nodes;

    Perft_print(i, nodes, diff);

    // Depths 7+ here
    if (i <= 6) MYASSERT(nodes == counts[i]);
  }

  return all_nodes;
}

static void Suite(const int depth)
{
  int i;
  BITBOARD nodes    = 0;

  Print("### Chess960 suite ( %i MB ) ###", MYHASH.size / MEGABYTE);
  PERFT_SUITE_TOTAL_TIME = 0;

  for (i = 0; i < 960; i++) {
    //printf("{\"%s\", {", PERFT_SUITE[i].fen);
    Fen(PERFT_SUITE[i].fen);
    Print("\n[ %i: %s ]", i + 1, PERFT_SUITE[i].fen);
    nodes += Suite_run(i, depth + 1);
  }

  Perft_final_print(nodes, PERFT_SUITE_TOTAL_TIME);
}

//
// LastEmperor-cli
//

static void Print_help(void)
{
  Print("%s %s by %s", NAME, VERSION, AUTHOR);
  Print("%s\n", SHORT_LICENSE);

  Print("Usage: lastemperor [COMMAND] [OPTION]? ...");
  Print("> lastemperor -hash 512 -perft 6 # Set 512 MB hash and run perft\n");

  Print("## LastEmperor Commands ##");
  Print("-help         This help");
  Print("-version      Show Version");
  Print("-fen [FEN]    Set fen");
  Print("-bench        Run LastEmperor benchmarks");
  Print("-id           Run Chess960 id tests");
  Print("-perft [1..]  Run perft position");
  Print("-suite [1..]  Run Chess960 test suite");
  Print("-hash N       Set hash in N MB\n");

  Print("Full source code, please see:\n  <https://github.com/SamuraiDangyo/LastEmperor/>");

  exit(EXIT_SUCCESS);
}

static void Commands(void)
{
  if (TOKENS_N < 2) {
    Print_help();
    exit(EXIT_SUCCESS);
  }

  Token_expect(";");
  while (Token_ok()) {
    if (     Token_next("help"))    Print_help();
    else if (Token_next("version")) Print("%s %s by %s", NAME, VERSION, AUTHOR);
    else if (Token_next("fen"))     Command_setfen();
    else if (Token_next("suite"))   Suite(Max(1, Token_next_int()));
    else if (Token_next("id"))      Suite(6);
    else if (Token_next("perft"))   Perft_run(Max(1, Token_next_int()));
    else if (Token_next("bench"))   Command_bench();
    else if (Token_next("hash"))    Hashtable_set_size(Token_next_int());
    Token_expect(";");
  }
}

static void Position_fen(void)
{
  POSITION_FEN[0] = '\0';
  while (Token_ok() && ! Token_is(";")) {
    MYASSERT(strlen(POSITION_FEN) + strlen(Token_current()) < 127);
    String_join(POSITION_FEN, Token_current());
    String_join(POSITION_FEN, " ");
    Token_pop();
  }

  POSITION_FEN[Max(0, strlen(POSITION_FEN) - 1)] = '\0';
  Fen(POSITION_FEN);
}

static void Command_setfen(void)
{
  Fen(STARTPOS);
  Position_fen();
}

static void Command_bench(void)
{
  Fen(STARTPOS);
  Perft_run(6);
}

//
// Init
//

static BITBOARD Permutate_bb(const BITBOARD moves, const int index)
{
  int i;
  int total             = 0;
  int good[64]          = {0};
  BITBOARD permutations = 0;
  const int popn        = POPCOUNT(moves);

  for (i = 0; i < 64; i++)
    if (moves & BIT(i)) {
      good[total] = i;
      total++;
    }

  for (i = 0; i < popn; i++)
    if ((1 << i) & index)
      permutations |= BIT(good[i]);

  return permutations & moves;
}

static void Init_bishop_magics(void)
{
  int i, j;
  BITBOARD magics, allmoves;

  for (i = 0; i < 64; i++) {
    magics = BISHOP_MOVE_MAGICS[i] & (~BIT(i));
    for (j = 0; j < 512; j++) {
      allmoves = Permutate_bb(magics, j);
      BISHOP_MAGIC_MOVES[i][Bishop_magic_index(i, allmoves)] = Mgen_bishop_real(i, allmoves);
    }
  }
}

static void Init_rook_magics(void)
{
  int i, j;
  BITBOARD magics, allmoves;

  for (i = 0; i < 64; i++) {
    magics = ROOK_MOVE_MAGICS[i] & (~BIT(i));
    for (j = 0; j < 4096; j++) {
      allmoves = Permutate_bb(magics, j);
      ROOK_MAGIC_MOVES[i][Rook_magic_index(i, allmoves)] = Mgen_rook_real(i, allmoves);
    }
  }
}

static void Init_tokens(int argc, char **argv)
{
  int i;

  Token_reset();
  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '-' && strlen(argv[i]) > 1) {
      Token_add(";");
      Token_add(argv[i] + 1);
    } else {
      Token_add(argv[i]);
    }
  }
  Token_add(";");
  //  Debug_tokens();
}

static void Init_board(void)
{
  Fen(STARTPOS);
}

static void Init_zobrist(void)
{
  int i, j;

  for (i = 0; i < 13; i++) for (j = 0; j < 64; j++) ZOBRIST_BOARD[i][j] = Random_u64();
  for (i = 0; i < 64; i++) ZOBRIST_EP[i]     = Random_u64();
  for (i = 0; i < 16; i++) ZOBRIST_CASTLE[i] = Random_u64();
  for (i = 0; i <  2; i++) ZOBRIST_WTM[i]    = Random_u64();
}

static void Init(void)
{
  RANDOM_SEED += ULL(time(NULL));
  Init_bishop_magics();
  Init_rook_magics();
  Init_zobrist();
  Hashtable_set_size(256 /* MB */ ); // Default size
  Init_board();
}

//
// Go
//

static void Ok(void)
{
  MYASSERT((sizeof(int) >= 4 && sizeof(double) >= 8 && sizeof(BITBOARD) >= 8 /* bytes */));
  MYASSERT(((0x1122334455667788ULL >> 32)                   == 0x11223344ULL));
  MYASSERT(((0x1122334455667788ULL << 32)                   == 0x5566778800000000ULL));
  MYASSERT(((0xFFFFFFFFFFFFFFFFULL & 0x8142241818244281ULL) == 0x8142241818244281ULL));
}


static void Happy(void)
{
  if (0) { // Makes gcc happy
    Debug_log(Int_to_string(0));
    Debug_tokens();
  }
}

static void Go(void)
{
  Ok();
  Happy();
  Init();
  Commands();
}

int main(int argc, char **argv)
{
  atexit(Hashtable_free_memory); // No memory leaks
  Init_tokens(argc, argv);
  Go();
  return EXIT_SUCCESS;
}
