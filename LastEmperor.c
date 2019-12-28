/**
* LastEmperor, a Chess960 move generator (Derived from Sapeli 1.67)
* Copyright (C) 2019 Toni Helminen
*
* LastEmperor is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* LastEmperor is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
**/

///
/// Headers
///

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/time.h>
#include <limits.h>
#include <assert.h>
#include <stdbool.h>

///
/// Constants
///

#define NAME                     "LastEmperor"
#define VERSION                  "1.0"
#define AUTHOR                   "Toni Helminen"

#define STARTPOS                 "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define MAX_MOVES                218
#define MAX_TOKENS               32
#define BITBOARD                 unsigned long long int
#define MEGABYTE                 (1 << 20)
#define WHITE                    (BRD->white[0] | BRD->white[1] | BRD->white[2] | BRD->white[3] | BRD->white[4] | BRD->white[5])
#define BLACK                    (BRD->black[0] | BRD->black[1] | BRD->black[2] | BRD->black[3] | BRD->black[4] | BRD->black[5])
#define BOTH                     (WHITE | BLACK)

///
/// Macros
///

#define X(a)                     ((a) & 7)
#define Y(a)                     ((a) >> 3)
#define INT(a)                   ((int) (a))
#define DOUBLE(f)                ((double) (f))
#define MAX(a, b)                (((a) > (b)) ? (a) : (b))
#define MIN(a, b)                (((a) < (b)) ? (a) : (b))
#define ULL(a)                   ((BITBOARD) (a))
#define RESET(a)                 memset((a), 0, sizeof((a)))
#define BIT(a)                   (0x1ULL << (a))
#define LSB(b)                   __builtin_ctzll(b)
#define POPCOUNT(b)              __builtin_popcountll(b)
#define MYASSERT(test)           if ( ! (test)) {P("LastEmperor error: Line: %i", __LINE__); exit(EXIT_FAILURE);}
#define BISHOP_MAGIC_INDEX(i, a) ((((a) & BISHOP_MASK[(i)]) * BISHOP_MAGIC[(i)]) >> 55)
#define ROOK_MAGIC_INDEX(i, a)   ((((a) & ROOK_MASK[(i)]) * ROOK_MAGIC[(i)]) >> 52)
#define BISHOP_MOVES(i, a)       BISHOP_MAGIC_MOVES[(i)][BISHOP_MAGIC_INDEX((i), (a))]
#define ROOK_MOVES(i, a)         ROOK_MAGIC_MOVES[(i)][ROOK_MAGIC_INDEX((i), (a))]

///
/// Structs
///

typedef struct {
  BITBOARD white[6];    // White bitboards
  BITBOARD black[6];    // Black bitboards
  BITBOARD hash;        // Hash position
  char board[64];       // Pieces
  char ep;              // En passant
  unsigned char from;   // Move-from square
  unsigned char to;     // Move-to square
  unsigned char castle; // Castling rights
} BOARD_T;

typedef struct {
  BITBOARD hash;
  BITBOARD nodes;
  int depth;
} HASHTABLE_ENTRY_T;

typedef struct {
  HASHTABLE_ENTRY_T *array;
  BITBOARD size; // Size in bytes
  int count;     // Number of entries
  int key;       // Hash key
} HASHTABLE_T;

///
/// LastEmperor headers
///

#include "fdec.h"
#include "data.h"
#include "perft.h"

///
/// Search : Static ( global ) variables
///

static HASHTABLE_T MYHASH = {0};
static BOARD_T *MOVES = 0;
static int MOVES_N = 0;

///
/// Board
///

static const BOARD_T BRD_EMPTY = {0};
static BOARD_T BRD_2 = {0};
static BOARD_T *BRD = &BRD_2;
static BOARD_T *BRD_ORIGINAL = 0;
static bool WTM = 0;

///
/// Move generator
///

static int KING_W = 0;
static int KING_B = 0;
static int ROOK_W[2] = {0};
static int ROOK_B[2] = {0};
static BITBOARD CASTLE_W[2] = {0};
static BITBOARD CASTLE_B[2] = {0};
static BITBOARD CASTLE_EMPTY_W[2] = {0};
static BITBOARD CASTLE_EMPTY_B[2] = {0};
static BITBOARD BISHOP_MAGIC_MOVES[64][512] = {{0}};
static BITBOARD ROOK_MAGIC_MOVES[64][4096] = {{0}};

///
/// Zobrist
///

static BITBOARD ZOBRIST_BOARD[13][64] = {{0}};
static BITBOARD ZOBRIST_EP[64] = {0};
static BITBOARD ZOBRIST_CASTLE[16] = {0};
static BITBOARD ZOBRIST_WTM[2] = {0};

///
/// Misc
///

static BITBOARD RANDOM_SEED = 131783;
static BITBOARD SUITE_TOTAL_TIME = 0;
static int TOKENS_N = 0;
static int TOKENS_I = 0;
static char POSITION_FEN[128] = {0};
static char TOKENS[MAX_TOKENS][128] = {{0}};

///
/// Utils
///

static inline /* <- make me faster! */ int Max(const int a, const int b)
{
  return a > b ? a : b;
}

static inline int Min(const int a, const int b)
{
  return a < b ? a : b;
}

static int Between(const int a, const int b, const int c)
{
  return Max(a, Min(b, c));
}

static bool Equal_strings(const char *s1, const char *s2)
{
  return strcmp(s1, s2) ? 0 : 1;
}

static BITBOARD Nps(const BITBOARD nodes, const BITBOARD ms)
{
  return ms < 1 ? 0 : (1000 * nodes) / ms;
}

static void P(const char *format, ...)
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

static BITBOARD Now()
{
  struct timeval tv;

  MYASSERT(gettimeofday(&tv, NULL) == 0)
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
static BITBOARD Random_bb()
{
  static BITBOARD a = 12311227ULL;
  static BITBOARD b = 1931311ULL;
  static BITBOARD c = 13138141ULL;

  a ^= b + c;
  b ^= b * c + 0x1717711ULL;
  c *= 3;
  c += 1;
#define MIXER(a) (((a) << 7) ^ ((a) >> 5))
  return MIXER(a) ^ MIXER(b) ^ MIXER(c);
}

static BITBOARD Random_u64()
{
  int i;
  BITBOARD ret = 0;

  for (i = 0; i < 8; i++)
    ret ^= Random_bb() << (8 * i);
  return ret;
}

static bool On_board(const int x, const int y)
{
  return x >= 0 && y >= 0 && x <= 7 && y <= 7;
}

static const char *Get_time_string()
{
  static char str[64] = {0};
  time_t tmr;
  struct tm *tmem;

  time(&tmr);
  tmem = localtime(&tmr);
  strftime(str, 64, "%d.%m.%Y %X", tmem);
  return str;
}

///
/// Debug
///

static void Debug_tokens()
{
  int i;

  P("TOKENS ( %i ) :", TOKENS_N);
  for (i = 0; i < TOKENS_N; i++)
    P("%i. %s", i, TOKENS[i]);
}

static void Debug_log(const char *str)
{
  FILE *file = fopen("LastEmperor-log.txt", "a+");

  fprintf(file, "%s\n%s\n:::\n", Get_time_string(), str);
  fclose(file);
}

///
/// Token stuff
///

static void Token_add(const char *token)
{
  MYASSERT(TOKENS_N + 2 < MAX_TOKENS)
  strcpy(TOKENS[TOKENS_N], token);
  TOKENS_N++;
}

static void Token_reset()
{
  TOKENS_I = 0;
  TOKENS_N = 0;
}

static const char *Token_current()
{
  return Token_ok() ? TOKENS[TOKENS_I] : "\0";
}

static void Token_pop()
{
  TOKENS_I++;
}

static bool Token_ok()
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
    P("LastEmperor error ( Unexpected token ) : '%s'", Token_current());
    exit(EXIT_FAILURE);
  }
  Token_pop();
}

static bool Token_next(const char *token)
{
  if ( ! Token_is(token))
    return 0;
  Token_pop();
  return 1;
}

static int Token_next_int()
{
  int ret = 0;

  if (Token_ok() && ! Token_is(";")) { // Assume number
    ret = atoi(TOKENS[TOKENS_I]);
    Token_pop();
  }
  return ret;
}

///
/// Board stuff
///

static void Build_bitboards()
{
  int i;

  RESET(BRD->white);
  RESET(BRD->black);
  for (i = 0; i < 64; i++)
    if (BRD->board[i] > 0)
      BRD->white[BRD->board[i] - 1] |= BIT(i);
    else if (BRD->board[i] < 0)
      BRD->black[-BRD->board[i] - 1] |= BIT(i);
}

static BITBOARD Fill(int from, const int to)
{
  BITBOARD ret = BIT(from);
  const int diff = from > to ? -1 : 1;

  if (from < 0 || to < 0 || from > 63 || to > 63)
    return 0;
  if (from == to)
    return ret;
  while (from != to) {
    from += diff;
    ret |= BIT(from);
  }
  return ret;
}

static void Find_castling_rooks_and_kings()
{
  int i;

  RESET(ROOK_W);
  RESET(ROOK_B);
  KING_W = 0;
  KING_B = 0;
  for (i = 0; i < 64; i++)
    if (BRD->board[i] == 6)
      KING_W = i;
  for (i = KING_W + 1; i < 8; i++)
    if (BRD->board[i] == 4)
      ROOK_W[0] = i;
  for (i = KING_W - 1; i > -1; i--)
    if (BRD->board[i] == 4)
      ROOK_W[1] = i;
  for (i = 0; i < 64; i++)
    if (BRD->board[i] == -6)
      KING_B = i;
  for (i = KING_B + 1; i < 64; i++)
    if (BRD->board[i] == -4)
      ROOK_B[0] = i;
  for (i = KING_B - 1; i > 64 - 8 - 1; i--)
    if (BRD->board[i] == -4)
      ROOK_B[1] = i;
}

static void Build_castle_bitboards()
{
  int i;
  const BITBOARD rank1 = 0xFFULL;
  const BITBOARD rank8 = 0xFF00000000000000ULL;

  CASTLE_W[0] = Fill(KING_W, 6);
  CASTLE_W[1] = Fill(KING_W, 2);
  CASTLE_B[0] = Fill(KING_B, 56 + 6);
  CASTLE_B[1] = Fill(KING_B, 56 + 2);
  CASTLE_EMPTY_W[0] = (CASTLE_W[0] | Fill(ROOK_W[0], 5)) ^ (BIT(KING_W) | BIT(ROOK_W[0]));
  CASTLE_EMPTY_B[0] = (CASTLE_B[0] | Fill(ROOK_B[0], 56 + 5)) ^ (BIT(KING_B) | BIT(ROOK_B[0]));
  CASTLE_EMPTY_W[1] = (CASTLE_W[1] | Fill(ROOK_W[1], 3)) ^ (BIT(KING_W) | BIT(ROOK_W[1]));
  CASTLE_EMPTY_B[1] = (CASTLE_B[1] | Fill(ROOK_B[1], 56 + 3)) ^ (BIT(KING_B) | BIT(ROOK_B[1]));
  for (i = 0; i < 2; i++) {
    CASTLE_EMPTY_W[i] &= rank1;
    CASTLE_W[i] &= rank1;
    CASTLE_EMPTY_B[i] &= rank8;
    CASTLE_B[i] &= rank8;
  }
}

static int Piece(const char piece)
{
  int i;

  for (i = 0; i < 6; i++)
    if (piece == "pnbrqk"[i])
      return -i - 1;
    else if (piece == "PNBRQK"[i])
      return i + 1;
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

static void Fen_wtm(const char *fen)
{
  WTM = *fen == 'w';
}

static void Fen_KQkq(const char *fen)
{
  int t;

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
      t = *fen - 'A';
      if (t > KING_W) {
        ROOK_W[0] = t;
        BRD->castle |= 1;
      } else if (t < KING_W) {
        ROOK_W[1] = t;
        BRD->castle |= 2;
      }
    } else if (*fen >= 'a' && *fen <= 'h') {
      t = *fen - 'a';
      if (t > X(KING_B)) {
        ROOK_B[0] = 56 + t;
        BRD->castle |= 4;
      } else if (t < X(KING_B)) {
        ROOK_B[1] = 56 + t;
        BRD->castle |= 8;
      }
    }
    fen++;
  }
}

static void Fen_ep(const char *fen)
{
  if (*fen == '-' || *fen == '\0' || *(fen + 1) == '\0')
    return;
  BRD->ep = *fen - 'a';
  fen++;
  BRD->ep += 8 * (*fen - '1');
}

// https://en.wikipedia.org/wiki/Forsyth-Edwards_Notation
static void Fen_create(const char *fen)
{
  char str[256] = {0};
  int len = 0;

#define MUNCH() while (*fen == ' ') fen++; if (*fen == '\0') return; len = 0; while (*fen != ' ' && *fen != '\0') \
    {MYASSERT(len < 256) str[len] = *fen; len++; str[len] = '\0'; fen++;}
  MUNCH()
  Fen_board(str);
  MUNCH()
  Fen_wtm(str);
  MUNCH()
  Find_castling_rooks_and_kings();
  Fen_KQkq(str);
  Build_castle_bitboards();
  MUNCH()
  Fen_ep(str);
}

static void Fen(const char *fen)
{
  BRD_2 = BRD_EMPTY;
  BRD = &BRD_2;
  WTM = 1;
  BRD->ep = -1;
  RESET(ROOK_W);
  RESET(ROOK_B);
  KING_W = 0;
  KING_B = 0;
  Fen_create(fen);
  Build_bitboards();
  BRD->hash = Hash(WTM);
  Assume_legal_position();
}

static void Assume_legal_position()
{
  int i;

  // Only 1 king per side
  MYASSERT(POPCOUNT(BRD->white[5]) == 1 && POPCOUNT(BRD->black[5]) == 1)
  // Kings on correct places
  MYASSERT(BRD->board[LSB(BRD->white[5])] == 6 && BRD->board[LSB(BRD->black[5])] == -6)
  for (i = 0; i < 64; i++) { // Pieces where they should be
    if (BRD->board[i] > 0) {
      MYASSERT((BRD->board[i] <= 6) && (BIT(i) & BRD->white[BRD->board[i] - 1]))
    } else if (BRD->board[i] < 0) {
      MYASSERT((BRD->board[i] >= -6) && (BIT(i) & BRD->black[-BRD->board[i] - 1]))
    } else {
      MYASSERT(! (BIT(i) & BOTH))
    }
  }
  // Check en passant validity
  MYASSERT(BRD->ep >= -1 && BRD->ep <= 63)
  if (BRD->ep != -1) {
    if (WTM) {
      MYASSERT(Y(BRD->ep) == 5)
    } else {
      MYASSERT(Y(BRD->ep) == 2)
    }
  }
  // Make sure castling rooks are in correct place
  MYASSERT((BRD->castle & 1) ? (X(ROOK_W[0]) >= X(KING_W) && BRD->board[ROOK_W[0]] == 4) : 1)
  MYASSERT((BRD->castle & 2) ? (X(ROOK_W[1]) <= X(KING_W) && BRD->board[ROOK_W[1]] == 4) : 1)
  MYASSERT((BRD->castle & 4) ? (X(ROOK_B[0]) >= X(KING_B) && BRD->board[ROOK_B[0]] == -4) : 1)
  MYASSERT((BRD->castle & 8) ? (X(ROOK_B[1]) <= X(KING_B) && BRD->board[ROOK_B[1]] == -4) : 1)
  // King can't be captured next
  MYASSERT(WTM ? ( ! Checks_w()) : ( ! Checks_b()))
}

///
/// Checks
///

static bool Checks_castle_w(BITBOARD squares)
{
  int pos;
  const BITBOARD both = BOTH;
  const BITBOARD bishop_plus_queen = BRD->white[2] | BRD->white[4];
  const BITBOARD rook_plus_queen = BRD->white[3] | BRD->white[4];

  while (squares) {
    pos = LSB(squares);
    squares &= squares - 1;
    if ((KNIGHT_MOVES[pos] & BRD->white[1]) |
        (BISHOP_MOVES(pos, both) & bishop_plus_queen) |
        (ROOK_MOVES(pos, both) & rook_plus_queen) |
        (PAWN_CHECKS_B[pos] & BRD->white[0]) |
        (KING_MOVES[pos] & BRD->white[5]))
      return 1;
  }
  return 0;
}

static bool Checks_castle_b(BITBOARD squares)
{
  int pos;
  const BITBOARD both = BOTH;
  const BITBOARD bishop_plus_queen = BRD->black[2] | BRD->black[4];
  const BITBOARD rook_plus_queen = BRD->black[3] | BRD->black[4];

  while (squares) {
    pos = LSB(squares);
    squares &= squares - 1;
    if ((KNIGHT_MOVES[pos] & BRD->black[1]) |
        (BISHOP_MOVES(pos, both) & bishop_plus_queen) |
        (ROOK_MOVES(pos, both) & rook_plus_queen) |
        (PAWN_CHECKS_W[pos] & BRD->black[0]) |
        (KING_MOVES[pos] & BRD->black[5]))
      return 1;
  }
  return 0;
}

static bool Checks_w()
{
  const BITBOARD both = BOTH;
  const int king_i = LSB(BRD->black[5]);

  return (
      (ROOK_MOVES(king_i, both) & (BRD->white[3] | BRD->white[4])) |
      (BISHOP_MOVES(king_i, both) & (BRD->white[2] | BRD->white[4])) |
      (KNIGHT_MOVES[king_i] & BRD->white[1]) |
      (PAWN_CHECKS_B[king_i] & BRD->white[0]) |
      (KING_MOVES[king_i] & BRD->white[5]));
}

static bool Checks_b()
{
  const BITBOARD both = BOTH;
  const int king_i = LSB(BRD->white[5]);

  return (
      (ROOK_MOVES(king_i, both) & (BRD->black[3] | BRD->black[4])) |
      (BISHOP_MOVES(king_i, both) & (BRD->black[2] | BRD->black[4])) |
      (KNIGHT_MOVES[king_i] & BRD->black[1]) |
      (PAWN_CHECKS_W[king_i] & BRD->black[0]) |
      (KING_MOVES[king_i] & BRD->black[5]));
}

///
/// Mgen
///

static BITBOARD Mgen_slider_real(const int *slider_moves, const int pos, const BITBOARD moves)
{
  BITBOARD t;
  int j, k, x2, y2;
  const int x = X(pos);
  const int y = Y(pos);
  BITBOARD possible_moves = 0;

  for (j = 0; j < 4; j++)
    for (k = 1; k < 8; k++) {
      x2 = x + k * slider_moves[2 * j];
      y2 = y + k * slider_moves[2 * j + 1];
      if ( ! On_board(x2, y2))
        break;
      t = BIT(x2 + 8 * y2);
      possible_moves |= t;
      if (t & moves)
        break;
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

static void Handle_castling_w(const int from, const int to)
{
  MOVES[MOVES_N] = *BRD;
  BRD = &MOVES[MOVES_N];
  BRD->ep = -1;
  BRD->from = from;
  BRD->to = to;
  BRD->castle &= 4 | 8;
}

static void Add_castle_O_O_w()
{
  if (Checks_castle_b(CASTLE_W[0]))
    return;
  Handle_castling_w(KING_W, 6);
  BRD->board[ROOK_W[0]] = 0;
  BRD->board[KING_W] = 0;
  BRD->board[5] = 4;
  BRD->board[6] = 6;
  BRD->white[3] ^= BIT(ROOK_W[0]);
  BRD->white[5] ^= BIT(KING_W);
  BRD->white[3] |= BIT(5);
  BRD->white[5] |= BIT(6);
  if (Checks_b())
    return;
  assert(BRD->white[5]);
  BRD->hash ^= ZOBRIST_WTM[0] ^
      ZOBRIST_WTM[1] ^
      ZOBRIST_EP[BRD_ORIGINAL->ep + 1] ^
      ZOBRIST_EP[BRD->ep + 1] ^
      ZOBRIST_CASTLE[BRD_ORIGINAL->castle] ^
      ZOBRIST_CASTLE[BRD->castle] ^
      ZOBRIST_BOARD[6 + 6][KING_W] ^
      ZOBRIST_BOARD[4 + 6][ROOK_W[0]] ^
      ZOBRIST_BOARD[6 + 6][6] ^
  ZOBRIST_BOARD[4 + 6][5];
  MOVES_N++;
}

static void Add_castle_O_O_O_w()
{
  if (Checks_castle_b(CASTLE_W[1]))
    return;
  Handle_castling_w(KING_W, 2);
  BRD->board[ROOK_W[1]] = 0;
  BRD->board[KING_W] = 0;
  BRD->board[3] = 4;
  BRD->board[2] = 6;
  BRD->white[3] ^= BIT(ROOK_W[1]);
  BRD->white[5] ^= BIT(KING_W);
  BRD->white[3] |= BIT(3);
  BRD->white[5] |= BIT(2);
  if (Checks_b())
    return;
  assert(BRD->white[5]);
  BRD->hash ^= ZOBRIST_WTM[0] ^
      ZOBRIST_WTM[1] ^
      ZOBRIST_EP[BRD_ORIGINAL->ep + 1] ^
      ZOBRIST_EP[BRD->ep + 1] ^
      ZOBRIST_CASTLE[BRD_ORIGINAL->castle] ^
      ZOBRIST_CASTLE[BRD->castle] ^
      ZOBRIST_BOARD[6 + 6][KING_W] ^
      ZOBRIST_BOARD[4 + 6][ROOK_W[1]] ^
      ZOBRIST_BOARD[6 + 6][2] ^
      ZOBRIST_BOARD[4 + 6][3];
  MOVES_N++;
}

static void Castling_moves_w(const BITBOARD both)
{
  if ((BRD->castle & 1) && ! (CASTLE_EMPTY_W[0] & both)) {
    Add_castle_O_O_w();
    BRD = BRD_ORIGINAL;
  }
  if ((BRD->castle & 2) && ! (CASTLE_EMPTY_W[1] & both)) {
    Add_castle_O_O_O_w();
    BRD = BRD_ORIGINAL;
  }
}

static void Handle_castling_b(const int from, const int to)
{
  MOVES[MOVES_N] = *BRD;
  BRD = &MOVES[MOVES_N];
  BRD->ep = -1;
  BRD->from = from;
  BRD->to = to;
  BRD->castle &= 1 | 2;
}

static void Add_castle_O_O_b()
{
  if (Checks_castle_w(CASTLE_B[0]))
    return;
  Handle_castling_b(KING_B, 56 + 6);
  BRD->board[ROOK_B[0]] = 0;
  BRD->board[KING_B] = 0;
  BRD->board[56 + 5] = -4;
  BRD->board[56 + 6] = -6;
  BRD->black[3] ^= BIT(ROOK_B[0]);
  BRD->black[5] ^= BIT(KING_B);
  BRD->black[3] |= BIT(56 + 5);
  BRD->black[5] |= BIT(56 + 6);
  if (Checks_w())
    return;
  assert(BRD->black[5]);
  BRD->hash ^= ZOBRIST_WTM[0] ^
      ZOBRIST_WTM[1] ^
      ZOBRIST_EP[BRD_ORIGINAL->ep + 1] ^
      ZOBRIST_EP[BRD->ep + 1] ^
      ZOBRIST_CASTLE[BRD_ORIGINAL->castle] ^
      ZOBRIST_CASTLE[BRD->castle] ^
      ZOBRIST_BOARD[-6 + 6][KING_B] ^
      ZOBRIST_BOARD[-4 + 6][ROOK_B[0]] ^
      ZOBRIST_BOARD[-6 + 6][56 + 6] ^
      ZOBRIST_BOARD[-4 + 6][56 + 5];
  MOVES_N++;
}

static void Add_castle_O_O_O_b()
{
  if (Checks_castle_w(CASTLE_B[1]))
    return;
  assert(BRD->board[ROOK_B[1]] == -4 && BRD->board[KING_B] == -6);
  Handle_castling_b(KING_B, 56 + 2);
  BRD->board[ROOK_B[1]] = 0;
  BRD->board[KING_B] = 0;
  BRD->board[56 + 3] = -4;
  BRD->board[56 + 2] = -6;
  BRD->black[3] ^= BIT(ROOK_B[1]);
  BRD->black[5] ^= BIT(KING_B);
  BRD->black[3] |= BIT(56 + 3);
  BRD->black[5] |= BIT(56 + 2);
  if (Checks_w())
    return;
  assert(BRD->black[5]);
  BRD->hash ^= ZOBRIST_WTM[0] ^
      ZOBRIST_WTM[1] ^
      ZOBRIST_EP[BRD_ORIGINAL->ep + 1] ^
      ZOBRIST_EP[BRD->ep + 1] ^
      ZOBRIST_CASTLE[BRD_ORIGINAL->castle] ^
      ZOBRIST_CASTLE[BRD->castle] ^
      ZOBRIST_BOARD[-6 + 6][KING_B] ^
      ZOBRIST_BOARD[-4 + 6][ROOK_B[1]] ^
      ZOBRIST_BOARD[-6 + 6][56 + 2] ^
      ZOBRIST_BOARD[-4 + 6][56 + 3];
  MOVES_N++;
}

static void Castling_moves_b(const BITBOARD both)
{
  if ((BRD->castle & 4) && ! (CASTLE_EMPTY_B[0] & both)) {
    Add_castle_O_O_b();
    BRD = BRD_ORIGINAL;
  }
  if ((BRD->castle & 8) && ! (CASTLE_EMPTY_B[1] & both)) {
    Add_castle_O_O_O_b();
    BRD = BRD_ORIGINAL;
  }
}

static void Check_castle_rights_w()
{
  if (BRD->board[KING_W] != 6) {
    BRD->castle &= 4 | 8;
    return;
  }
  if (BRD->board[ROOK_W[0]] != 4)
    BRD->castle &= 2 | 4 | 8;
  if (BRD->board[ROOK_W[1]] != 4)
    BRD->castle &= 1 | 4 | 8;
}

static void Check_castle_rights_b()
{
  if (BRD->board[KING_B] != -6) {
    BRD->castle &= 1 | 2;
    return;
  }
  if (BRD->board[ROOK_B[0]] != -4)
    BRD->castle &= 1 | 2 | 8;
  if (BRD->board[ROOK_B[1]] != -4)
    BRD->castle &= 1 | 2 | 4;
}

static void Handle_castle_rights()
{
  if ( ! BRD->castle) // Make LastEmperor castle ASAP for speedups
    return;
  Check_castle_rights_w();
  Check_castle_rights_b();
}

static void Add_underpromotion_w(const BOARD_T *board, const int piece, const int to)
{
  MOVES[MOVES_N] = *board;
  BRD = &MOVES[MOVES_N];
  BRD->board[to] = piece + 1;
  BRD->hash ^= ZOBRIST_BOARD[5 + 6][to];
  BRD->hash ^= ZOBRIST_BOARD[piece + 1 + 6][to];
  BRD->white[4] ^= BIT(to);
  BRD->white[piece] |= BIT(to);
  MOVES_N++;
}

static bool Add_pawn_w(const int from, const int to)
{
  BOARD_T *board;

  if (to > 55) {
    BRD->white[0] ^= BIT(to);
    BRD->board[to] = 5;
    BRD->white[4] |= BIT(to);
    BRD->hash ^= ZOBRIST_EP[BRD->ep + 1] ^ ZOBRIST_BOARD[5 + 6][to];
    if (Checks_b())
      return 1;
    Handle_castle_rights();
    BRD->hash ^= ZOBRIST_CASTLE[BRD->castle];
    MOVES_N++;
    board = BRD;
    Add_underpromotion_w(board, 1, to);
    Add_underpromotion_w(board, 2, to);
    Add_underpromotion_w(board, 3, to);
    return 1;
  } else if (to == BRD_ORIGINAL->ep) {
    BRD->board[to - 8] = 0;
    BRD->hash ^= ZOBRIST_BOARD[-1 + 6][to - 8];
    BRD->black[0] ^= BIT(to - 8);
  } else if (Y(from) == 1 && Y(to) == 3) {
    BRD->ep = to - 8;
  }
  return 0;
}

static void Add_w(const int from, const int to)
{
  const int me = BRD->board[from];
  const int eat = BRD->board[to];

  MOVES[MOVES_N] = *BRD;
  BRD = &MOVES[MOVES_N];
  BRD->from = from;
  BRD->to = to;
  BRD->ep = -1;
  BRD->board[to] = me;
  BRD->board[from] = 0;
  BRD->white[me - 1] = (BRD->white[me - 1] ^ BIT(from)) | BIT(to);
  BRD->hash ^= ZOBRIST_WTM[0] ^
      ZOBRIST_WTM[1] ^
      ZOBRIST_EP[BRD_ORIGINAL->ep + 1] ^
      ZOBRIST_CASTLE[BRD_ORIGINAL->castle] ^
      ZOBRIST_BOARD[me + 6][from];
  if (eat) {
    BRD->hash ^= ZOBRIST_BOARD[eat + 6][to];
    BRD->black[-eat - 1] ^= BIT(to);
  }
  if (BRD->board[to] == 1 && Add_pawn_w(from, to))
    return;
  if (Checks_b())
    return;
  Handle_castle_rights();
  BRD->hash ^= ZOBRIST_BOARD[me + 6][to] ^ ZOBRIST_CASTLE[BRD->castle] ^ ZOBRIST_EP[BRD->ep + 1];
  MOVES_N++;
  assert(MOVES_N < MAX_MOVES);
}

static void Add_underpromotion_b(const BOARD_T *board, const int piece, const int to)
{
  MOVES[MOVES_N] = *board;
  BRD = &MOVES[MOVES_N];
  BRD->board[to] = piece - 1;
  BRD->hash ^= ZOBRIST_BOARD[-5 + 6][to];
  BRD->hash ^= ZOBRIST_BOARD[piece - 1 + 6][to];
  BRD->black[4] ^= BIT(to);
  BRD->black[-(piece)] |= BIT(to);
  MOVES_N++;
}

static bool Add_pawn_b(const int from, const int to)
{
  BOARD_T *board;

  if (to < 8) {
    BRD->black[0] ^= BIT(to);
    BRD->board[to] = -5;
    BRD->black[4] |= BIT(to);
    BRD->hash ^= ZOBRIST_EP[BRD->ep + 1] ^ ZOBRIST_BOARD[-5 + 6][to];
    if (Checks_w())
      return 1;
    Handle_castle_rights();
    BRD->hash ^= ZOBRIST_CASTLE[BRD->castle];
    MOVES_N++;
    board = BRD;
    Add_underpromotion_b(board, -1, to);
    Add_underpromotion_b(board, -2, to);
    Add_underpromotion_b(board, -3, to);
    return 1;
  } else if (to == BRD_ORIGINAL->ep) {
    BRD->board[to + 8] = 0;
    BRD->hash ^= ZOBRIST_BOARD[1 + 6][to + 8];
    BRD->white[0] ^= BIT(to + 8);
  } else if (Y(from) == 6 && Y(to) == 4) {
    BRD->ep = to + 8;
  }
  return 0;
}

static void Add_b(const int from, const int to)
{
  const int me = BRD->board[from];
  const int eat = BRD->board[to];

  MOVES[MOVES_N] = *BRD;
  BRD = &MOVES[MOVES_N];
  BRD->from = from;
  BRD->to = to;
  BRD->ep = -1;
  BRD->board[to] = me;
  BRD->board[from] = 0;
  BRD->black[-me - 1] = (BRD->black[-me - 1] ^ BIT(from)) | BIT(to);
  BRD->hash ^= ZOBRIST_WTM[0] ^
      ZOBRIST_WTM[1] ^
      ZOBRIST_EP[BRD_ORIGINAL->ep + 1] ^
      ZOBRIST_CASTLE[BRD_ORIGINAL->castle] ^
      ZOBRIST_BOARD[me + 6][from];
  if (eat) {
    BRD->hash ^= ZOBRIST_BOARD[eat + 6][to];
    BRD->white[eat - 1] ^= BIT(to);
  }
  if ((BRD->board[to] == -1 && Add_pawn_b(from, to)))
    return;
  if (Checks_w())
    return;
  Handle_castle_rights();
  BRD->hash ^= ZOBRIST_BOARD[me + 6][to] ^ ZOBRIST_CASTLE[BRD->castle] ^ ZOBRIST_EP[BRD->ep + 1];
  MOVES_N++;
  assert(MOVES_N < MAX_MOVES);
}

static void Add_moves_w(const int from, BITBOARD moves)
{
  while (moves) {
    Add_w(from, LSB(moves));
    moves &= moves - 1;
    BRD = BRD_ORIGINAL;
  }
}

static void Add_moves_b(const int from, BITBOARD moves)
{
  while (moves) {
    Add_b(from, LSB(moves));
    moves &= moves - 1;
    BRD = BRD_ORIGINAL;
  }
}

#define POP(/* Once you pop you can't stop */) pos = LSB(pieces); pieces &= pieces - 1

static void Mgen_all_w()
{
  int pos;
  BITBOARD pieces;
  const BITBOARD white = WHITE;
  const BITBOARD black = BLACK;
  const BITBOARD both  = white | black;
  const BITBOARD empty = ~both;
  const BITBOARD good  = ~white;
  BITBOARD pawn_sq     = black;

  assert( ! Checks_w());
  // Pawns
  if (BRD->ep >= 0)
    pawn_sq |= BIT(BRD->ep) & 0x0000FF0000000000ULL;
  pieces = BRD->white[0];
  while (pieces) {
    POP();
    Add_moves_w(pos, PAWN_CHECKS_W[pos] & pawn_sq);
    if (Y(pos) == 1) {
      if (PAWN_MOVES_W[pos] & empty)
        Add_moves_w(pos, PAWN_MOVES_2_W[pos] & empty);
    } else {
      Add_moves_w(pos, PAWN_MOVES_W[pos] & empty);
    }
  }
  // Knights
  pieces = BRD->white[1];
  while (pieces) {
    POP();
    Add_moves_w(pos, KNIGHT_MOVES[pos] & good);
  }
  // Bishops + Queens
  pieces = BRD->white[2] | BRD->white[4];
  while (pieces) {
    POP();
    Add_moves_w(pos, BISHOP_MOVES(pos, both) & good);
  }
  // Rooks + Queens
  pieces = BRD->white[3] | BRD->white[4];
  while (pieces) {
    POP();
    Add_moves_w(pos, ROOK_MOVES(pos, both) & good);
  }
  // Kings
  pos = LSB(BRD->white[5]);
  Add_moves_w(pos, KING_MOVES[pos] & good);
  Castling_moves_w(both);
}

static void Mgen_all_b()
{
  int pos;
  BITBOARD pieces;
  const BITBOARD white = WHITE;
  const BITBOARD black = BLACK;
  const BITBOARD both  = white | black;
  const BITBOARD empty = ~both;
  const BITBOARD good  = ~black;
  BITBOARD pawn_sq     = white;

  assert( ! Checks_b());
  // Pawns
  if (BRD->ep >= 0)
    pawn_sq |= BIT(BRD->ep) & 0x0000000000FF0000ULL;
  pieces = BRD->black[0];
  while (pieces) {
    POP();
    Add_moves_b(pos, PAWN_CHECKS_B[pos] & pawn_sq);
    if (Y(pos) == 6) {
      if (PAWN_MOVES_B[pos] & empty)
        Add_moves_b(pos, PAWN_MOVES_2_B[pos] & empty);
    } else {
      Add_moves_b(pos, PAWN_MOVES_B[pos] & empty);
    }
  }
  // Knights
  pieces = BRD->black[1];
  while (pieces) {
    POP();
    Add_moves_b(pos, KNIGHT_MOVES[pos] & good);
  }
  // Bishops + Queens
  pieces = BRD->black[2] | BRD->black[4];
  while (pieces) {
    POP();
    Add_moves_b(pos, BISHOP_MOVES(pos, both) & good);
  }
  // Rooks + Queens
  pieces = BRD->black[3] | BRD->black[4];
  while (pieces) {
    POP();
    Add_moves_b(pos, ROOK_MOVES(pos, both) & good);
  }
  // Kings
  pos = LSB(BRD->black[5]);
  Add_moves_b(pos, KING_MOVES[pos] & good);
  Castling_moves_b(both);
}

static inline void Swap(BOARD_T *a, BOARD_T *b)
{
  BOARD_T t = *a;

  *a = *b;
  *b = t;
}

static int Mgen_w(BOARD_T *moves)
{
  MOVES_N = 0;
  MOVES = moves;
  BRD_ORIGINAL = BRD;
  Mgen_all_w();
  return MOVES_N;
}

static int Mgen_b(BOARD_T *moves)
{
  MOVES_N = 0;
  MOVES = moves;
  BRD_ORIGINAL = BRD;
  Mgen_all_b();
  return MOVES_N;
}

///
/// Hash
///

static BITBOARD Hash(const int wtm)
{
  int pos;

  BITBOARD hash = ZOBRIST_EP[BRD->ep + 1] ^ ZOBRIST_WTM[wtm] ^ ZOBRIST_CASTLE[BRD->castle];
  BITBOARD both = BOTH;
  while (both) {
    pos = LSB(both);
    both &= both - 1;
    hash ^= ZOBRIST_BOARD[BRD->board[pos] + 6][pos];
  }
  return hash;
}

static void Hashtable_free_memory(HASHTABLE_T *table)
{
  if (table->size > 0) {
    free(table->array);
    table->array = 0;
    table->size = 0;
  }
}

static void Free_memory()
{
  Hashtable_free_memory(&MYHASH);
}

// Allocate memory for Hashtable ( Evals / Good Moves )
// -- The More The Better --
// [16 MB, 1 PB]
static void Hashtable_set_size(HASHTABLE_T *table, const int usize /* MB */)
{
  BITBOARD size = ULL(usize);

  Hashtable_free_memory(table);
  size = MAX(size, 16);
  size = MIN(size, 1024 * 1024); // 1 PB
  size = (1 << 20) * size; // To MB
  table->size = 1;
  while (table->size <= size) // Calculate needed memory in bytes
    table->size <<= 1;
  table->size >>= 1;
  table->count = INT(table->size / sizeof(HASHTABLE_ENTRY_T));
  table->key = 1;
  while (table->key <= table->count) // Create key according to count
    table->key <<= 1;
  table->key >>= 1;
  //table->depth = 0;
  table->key -= 1; // 1000b = 8d / - 1d / 0111b = 7d
  table->array = (HASHTABLE_ENTRY_T*) calloc(table->count, sizeof(HASHTABLE_ENTRY_T)); // <- Cast for g++
  MYASSERT(table->array != NULL) // Make sure there is enough space
}

///
/// Perft
///

static BITBOARD Get_perft(const BITBOARD hash, const int depth)
{
  const HASHTABLE_ENTRY_T *entry = &MYHASH.array[INT(hash & MYHASH.key)];

  if (entry->hash == hash && entry->depth == depth)
    return entry->nodes;
  return 0;
}

static void Add_perft(const BITBOARD hash, const BITBOARD nodes, const int depth)
{
  HASHTABLE_ENTRY_T *entry = &MYHASH.array[INT(hash & MYHASH.key)];

  if ( ! nodes || (entry->hash == hash && entry->nodes > nodes))
    return;
  entry->depth = depth;
  entry->hash = hash;
  entry->nodes = nodes;
}

static BITBOARD Perft_w(const int depth)
{
  int i, len;
  BOARD_T moves[MAX_MOVES];
  const BITBOARD hash = BRD->hash;
  BITBOARD nodes = Get_perft(hash, depth);

  if (nodes)
    return nodes;
  assert(Hash(1) == BRD->hash);
  len = Mgen_w(moves);
  if (depth <= 0)
    return len;
  nodes = 0;
  for (i = 0; i < len; i++) {
    BRD = moves + i;
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
  BITBOARD nodes = Get_perft(hash, depth);

  if (nodes)
    return nodes;
  assert(Hash(0) == BRD->hash);
  len = Mgen_b(moves);
  if (depth <= 0)
    return len;
  for (i = 0; i < len; i++) {
    BRD = moves + i;
    nodes += Perft_w(depth - 1);
  }
  Add_perft(hash, nodes, depth);
  return nodes;
}

static BITBOARD Perft(const int depth)
{
  BOARD_T *board = BRD;
  BITBOARD nodes = 1;

  if (depth > 0)
    nodes = WTM ? Perft_w(depth - 1) : Perft_b(depth - 1);
  BRD = board;
  return nodes;
}

static void Padding(const char *str, const int space)
{
  int i;
  int len = space - strlen(str);

  for (i = 0; i < len; i++)
    printf(" ");
  printf("%s", str);
}

#define PHEADER() P("depth          nodes           mnps           time")

static void Perft_final_print(const int depth, const BITBOARD nodes, const BITBOARD ms)
{
  P("\n===");
  PHEADER();
  Perft_print(depth, nodes, ms);
}

static void Perft_print(const int depth, const BITBOARD nodes, const BITBOARD ms)
{
  static char str[32];

  sprintf(str, "%i", depth); Padding(str, 5);
  sprintf(str, "%llu", nodes); Padding(str, 15);
  sprintf(str, "%.3f", 0.000001f * DOUBLE(Nps(nodes, ms))); Padding(str, 15);
  sprintf(str, "%.3f", 0.001f * DOUBLE(ms)); Padding(str, 15);
  printf("\n");
}

static void Perft_run(const int depth)
{
  int i;
  BITBOARD nodes, start_time, diff_time;
  BITBOARD totaltime = 0;
  BITBOARD allnodes = 0;

  P("### Perft ( %i MB ) ###", MYHASH.size / MEGABYTE);
  if (POSITION_FEN[0] == '\0')
    P("\n[ %s ]", STARTPOS);
  else
    P("\n[ %s ]", POSITION_FEN);
  PHEADER();
  for (i = 0; i < depth + 1; i++) {
    start_time = Now();
    nodes = Perft(i);
    diff_time = Now() - start_time;
    totaltime += diff_time;
    allnodes += nodes;
    Perft_print(i, nodes, diff_time);
  }
  Perft_final_print(depth, allnodes, totaltime);
}

static BITBOARD Suite_run(const int suite_i, const int depth)
{
  int i;
  BITBOARD start, diff, nodes;
  BITBOARD all_nodes = 0;
  const BITBOARD *counts = SUITE[suite_i].nodes;

  PHEADER();
  for (i = 0; i < depth; i++) {
    start = Now();
    nodes = Perft(i);
    diff = Now() - start;
    SUITE_TOTAL_TIME += diff;
    all_nodes += nodes;
    Perft_print(i, nodes, diff);
    // Depth 7 and 8 here
    if (i <= 6) {MYASSERT(nodes == counts[i])}
  }
  return all_nodes;
}

static void Suite(const int depth)
{
  int suite_i = 0;
  BITBOARD nodes = 0;

  P("### Running Chess960 suite ( %i MB ) ###", MYHASH.size / MEGABYTE);
  assert(depth >= 1 && depth <= 8);
  SUITE_TOTAL_TIME = 0;
  while (1) {
    if ( ! SUITE[suite_i].nodes[0])
      break;
    Fen(SUITE[suite_i].fen);
    P("\n[ %i: %s ]", suite_i + 1, SUITE[suite_i].fen);
    nodes += Suite_run(suite_i, depth + 1);
    suite_i++;
  }
  Perft_final_print(depth, nodes, SUITE_TOTAL_TIME);
}

///
/// LastEmperor-cli
///

static void Print_help()
{
  P("%s %s by %s", NAME, VERSION, AUTHOR);
  P(SHORT_LICENSE);
  printf("\n");
  P("Usage: lastemperor [COMMAND] [OPTION]?, ...");
  P("> lastemperor -hash 1000 -perft 6 # Set 1024 MB hash and run perft");
  printf("\n");
  P("## LastEmperor Commands ##");
  P("-h(elp)         This help");
  P("-v(ersion)      Show Version");
  P("-fen [FEN]      Set fen");
  P("-id             Verify that LastEmperor works!");
  P("-perft [1..]    Run perft position");
  P("-suite [1..8]   Run perft suite");
  P("-hash N         Set hash in N MB");
  printf("\n");
  P("Full source code, please see: <https://github.com/SamuraiDangyo/LastEmperor/>");
  exit(EXIT_SUCCESS);
}

static void Position_fen()
{
  POSITION_FEN[0] = '\0';
  while (Token_ok() && ! Token_is(";")) {
    MYASSERT(strlen(POSITION_FEN) + strlen(Token_current()) < 127)
    String_join(POSITION_FEN, Token_current());
    String_join(POSITION_FEN, " ");
    Token_pop();
  }
  POSITION_FEN[Max(0, strlen(POSITION_FEN) - 1)] = '\0';
  Fen(POSITION_FEN);
}

static void Command_setfen()
{
  Fen(STARTPOS);
  Position_fen();
}

static void Cli_commands()
{
  Token_expect(";");
  while (Token_ok()) {
    if (Token_next("h") || Token_next("help"))
      Print_help();
    else if (Token_next("v") || Token_next("version"))
      P("%s %s by %s", NAME, VERSION, AUTHOR);
    else if (Token_next("fen"))
      Command_setfen();
    else if (Token_next("suite"))
      Suite(Between(1, Token_next_int(), 6));
    else if (Token_next("perft"))
      Perft_run(Max(1, Token_next_int()));
    else if (Token_next("hash"))
      Hashtable_set_size(&MYHASH, Max(16, Token_next_int()));
    else if (Token_next("id"))
      Suite(3);
    Token_expect(";");
  }
}

///
/// Init
///

static BITBOARD Permutate_bb(const BITBOARD moves, const int index)
{
  int i, popn;
  int total = 0;
  int good[64] = {0};
  BITBOARD permutations = 0;

  for (i = 0; i < 64; i++)
    if (moves & BIT(i)) {
      good[total] = i;
      total++;
    }
  popn = POPCOUNT(moves);
  for (i = 0; i < popn; i++)
    if ((1 << i) & index)
      permutations |= BIT(good[i]);
  return permutations & moves;
}

static void Init_bishop_magics()
{
  int i, j;
  BITBOARD magics, allmoves;

  for (i = 0; i < 64; i++) {
    magics = BISHOP_MOVE_MAGICS[i] & (~BIT(i));
    for (j = 0; j < 512; j++) {
      allmoves = Permutate_bb(magics, j);
      BISHOP_MAGIC_MOVES[i][BISHOP_MAGIC_INDEX(i, allmoves)] = Mgen_bishop_real(i, allmoves);
    }
  }
}

static void Init_rook_magics()
{
  int i, j;
  BITBOARD magics, allmoves;

  for (i = 0; i < 64; i++) {
    magics = ROOK_MOVE_MAGICS[i] & (~BIT(i));
    for (j = 0; j < 4096; j++) {
      allmoves = Permutate_bb(magics, j);
      ROOK_MAGIC_MOVES[i][ROOK_MAGIC_INDEX(i, allmoves)] = Mgen_rook_real(i, allmoves);
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

static void Init_random_seed()
{
  RANDOM_SEED += ULL(time(NULL));
}

static void Init_board()
{
  Fen(STARTPOS);
}

static void Init_zobrist()
{
  int i, j;

  for (i = 0; i < 13; i++)
    for (j = 0; j < 64; j++)
      ZOBRIST_BOARD[i][j] = Random_u64();
  for (i = 0; i < 64; i++)
    ZOBRIST_EP[i] = Random_u64();
  for (i = 0; i < 16; i++)
    ZOBRIST_CASTLE[i] = Random_u64();
  for (i = 0; i < 2; i++)
    ZOBRIST_WTM[i] = Random_u64();
}

static void Init_hash()
{
  Hashtable_set_size(&MYHASH, 256 /* MB */ ); // Default size
}

static void Init()
{
  Init_random_seed();
  Init_bishop_magics();
  Init_rook_magics();
  Init_zobrist();
  Init_hash();
  Init_board();
}

///
/// Run
///

static void Ok()
{
  MYASSERT(sizeof(int) >= 4 /* bytes */)
  MYASSERT(sizeof(double) >= 8 /* bytes */)
  MYASSERT(sizeof(BITBOARD) >= 8 /* bytes */)
  MYASSERT((0x1122334455667788ULL >> 32) == 0x11223344ULL)
  MYASSERT((0x1122334455667788ULL << 32) == 0x5566778800000000ULL)
  MYASSERT((0xFFFFFFFFFFFFFFFFULL & 0x8142241818244281ULL) == 0x8142241818244281ULL)
}

static void Happy()
{
  if (0) { // Makes gcc happy
    Debug_log(Int_to_string(0));
    Debug_tokens();
  }
}

static void Run()
{
  Ok();
  Happy();
  Init();
  Cli_commands();
}

int main(int argc, char **argv)
{
  atexit(Free_memory); // No memory leaks
  Init_tokens(argc, argv);
  Run();
  return EXIT_SUCCESS;
}