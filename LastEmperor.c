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

// Headers

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/time.h>
#include <limits.h>
#include <stdbool.h>

// Constants

#define NAME           "LastEmperor 1.08"
#define STARTPOS       "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define MAX_MOVES      218
#define MAX_TOKENS     32
#define U64            unsigned long long int
#define MEGABYTE       (1 << 20)
#define WHITE()        (BRD->white[0] | BRD->white[1] | BRD->white[2] | BRD->white[3] | BRD->white[4] | BRD->white[5])
#define BLACK()        (BRD->black[0] | BRD->black[1] | BRD->black[2] | BRD->black[3] | BRD->black[4] | BRD->black[5])
#define BOTH()         (WHITE() | BLACK())
#define LICENSE        "GNU General Public License version 3; for details see LICENSE"

// Macros

#define X(a)           ((a) & 7)
#define Y(a)           ((a) >> 3)
#define INT(a)         ((int) (a))
#define DOUBLE(f)      ((double) (f))
#define ULL(a)         ((U64) (a))
#define MAX(a, b)      (((a) > (b)) ? (a) : (b))
#define MIN(a, b)      (((a) < (b)) ? (a) : (b))
#define RESET(a)       memset((a), 0, sizeof((a)))
#define MYASSERT(test) Assert((test), (__LINE__))

// Structs

typedef struct {
  U64 white[6];          // White bitboards
  U64 black[6];          // Black bitboards
  char board[64];        // Pieces
  U64 hash;              // Hash of the position
  char epsq;             // En passant square
  unsigned char crigths; // Castling rights
} BOARD_T;

typedef struct {
  U64 hash;
  U64 nodes;
  int depth;
} HASH_ENTRY_T;

typedef struct {
  HASH_ENTRY_T *array;
  U64 size;
  int count;
  int key;
} HASH_T;

// Prototypes

static inline U64 Bishop_magic_moves(const int, const U64);
static inline U64 Rook_magic_moves(const int, const U64);
static bool Checks_w(void);
static bool Checks_b(void);
static U64 Perft_b(const int);
static void Print_help(void);

// BitBoard magics

static const U64 ROOK_MAGIC[64] = {
  0x548001400080106cULL,0x900184000110820ULL,0x428004200a81080ULL,0x140088082000c40ULL,0x1480020800011400ULL,0x100008804085201ULL,0x2a40220001048140ULL,0x50000810000482aULL,
  0x250020100020a004ULL,0x3101880100900a00ULL,0x200a040a00082002ULL,0x1004300044032084ULL,0x2100408001013ULL,0x21f00440122083ULL,0xa204280406023040ULL,0x2241801020800041ULL,
  0xe10100800208004ULL,0x2010401410080ULL,0x181482000208805ULL,0x4080101000021c00ULL,0xa250210012080022ULL,0x4210641044000827ULL,0x8081a02300d4010ULL,0x8008012000410001ULL,
  0x28c0822120108100ULL,0x500160020aa005ULL,0xc11050088c1000ULL,0x48c00101000a288ULL,0x494a184408028200ULL,0x20880100240006ULL,0x10b4010200081ULL,0x40a200260000490cULL,
  0x22384003800050ULL,0x7102001a008010ULL,0x80020c8010900c0ULL,0x100204082a001060ULL,0x8000118188800428ULL,0x58e0020009140244ULL,0x100145040040188dULL,0x44120220400980ULL,
  0x114001007a00800ULL,0x80a0100516304000ULL,0x7200301488001000ULL,0x1000151040808018ULL,0x3000a200010e0020ULL,0x1000849180802810ULL,0x829100210208080ULL,0x1004050021528004ULL,
  0x61482000c41820b0ULL,0x241001018a401a4ULL,0x45020c009cc04040ULL,0x308210c020081200ULL,0xa000215040040ULL,0x10a6024001928700ULL,0x42c204800c804408ULL,0x30441a28614200ULL,
  0x40100229080420aULL,0x9801084000201103ULL,0x8408622090484202ULL,0x4022001048a0e2ULL,0x280120020049902ULL,0x1200412602009402ULL,0x914900048020884ULL,0x104824281002402ULL
};

static const U64 ROOK_MASK[64] = {
  0x101010101017eULL,0x202020202027cULL,0x404040404047aULL,0x8080808080876ULL,0x1010101010106eULL,0x2020202020205eULL,0x4040404040403eULL,0x8080808080807eULL,
  0x1010101017e00ULL,0x2020202027c00ULL,0x4040404047a00ULL,0x8080808087600ULL,0x10101010106e00ULL,0x20202020205e00ULL,0x40404040403e00ULL,0x80808080807e00ULL,
  0x10101017e0100ULL,0x20202027c0200ULL,0x40404047a0400ULL,0x8080808760800ULL,0x101010106e1000ULL,0x202020205e2000ULL,0x404040403e4000ULL,0x808080807e8000ULL,
  0x101017e010100ULL,0x202027c020200ULL,0x404047a040400ULL,0x8080876080800ULL,0x1010106e101000ULL,0x2020205e202000ULL,0x4040403e404000ULL,0x8080807e808000ULL,
  0x1017e01010100ULL,0x2027c02020200ULL,0x4047a04040400ULL,0x8087608080800ULL,0x10106e10101000ULL,0x20205e20202000ULL,0x40403e40404000ULL,0x80807e80808000ULL,
  0x17e0101010100ULL,0x27c0202020200ULL,0x47a0404040400ULL,0x8760808080800ULL,0x106e1010101000ULL,0x205e2020202000ULL,0x403e4040404000ULL,0x807e8080808000ULL,
  0x7e010101010100ULL,0x7c020202020200ULL,0x7a040404040400ULL,0x76080808080800ULL,0x6e101010101000ULL,0x5e202020202000ULL,0x3e404040404000ULL,0x7e808080808000ULL,
  0x7e01010101010100ULL,0x7c02020202020200ULL,0x7a04040404040400ULL,0x7608080808080800ULL,0x6e10101010101000ULL,0x5e20202020202000ULL,0x3e40404040404000ULL,0x7e80808080808000ULL
};

static const U64 ROOK_MOVE_MAGICS[64] = {
  0x101010101017eULL,0x202020202027cULL,0x404040404047aULL,0x8080808080876ULL,0x1010101010106eULL,0x2020202020205eULL,0x4040404040403eULL,0x8080808080807eULL,
  0x1010101017e00ULL,0x2020202027c00ULL,0x4040404047a00ULL,0x8080808087600ULL,0x10101010106e00ULL,0x20202020205e00ULL,0x40404040403e00ULL,0x80808080807e00ULL,
  0x10101017e0100ULL,0x20202027c0200ULL,0x40404047a0400ULL,0x8080808760800ULL,0x101010106e1000ULL,0x202020205e2000ULL,0x404040403e4000ULL,0x808080807e8000ULL,
  0x101017e010100ULL,0x202027c020200ULL,0x404047a040400ULL,0x8080876080800ULL,0x1010106e101000ULL,0x2020205e202000ULL,0x4040403e404000ULL,0x8080807e808000ULL,
  0x1017e01010100ULL,0x2027c02020200ULL,0x4047a04040400ULL,0x8087608080800ULL,0x10106e10101000ULL,0x20205e20202000ULL,0x40403e40404000ULL,0x80807e80808000ULL,
  0x17e0101010100ULL,0x27c0202020200ULL,0x47a0404040400ULL,0x8760808080800ULL,0x106e1010101000ULL,0x205e2020202000ULL,0x403e4040404000ULL,0x807e8080808000ULL,
  0x7e010101010100ULL,0x7c020202020200ULL,0x7a040404040400ULL,0x76080808080800ULL,0x6e101010101000ULL,0x5e202020202000ULL,0x3e404040404000ULL,0x7e808080808000ULL,
  0x7e01010101010100ULL,0x7c02020202020200ULL,0x7a04040404040400ULL,0x7608080808080800ULL,0x6e10101010101000ULL,0x5e20202020202000ULL,0x3e40404040404000ULL,0x7e80808080808000ULL
};

static const U64 BISHOP_MAGIC[64] = {
  0x2890208600480830ULL,0x324148050f087ULL,0x1402488a86402004ULL,0xc2210a1100044bULL,0x88450040b021110cULL,0xc0407240011ULL,0xd0246940cc101681ULL,0x1022840c2e410060ULL,
  0x4a1804309028d00bULL,0x821880304a2c0ULL,0x134088090100280ULL,0x8102183814c0208ULL,0x518598604083202ULL,0x67104040408690ULL,0x1010040020d000ULL,0x600001028911902ULL,
  0x8810183800c504c4ULL,0x2628200121054640ULL,0x28003000102006ULL,0x4100c204842244ULL,0x1221c50102421430ULL,0x80109046e0844002ULL,0xc128600019010400ULL,0x812218030404c38ULL,
  0x1224152461091c00ULL,0x1c820008124000aULL,0xa004868015010400ULL,0x34c080004202040ULL,0x200100312100c001ULL,0x4030048118314100ULL,0x410000090018ULL,0x142c010480801ULL,
  0x8080841c1d004262ULL,0x81440f004060406ULL,0x400a090008202ULL,0x2204020084280080ULL,0xb820060400008028ULL,0x110041840112010ULL,0x8002080a1c84400ULL,0x212100111040204aULL,
  0x9412118200481012ULL,0x804105002001444cULL,0x103001280823000ULL,0x40088e028080300ULL,0x51020d8080246601ULL,0x4a0a100e0804502aULL,0x5042028328010ULL,0xe000808180020200ULL,
  0x1002020620608101ULL,0x1108300804090c00ULL,0x180404848840841ULL,0x100180040ac80040ULL,0x20840000c1424001ULL,0x82c00400108800ULL,0x28c0493811082aULL,0x214980910400080cULL,
  0x8d1a0210b0c000ULL,0x164c500ca0410cULL,0xc6040804283004ULL,0x14808001a040400ULL,0x180450800222a011ULL,0x600014600490202ULL,0x21040100d903ULL,0x10404821000420ULL
};

static const U64 BISHOP_MASK[64] = {
  0x40201008040200ULL,0x402010080400ULL,0x4020100a00ULL,0x40221400ULL,0x2442800ULL,0x204085000ULL,0x20408102000ULL,0x2040810204000ULL,
  0x20100804020000ULL,0x40201008040000ULL,0x4020100a0000ULL,0x4022140000ULL,0x244280000ULL,0x20408500000ULL,0x2040810200000ULL,0x4081020400000ULL,
  0x10080402000200ULL,0x20100804000400ULL,0x4020100a000a00ULL,0x402214001400ULL,0x24428002800ULL,0x2040850005000ULL,0x4081020002000ULL,0x8102040004000ULL,
  0x8040200020400ULL,0x10080400040800ULL,0x20100a000a1000ULL,0x40221400142200ULL,0x2442800284400ULL,0x4085000500800ULL,0x8102000201000ULL,0x10204000402000ULL,
  0x4020002040800ULL,0x8040004081000ULL,0x100a000a102000ULL,0x22140014224000ULL,0x44280028440200ULL,0x8500050080400ULL,0x10200020100800ULL,0x20400040201000ULL,
  0x2000204081000ULL,0x4000408102000ULL,0xa000a10204000ULL,0x14001422400000ULL,0x28002844020000ULL,0x50005008040200ULL,0x20002010080400ULL,0x40004020100800ULL,
  0x20408102000ULL,0x40810204000ULL,0xa1020400000ULL,0x142240000000ULL,0x284402000000ULL,0x500804020000ULL,0x201008040200ULL,0x402010080400ULL,
  0x2040810204000ULL,0x4081020400000ULL,0xa102040000000ULL,0x14224000000000ULL,0x28440200000000ULL,0x50080402000000ULL,0x20100804020000ULL,0x40201008040200ULL
};

static const U64 BISHOP_MOVE_MAGICS[64] = {
  0x40201008040200ULL,0x402010080400ULL,0x4020100a00ULL,0x40221400ULL,0x2442800ULL,0x204085000ULL,0x20408102000ULL,0x2040810204000ULL,
  0x20100804020000ULL,0x40201008040000ULL,0x4020100a0000ULL,0x4022140000ULL,0x244280000ULL,0x20408500000ULL,0x2040810200000ULL,0x4081020400000ULL,
  0x10080402000200ULL,0x20100804000400ULL,0x4020100a000a00ULL,0x402214001400ULL,0x24428002800ULL,0x2040850005000ULL,0x4081020002000ULL,0x8102040004000ULL,
  0x8040200020400ULL,0x10080400040800ULL,0x20100a000a1000ULL,0x40221400142200ULL,0x2442800284400ULL,0x4085000500800ULL,0x8102000201000ULL,0x10204000402000ULL,
  0x4020002040800ULL,0x8040004081000ULL,0x100a000a102000ULL,0x22140014224000ULL,0x44280028440200ULL,0x8500050080400ULL,0x10200020100800ULL,0x20400040201000ULL,
  0x2000204081000ULL,0x4000408102000ULL,0xa000a10204000ULL,0x14001422400000ULL,0x28002844020000ULL,0x50005008040200ULL,0x20002010080400ULL,0x40004020100800ULL,
  0x20408102000ULL,0x40810204000ULL,0xa1020400000ULL,0x142240000000ULL,0x284402000000ULL,0x500804020000ULL,0x201008040200ULL,0x402010080400ULL,
  0x2040810204000ULL,0x4081020400000ULL,0xa102040000000ULL,0x14224000000000ULL,0x28440200000000ULL,0x50080402000000ULL,0x20100804020000ULL,0x40201008040200ULL
};

// Board

static const BOARD_T BRD_EMPTY   = {0};
static BOARD_T BRD_2             = {0};
static BOARD_T *BRD              = &BRD_2;
static BOARD_T *BRD_ORIGINAL     = 0;
static bool WTM                  = 0;

// Move generator

static HASH_T MYHASH             = {0};
static BOARD_T *MGEN_MOVES       = 0;
static int MGEN_MOVES_N          = 0;
static int KING_W                = 0;
static int KING_B                = 0;
static int ROOK_W[2]             = {0};
static int ROOK_B[2]             = {0};

static U64 MGEN_WHITE            = 0;
static U64 MGEN_BLACK            = 0;
static U64 MGEN_BOTH             = 0;
static U64 MGEN_EMPTY            = 0;
static U64 MGEN_GOOD             = 0;
static U64 MGEN_PAWN_SQ          = 0;

static U64 CASTLE_W[2]           = {0};
static U64 CASTLE_B[2]           = {0};
static U64 CASTLE_EMPTY_W[2]     = {0};
static U64 CASTLE_EMPTY_B[2]     = {0};
static U64 BISHOP_MAGIC_MOVES[64][512] = {{0}};
static U64 ROOK_MAGIC_MOVES[64][4096]  = {{0}};
static U64 KNIGHT_MOVES[64]      = {0};
static U64 KING_MOVES[64]        = {0};
static U64 PAWN_CHECKS_W[64]     = {0};
static U64 PAWN_CHECKS_B[64]     = {0};
static U64 PAWN_1_MOVES_W[64]    = {0};
static U64 PAWN_1_MOVES_B[64]    = {0};
static U64 PAWN_2_MOVES_W[64]    = {0};
static U64 PAWN_2_MOVES_B[64]    = {0};

// Zobrist

static U64 ZOBRIST_BOARD[13][64] = {{0}};
static U64 ZOBRIST_EP[64]        = {0};
static U64 ZOBRIST_CASTLE[16]    = {0};
static U64 ZOBRIST_WTM[2]        = {0};

// Misc

static U64 RANDOM_SEED           = 131783;
static int TOKENS_N              = 0;
static int TOKENS_I              = 0;
static char POSITION_FEN[128]    = STARTPOS;
static char TOKENS[MAX_TOKENS][128] = {{0}};
static char FEN_STR[4][128]         = {{0}};

// static void Debug_tokens(void) {int i; Print("TOKENS ( %i ) :", TOKENS_N); for (i = 0; i < TOKENS_N; i++) Print("%i. %s", i, TOKENS[i]);}
// static void Debug_log(const char *str) {FILE *file = fopen("LastEmperor-log.txt", "a+"); fprintf(file, "%s\n:::\n", str); fclose(file);}

// Utils

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

static U64 Nps(const U64 nodes, const U64 ms)
{
  return ms ? (1000 * nodes) / ms : 0;
}

static inline int Lsb(const U64 bb)
{
  return __builtin_ctzll(bb);
}

static inline int Popcount(const U64 bb)
{
  return __builtin_popcountll(bb);
}

static inline U64 Clear_bit(const U64 bb)
{
  return bb & (bb - 1);
}

static inline U64 Bit(const int nbits)
{
  return 0x1ULL << nbits;
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

static void Assert(const bool test, const int line_number)
{
  if (test) return;
  Print("LastEmperor error: Line: %i", line_number);
  exit(EXIT_FAILURE);
}

static U64 Now(void)
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
static U64 Random_bb(void)
{
  static U64 a = 0x12311227ULL, b = 0x1931311ULL, c = 0x13138141ULL;
  a ^= b + c;
  b ^= b * c + 0x1717711ULL;
  c *= 3;
  c += 1;
#define MIXER(val) (((val) << 7) ^ ((val) >> 5))
  return MIXER(a) ^ MIXER(b) ^ MIXER(c);
}

static U64 Random_u64(void)
{
  int i;
  U64 ret = 0;
  for (i = 0; i < 8; i++) ret ^= Random_bb() << (8 * i);
  return ret;
}

static bool On_board(const int x, const int y)
{
  return x >= 0 && y >= 0 && x <= 7 && y <= 7;
}

// Token stuff

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

static bool Token_ok(void)
{
  return TOKENS_I < TOKENS_N;
}

static const char *Token_current(void)
{
  return Token_ok() ? TOKENS[TOKENS_I] : "\0";
}

static void Token_pop(void)
{
  TOKENS_I++;
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
  if (Token_ok() && ! Token_is(";")) {
    ret = atoi(TOKENS[TOKENS_I]);
    Token_pop();
  }
  return ret;
}

// Board stuff

static void Build_bitboards(void)
{
  RESET(BRD->white);
  RESET(BRD->black);
  for (int i = 0; i < 64; i++)
    if (BRD->board[i] > 0)      BRD->white[ BRD->board[i] - 1] |= Bit(i);
    else if (BRD->board[i] < 0) BRD->black[-BRD->board[i] - 1] |= Bit(i);
}

static U64 Fill(int from, const int to)
{
  U64 ret   = Bit(from);
  const int diff = from > to ? -1 : 1;
  if (from < 0 || to < 0 || from > 63 || to > 63) return 0;
  if (from == to) return ret;
  while (from != to) {
    from += diff;
    ret  |= Bit(from);
  }
  return ret;
}

static void Find_crigths_rooks_and_kings(void)
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
  for (int i = 0; i < 6; i++)
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
      BRD->crigths |= 1;
    } else if (*fen == 'Q') {
      BRD->crigths |= 2;
    } else if (*fen == 'k') {
      BRD->crigths |= 4;
    } else if (*fen == 'q') {
      BRD->crigths |= 8;
    } else if (*fen >= 'A' && *fen <= 'H') {
      tmp = *fen - 'A';
      if (tmp > KING_W) {
        ROOK_W[0]    = tmp;
        BRD->crigths |= 1;
      } else if (tmp < KING_W) {
        ROOK_W[1]    = tmp;
        BRD->crigths |= 2;
      }
    } else if (*fen >= 'a' && *fen <= 'h') {
      tmp = *fen - 'a';
      if (tmp > X(KING_B)) {
        ROOK_B[0]    = 56 + tmp;
        BRD->crigths |= 4;
      } else if (tmp < X(KING_B)) {
        ROOK_B[1]    = 56 + tmp;
        BRD->crigths |= 8;
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
  int len, i, piece = 0;
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

static void Build_crigths_bitboards(void)
{
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

// https://en.wikipedia.org/wiki/Forsyth-Edwards_Notation
static void Fen_create(const char *fen)
{
  Fen_split(fen);
  if (FEN_STR[0][0] == '\0') return;
  Fen_board(FEN_STR[0]);
  if (FEN_STR[1][0] == '\0') return;
  WTM = FEN_STR[1][0] == 'w';
  if (FEN_STR[2][0] == '\0') return;
  Find_crigths_rooks_and_kings();
  Fen_KQkq(FEN_STR[2]);
  Build_crigths_bitboards();
  if (FEN_STR[3][0] == '\0') return;
  Fen_ep(FEN_STR[3]);
}

static void Assume_legal_position(void)
{
  MYASSERT((Popcount(BRD->white[5]) == 1 && Popcount(BRD->black[5]) == 1));
  if (WTM) MYASSERT(( ! Checks_w())); else MYASSERT(( ! Checks_b()));
  MYASSERT((BRD->board[Lsb(BRD->white[5])] == 6 && BRD->board[Lsb(BRD->black[5])] == -6));
  for (int i = 0; i < 64; i++) {
    if (BRD->board[i] > 0)
      MYASSERT(((BRD->board[i] <=  6) && (Bit(i) & BRD->white[ BRD->board[i] - 1])));
    else if (BRD->board[i] < 0)
      MYASSERT(((BRD->board[i] >= -6) && (Bit(i) & BRD->black[-BRD->board[i] - 1])));
    else
      MYASSERT(( ! (Bit(i) & BOTH())));
  }
  MYASSERT((BRD->epsq >= -1 && BRD->epsq <= 63));
  if (BRD->epsq != -1) {
    if (WTM) MYASSERT((Y(BRD->epsq) == 5)); else MYASSERT((Y(BRD->epsq) == 2));
  }
  if (BRD->crigths & 1) MYASSERT((X(ROOK_W[0]) >= X(KING_W) && BRD->board[ROOK_W[0]] ==  4));
  if (BRD->crigths & 2) MYASSERT((X(ROOK_W[1]) <= X(KING_W) && BRD->board[ROOK_W[1]] ==  4));
  if (BRD->crigths & 4) MYASSERT((X(ROOK_B[0]) >= X(KING_B) && BRD->board[ROOK_B[0]] == -4));
  if (BRD->crigths & 8) MYASSERT((X(ROOK_B[1]) <= X(KING_B) && BRD->board[ROOK_B[1]] == -4));
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
  Assume_legal_position();
}

// Checks


static inline bool Checks_here_w(const int pos)
{
  const U64 both = BOTH();
  return ((PAWN_CHECKS_B[pos]            & BRD->white[0])                   |
          (KNIGHT_MOVES[pos]             & BRD->white[1])                   |
          (Bishop_magic_moves(pos, both) & (BRD->white[2] | BRD->white[4])) |
          (Rook_magic_moves(pos, both)   & (BRD->white[3] | BRD->white[4])) |
          (KING_MOVES[pos]               & BRD->white[5]));
}

static inline bool Checks_here_b(const int pos)
{
  const U64 both = BOTH();
  return ((PAWN_CHECKS_W[pos]            & BRD->black[0])                   |
          (KNIGHT_MOVES[pos]             & BRD->black[1])                   |
          (Bishop_magic_moves(pos, both) & (BRD->black[2] | BRD->black[4])) |
          (Rook_magic_moves(pos, both)   & (BRD->black[3] | BRD->black[4])) |
          (KING_MOVES[pos]               & BRD->black[5]));
}

static bool Checks_crigths_w(U64 squares)
{
  while (squares) {
    if (Checks_here_w(Lsb(squares))) return 1;
    squares = Clear_bit(squares);
  }
  return 0;
}

static bool Checks_crigths_b(U64 squares)
{
  while (squares) {
    if (Checks_here_b(Lsb(squares))) return 1;
    squares = Clear_bit(squares);
  }
  return 0;
}

static bool Checks_w(void)
{
  return Checks_here_w(Lsb(BRD->black[5]));
}

static bool Checks_b(void)
{
  return Checks_here_b(Lsb(BRD->white[5]));
}

// Mgen

static inline U64 Bishop_magic_index(const int position, const U64 mask)
{
  return ((mask & BISHOP_MASK[position]) * BISHOP_MAGIC[position]) >> 55;
}

static inline U64 Rook_magic_index(const int position, const U64 mask)
{
  return ((mask & ROOK_MASK[position]) * ROOK_MAGIC[position]) >> 52;
}

static inline U64 Bishop_magic_moves(const int position, const U64 mask)
{
  return BISHOP_MAGIC_MOVES[position][Bishop_magic_index(position, mask)];
}

static inline U64 Rook_magic_moves(const int position, const U64 mask)
{
  return ROOK_MAGIC_MOVES[position][Rook_magic_index(position, mask)];
}

static void Add_crigths_OO_w(void)
{
  if (Checks_crigths_b(CASTLE_W[0])) return;
  MGEN_MOVES[MGEN_MOVES_N] = *BRD;
  BRD                    = &MGEN_MOVES[MGEN_MOVES_N];
  BRD->epsq              = -1;
  BRD->crigths           &= 4 | 8;
  BRD->board[ROOK_W[0]]  = 0;
  BRD->board[KING_W]     = 0;
  BRD->board[5]          = 4;
  BRD->board[6]          = 6;
  BRD->white[3]         ^= Bit(ROOK_W[0]);
  BRD->white[5]         ^= Bit(KING_W);
  BRD->white[3]         |= Bit(5);
  BRD->white[5]         |= Bit(6);
  if (Checks_b()) return;
  MGEN_MOVES_N++;
}

static void Add_crigths_OOO_w(void)
{
  if (Checks_crigths_b(CASTLE_W[1])) return;
  MGEN_MOVES[MGEN_MOVES_N] = *BRD;
  BRD                    = &MGEN_MOVES[MGEN_MOVES_N];
  BRD->epsq              = -1;
  BRD->crigths           &= 4 | 8;
  BRD->board[ROOK_W[1]]  = 0;
  BRD->board[KING_W]     = 0;
  BRD->board[3]          = 4;
  BRD->board[2]          = 6;
  BRD->white[3]         ^= Bit(ROOK_W[1]);
  BRD->white[5]         ^= Bit(KING_W);
  BRD->white[3]         |= Bit(3);
  BRD->white[5]         |= Bit(2);
  if (Checks_b()) return;
  MGEN_MOVES_N++;
}

static void Mgen_crigths_moves_w(void)
{
  if ((BRD->crigths & 1) && ! (CASTLE_EMPTY_W[0] & MGEN_BOTH)) {Add_crigths_OO_w();  BRD = BRD_ORIGINAL;}
  if ((BRD->crigths & 2) && ! (CASTLE_EMPTY_W[1] & MGEN_BOTH)) {Add_crigths_OOO_w(); BRD = BRD_ORIGINAL;}
}

static void Add_crigths_OO_b(void)
{
  if (Checks_crigths_w(CASTLE_B[0])) return;
  MGEN_MOVES[MGEN_MOVES_N] = *BRD;
  BRD = &MGEN_MOVES[MGEN_MOVES_N];
  BRD->epsq              = -1;
  BRD->crigths           &= 1 | 2;
  BRD->board[ROOK_B[0]]  = 0;
  BRD->board[KING_B]     = 0;
  BRD->board[56 + 5]     = -4;
  BRD->board[56 + 6]     = -6;
  BRD->black[3]         ^= Bit(ROOK_B[0]);
  BRD->black[5]         ^= Bit(KING_B);
  BRD->black[3]         |= Bit(56 + 5);
  BRD->black[5]         |= Bit(56 + 6);
  if (Checks_w()) return;
  MGEN_MOVES_N++;
}

static void Add_crigths_OOO_b(void)
{
  if (Checks_crigths_w(CASTLE_B[1])) return;
  MGEN_MOVES[MGEN_MOVES_N] = *BRD;
  BRD = &MGEN_MOVES[MGEN_MOVES_N];
  BRD->epsq              = -1;
  BRD->crigths           &= 1 | 2;
  BRD->board[ROOK_B[1]]  = 0;
  BRD->board[KING_B]     = 0;
  BRD->board[56 + 3]     = -4;
  BRD->board[56 + 2]     = -6;
  BRD->black[3]         ^= Bit(ROOK_B[1]);
  BRD->black[5]         ^= Bit(KING_B);
  BRD->black[3]         |= Bit(56 + 3);
  BRD->black[5]         |= Bit(56 + 2);
  if (Checks_w()) return;
  MGEN_MOVES_N++;
}

static void Mgen_crigths_moves_b(void)
{
  if ((BRD->crigths & 4) && ! (CASTLE_EMPTY_B[0] & MGEN_BOTH)) {Add_crigths_OO_b();  BRD = BRD_ORIGINAL;}
  if ((BRD->crigths & 8) && ! (CASTLE_EMPTY_B[1] & MGEN_BOTH)) {Add_crigths_OOO_b(); BRD = BRD_ORIGINAL;}
}

static void Check_crigths_rights_w(void)
{
  if (BRD->board[KING_W]    != 6) {BRD->crigths &= 4 | 8; return;}
  if (BRD->board[ROOK_W[0]] != 4)  BRD->crigths &= 2 | 4 | 8;
  if (BRD->board[ROOK_W[1]] != 4)  BRD->crigths &= 1 | 4 | 8;
}

static void Check_crigths_rights_b(void)
{
  if (BRD->board[KING_B]    != -6) {BRD->crigths &= 1 | 2; return;}
  if (BRD->board[ROOK_B[0]] != -4)  BRD->crigths &= 1 | 2 | 8;
  if (BRD->board[ROOK_B[1]] != -4)  BRD->crigths &= 1 | 2 | 4;
}

static void Handle_crigths_rights(void)
{
  if ( ! BRD->crigths) return;
  Check_crigths_rights_w();
  Check_crigths_rights_b();
}

static void Modify_pawn_stuff_w(const int from, const int to)
{
  if (to == BRD_ORIGINAL->epsq) {
    BRD->board[to - 8]  = 0;
    BRD->black[0]      ^= Bit(to - 8);
  } else if (Y(to) - Y(from) == 2) {
    BRD->epsq = to - 8;
  }
}

static void Add_promotion_w(const int from, const int to, const int piece)
{
  const int eat = BRD->board[to];
  MGEN_MOVES[MGEN_MOVES_N] = *BRD;
  BRD = &MGEN_MOVES[MGEN_MOVES_N];
  BRD->epsq         = -1;
  BRD->board[to]    = piece;
  BRD->board[from]  = 0;
  BRD->white[0]    ^= Bit(from);
  BRD->white[piece - 1] |= Bit(to);
  if (eat) BRD->black[-eat - 1] ^= Bit(to);
  if (Checks_b()) return;
  Handle_crigths_rights();
  MGEN_MOVES_N++;
}

static void Add_normal_stuff_w(const int from, const int to)
{
  const int me = BRD->board[from], eat = BRD->board[to];
  MGEN_MOVES[MGEN_MOVES_N] = *BRD;
  BRD = &MGEN_MOVES[MGEN_MOVES_N];
  BRD->epsq           = -1;
  BRD->board[from]    = 0;
  BRD->board[to]      = me;
  BRD->white[me - 1] ^= Bit(from);
  BRD->white[me - 1] |= Bit(to);
  if (eat)
    BRD->black[-eat - 1] ^= Bit(to);
  if (BRD->board[to] == 1) Modify_pawn_stuff_w(from, to);
  if (Checks_b()) return;
  Handle_crigths_rights();
  MGEN_MOVES_N++;
}

static void Add_promotion_stuff_w(const int from, const int to) // =qnrb
{
  BOARD_T *board = BRD;
  for (int piece = 2; piece <= 5; piece++) {Add_promotion_w(from, to, piece); BRD = board;}
}

static void Add_w(const int from, const int to)
{
  if (BRD->board[from] == 1 && Y(from) == 6)
    Add_promotion_stuff_w(from, to);
  else
    Add_normal_stuff_w(from, to);
}

static void Modify_pawn_stuff_b(const int from, const int to)
{
  if (to == BRD_ORIGINAL->epsq) {
    BRD->board[to + 8]  = 0;
    BRD->white[0]      ^= Bit(to + 8);
  } else if (Y(to) - Y(from) == -2) {
    BRD->epsq = to + 8;
  }
}

static void Add_normal_stuff_b(const int from, const int to)
{
  const int me = BRD->board[from], eat = BRD->board[to];

  MGEN_MOVES[MGEN_MOVES_N] = *BRD;
  BRD                  = &MGEN_MOVES[MGEN_MOVES_N];
  BRD->epsq            = -1;
  BRD->board[to]       = me;
  BRD->board[from]     = 0;
  BRD->black[-me - 1] ^= Bit(from);
  BRD->black[-me - 1] |= Bit(to);
  if (eat)
    BRD->white[eat - 1] ^= Bit(to);
  if (BRD->board[to] == -1) Modify_pawn_stuff_b(from, to);
  if (Checks_w()) return;
  Handle_crigths_rights();
  MGEN_MOVES_N++;
}

static void Add_promotion_b(const int from, const int to, const int piece)
{
  const int eat = BRD->board[to];
  MGEN_MOVES[MGEN_MOVES_N] = *BRD;
  BRD = &MGEN_MOVES[MGEN_MOVES_N];
  BRD->epsq         = -1;
  BRD->board[from]  = 0;
  BRD->board[to]    = piece;
  BRD->black[0]    ^= Bit(from);
  BRD->black[-piece - 1] |= Bit(to);
  if (eat) BRD->white[eat - 1] ^= Bit(to);
  if (Checks_w()) return;
  Handle_crigths_rights();
  MGEN_MOVES_N++;
}

static void Add_promotion_stuff_b(const int from, const int to)
{
  BOARD_T *board = BRD;
  for (int piece = 2; piece <= 5; piece++) {Add_promotion_b(from, to, -piece); BRD = board;}
}

static void Add_b(const int from, const int to)
{
  if (BRD->board[from] == -1 && Y(from) == 1)
    Add_promotion_stuff_b(from, to);
  else
    Add_normal_stuff_b(from, to);
}

static void Add_moves_w(const int from, U64 moves)
{
  while (moves) {
    Add_w(from, Lsb(moves));
    moves = Clear_bit(moves);
    BRD   = BRD_ORIGINAL;
  }
}

static void Add_moves_b(const int from, U64 moves)
{
  while (moves) {
    Add_b(from, Lsb(moves));
    moves = Clear_bit(moves);
    BRD   = BRD_ORIGINAL;
  }
}

static void Mgen_setup_w(void)
{
  MGEN_WHITE   = WHITE();
  MGEN_BLACK   = BLACK();
  MGEN_BOTH    = MGEN_WHITE | MGEN_BLACK;
  MGEN_EMPTY   = ~MGEN_BOTH;
  MGEN_PAWN_SQ = BRD->epsq > 0 ? (MGEN_BLACK | (Bit(BRD->epsq) & 0x0000FF0000000000ULL)) : MGEN_BLACK;
}

static void Mgen_setup_b(void)
{
  MGEN_WHITE   = WHITE();
  MGEN_BLACK   = BLACK();
  MGEN_BOTH    = MGEN_WHITE | MGEN_BLACK;
  MGEN_EMPTY   = ~MGEN_BOTH;
  MGEN_PAWN_SQ = BRD->epsq > 0 ? (MGEN_WHITE | (Bit(BRD->epsq) & 0x0000000000FF0000ULL)) : MGEN_WHITE;
}

#define POP() pos = Lsb(pieces); pieces = Clear_bit(pieces)

static void Mgen_pawns_w(void)
{
  int pos;
  U64 pieces = BRD->white[0];
  while (pieces) {
    POP();
    Add_moves_w(pos, PAWN_CHECKS_W[pos] & MGEN_PAWN_SQ);
    if (Y(pos) == 1) {
      if (PAWN_1_MOVES_W[pos] & MGEN_EMPTY) Add_moves_w(pos, PAWN_2_MOVES_W[pos] & MGEN_EMPTY);
    } else {
      Add_moves_w(pos, PAWN_1_MOVES_W[pos] & MGEN_EMPTY);
    }
  }
}

static void Mgen_pawns_b(void)
{
  int pos;
  U64 pieces = BRD->black[0];
  while (pieces) {
    POP();
    Add_moves_b(pos, PAWN_CHECKS_B[pos] & MGEN_PAWN_SQ);
    if (Y(pos) == 6) {
      if (PAWN_1_MOVES_B[pos] & MGEN_EMPTY) Add_moves_b(pos, PAWN_2_MOVES_B[pos] & MGEN_EMPTY);
    } else {
      Add_moves_b(pos, PAWN_1_MOVES_B[pos] & MGEN_EMPTY);
    }
  }
}

static void Mgen_knights_w(void)
{
  int pos;
  U64 pieces = BRD->white[1];
  while (pieces) {POP(); Add_moves_w(pos, KNIGHT_MOVES[pos] & MGEN_GOOD);}
}

static void Mgen_knights_b(void)
{
  int pos;
  U64 pieces = BRD->black[1];
  while (pieces) {POP(); Add_moves_b(pos, KNIGHT_MOVES[pos] & MGEN_GOOD);}
}

static void Mgen_bishops_plus_queens_w(void)
{
  int pos;
  U64 pieces = BRD->white[2] | BRD->white[4];
  while (pieces) {POP(); Add_moves_w(pos, Bishop_magic_moves(pos, MGEN_BOTH) & MGEN_GOOD);}
}

static void Mgen_bishops_plus_queens_b(void)
{
  int pos;
  U64 pieces = BRD->black[2] | BRD->black[4];
  while (pieces) {POP(); Add_moves_b(pos, Bishop_magic_moves(pos, MGEN_BOTH) & MGEN_GOOD);}
}

static void Mgen_rooks_plus_queens_w(void)
{
  int pos;
  U64 pieces = BRD->white[3] | BRD->white[4];
  while (pieces) {POP(); Add_moves_w(pos, Rook_magic_moves(pos, MGEN_BOTH) & MGEN_GOOD);}
}

static void Mgen_rooks_plus_queens_b(void)
{
  int pos;
  U64 pieces = BRD->black[3] | BRD->black[4];
  while (pieces) {POP(); Add_moves_b(pos, Rook_magic_moves(pos, MGEN_BOTH) & MGEN_GOOD);}
}

static void Mgen_king_w(void)
{
  const int pos = Lsb(BRD->white[5]);
  Add_moves_w(pos, KING_MOVES[pos] & MGEN_GOOD);
}

static void Mgen_king_b(void)
{
  const int pos = Lsb(BRD->black[5]);
  Add_moves_b(pos, KING_MOVES[pos] & MGEN_GOOD);
}

static void Mgen_all_w(void)
{
  Mgen_setup_w();
  MGEN_GOOD = ~MGEN_WHITE;
  Mgen_pawns_w();
  Mgen_knights_w();
  Mgen_bishops_plus_queens_w();
  Mgen_rooks_plus_queens_w();
  Mgen_king_w();
  Mgen_crigths_moves_w();
}

static void Mgen_all_b(void)
{
  Mgen_setup_b();
  MGEN_GOOD = ~MGEN_BLACK;
  Mgen_pawns_b();
  Mgen_knights_b();
  Mgen_bishops_plus_queens_b();
  Mgen_rooks_plus_queens_b();
  Mgen_king_b();
  Mgen_crigths_moves_b();
}

static inline void Swap(BOARD_T *board_a, BOARD_T *board_b)
{
  const BOARD_T tmp = *board_a;
  *board_a          = *board_b;
  *board_b          = tmp;
}

static int Mgen_w(BOARD_T *moves)
{
  MGEN_MOVES_N = 0;
  MGEN_MOVES   = moves;
  BRD_ORIGINAL = BRD;
  Mgen_all_w();
  return MGEN_MOVES_N;
}

static int Mgen_b(BOARD_T *moves)
{
  MGEN_MOVES_N = 0;
  MGEN_MOVES   = moves;
  BRD_ORIGINAL = BRD;
  Mgen_all_b();
  return MGEN_MOVES_N;
}

// Hash

static U64 Hash(const int wtm)
{
  int pos;
  U64 hash = ZOBRIST_EP[BRD->epsq + 1] ^ ZOBRIST_WTM[wtm] ^ ZOBRIST_CASTLE[BRD->crigths], both = BOTH();
  while (both) {
    pos  = Lsb(both);
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

// [1 MB, 1 PB]
static void Hashtable_set_size(const int usize)
{
  U64 size = ULL(usize);
  Hashtable_free_memory();
  size = MAX(size, 1);
  size = MIN(size, 1024 * 1024);
  size = MEGABYTE * size;
  MYHASH.size = 1;
  while (MYHASH.size <= size) MYHASH.size <<= 1;
  MYHASH.size  >>= 1;
  MYHASH.count   = INT(MYHASH.size / sizeof(HASH_ENTRY_T));
  MYHASH.key     = 1;
  while (MYHASH.key <= MYHASH.count) MYHASH.key <<= 1;
  MYHASH.key   >>= 1;
  MYHASH.key    -= 1; // 1000b = 8d / - 1d / 0111b = 7d
  MYHASH.array   = (HASH_ENTRY_T*) calloc(MYHASH.count, sizeof(HASH_ENTRY_T)); // <- Cast for g++
  MYASSERT(MYHASH.array != NULL);
}

// Perft

static U64 Get_perft(const U64 hash, const int depth)
{
  const HASH_ENTRY_T *entry = &MYHASH.array[(unsigned int)(hash & MYHASH.key)];
  return entry->hash == hash && entry->depth == depth ? entry->nodes : 0;
}

static void Add_perft(const U64 hash, const U64 nodes, const int depth)
{
  HASH_ENTRY_T *entry = &MYHASH.array[(unsigned int)(hash & MYHASH.key)];
  if ( ! nodes || (entry->hash == hash && entry->nodes > nodes)) return;
  entry->depth = depth;
  entry->hash  = hash;
  entry->nodes = nodes;
}

// Bulk counting + hashing
static U64 Perft_w(const int depth)
{
  int i, len;
  BOARD_T moves[MAX_MOVES];
  const U64 hash = Hash(1);
  U64 nodes      = Get_perft(hash, depth);
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

static U64 Perft_b(const int depth)
{
  int i, len;
  BOARD_T moves[MAX_MOVES];
  const U64 hash = Hash(0);
  U64 nodes      = Get_perft(hash, depth);
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

static U64 Perft(const int depth)
{
  Fen(POSITION_FEN);
  if (depth > 0) return WTM ? Perft_w(depth - 1) : Perft_b(depth - 1);
  return 1;
}

static void Padding(const char *str, const int space)
{
  const int len = space - strlen(str);
  for (int i = 0; i < len; i++) printf(" ");
  printf("%s", str);
}

// 561735852 -> 561,735,852
static const char *Big_number(U64 number)
{
  char str[256] = "";
  static char ret[256];
  int counter = 0, three = 2, i, len;
  for (i = 0; i < 64; i++) {if ( ! number) break; str[counter] = '0' + (number % 10); counter++; number /= 10; if ( ! three && number) {str[counter] = ','; counter++; three = 3;} three--; str[counter + 1] = '\0';}
  for (counter = 0, len = strlen(str), i = len - 1; i >= 0; i--) {ret[counter] = str[i]; counter++;}
  ret[counter] = '\0';
  return ret;
}

static void Perft_print(const int depth, const U64 nodes, const U64 ms)
{
  static char str[32];
  const char *big_num = Big_number(nodes);
  if (depth < 0) {printf("Total");} else {sprintf(str, "%i", depth); Padding(str, 5);}
  //sprintf(str, "%llu", nodes);
  Padding(big_num, 20);
  sprintf(str, "%.3f", 0.000001f * DOUBLE(Nps(nodes, ms))); Padding(str, 12);
  sprintf(str, "%.3f", 0.001f * DOUBLE(ms)); Padding(str, 12);
  printf("\n");
}

static void Perft_run(const int depth)
{
  int i;
  U64 nodes, start_time, diff_time, totaltime = 0, allnodes = 0;
  Print("[ %s ]", POSITION_FEN);
  Print("Depth               Nodes        Mnps        Time");
  for (i = 0; i < depth + 1; i++) {
    start_time = Now();
    nodes      = Perft(i);
    diff_time  = Now() - start_time;
    totaltime  += diff_time;
    allnodes   += nodes;
    Perft_print(i, nodes, diff_time);
  }

  Print("=================================================");
  Print("                    Nodes        Mnps        Time");
  Perft_print(-1, allnodes, totaltime);
}

static U64 Suite_run(const int depth)
{
  U64 start, nodes = 0, allnodes = 0;
  Print("Depth               Nodes        Mnps        Time");
  for (int i = 0; i <= depth; i++) {
    start = Now();
    nodes = Perft(i);
    allnodes += nodes;
    Perft_print(i, nodes, Now() - start);
  }
  return allnodes;
}

static void Bench(const bool fullsuite)
{
  U64 nodes = 0;
  const char fens[12][256] = {
    // Normal : https://www.chessprogramming.org/Perft_Results
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 ",
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ",
    "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ",
    "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
    "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8 ",
    "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10 ",
    // Chess960 : https://www.chessprogramming.org/Chess960_Perft_Results
    "bqnb1rkr/pp3ppp/3ppn2/2p5/5P2/P2P4/NPP1P1PP/BQ1BNRKR w HFhf - 2 9",
    "bnqbnr1r/p1p1ppkp/3p4/1p4p1/P7/3NP2P/1PPP1PP1/BNQB1RKR w HF - 0 9",
    "nrbq2kr/ppppppb1/5n1p/5Pp1/8/P5P1/1PPPP2P/NRBQNBKR w HBhb - 1 9",
    "1r1bkqbr/pppp1ppp/2nnp3/8/2P5/N4P2/PP1PP1PP/1RNBKQBR w Hh - 0 9",
    "rkqnbbnr/ppppppp1/8/7p/3N4/6PP/PPPPPP2/RKQNBB1R w HAa - 0 9",
    "rbqkr1bn/pp1ppp2/2p1n2p/6p1/8/4BPNP/PPPPP1P1/RBQKRN2 w EAea - 0 9"
  };
  U64 start = Now();
  for (int i = 0; i < 12; i++) {
    strcpy(POSITION_FEN, fens[i]);
    Print("%s[ #%i: %s ]", i ? "\n" : "", i + 1, fens[i]);
    nodes += Suite_run(fullsuite ? 6 : 5);
  }
  Print("\n=================================================\n");
  Print("                    Nodes        Mnps        Time");
  Perft_print(-1, nodes, Now() - start);
  MYASSERT((nodes == (fullsuite ? 21799671196 : 561735852)));
}

// Commands

static void Set_fen(void)
{
  Fen(STARTPOS);
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

static void Commands(void)
{
  if (TOKENS_N < 2) {
    Print_help();
    exit(EXIT_SUCCESS);
  }
  Token_expect(";");
  while (Token_ok()) {
    if (     Token_next("help"))    Print_help();
    else if (Token_next("version")) Print(NAME);
    else if (Token_next("fen"))     Set_fen();
    else if (Token_next("perft"))   Perft_run(Max(1, Token_next_int()));
    else if (Token_next("bench"))   Bench(Token_next_int());
    else if (Token_next("hash"))    Hashtable_set_size(Token_next_int());
    Token_expect(";");
  }
}

static void Print_help(void)
{
  Print("# %s", NAME);
  Print("%s\n", LICENSE);
  Print("Usage: lastemperor [COMMAND] [OPTION]? ...");
  Print("> lastemperor -hash 512 -perft 6 # Set 512 MB hash and run perft\n");
  Print("## LastEmperor Commands");
  Print("-help         This help");
  Print("-version      Show Version");
  Print("-hash N       Set hash in N MB");
  Print("-fen [FEN]    Set fen");
  Print("-perft [1..]  Run perft position");
  Print("-bench [01]   Benchmark (0 = normal, 1 = full)\n");
  Print("Full source code, please see: <https://github.com/SamuraiDangyo/LastEmperor/>");
  exit(EXIT_SUCCESS);
}

// Init

static U64 Permutate_bb(const U64 moves, const int index)
{
  int i, total = 0, good[64] = {0};
  const int popn = Popcount(moves);
  U64 permutations = 0;
  for (i = 0; i < 64; i++)
    if (moves & Bit(i)) {
      good[total] = i;
      total++;
    }
  for (i = 0; i < popn; i++)
    if ((1 << i) & index)
      permutations |= Bit(good[i]);
  return permutations & moves;
}

static U64 Make_slider_magic_moves(const int *slider_vectors, const int pos, const U64 moves)
{
  int j, k, x, y;
  U64 tmp, possible_moves = 0;
  const int x_pos = X(pos), y_pos = Y(pos);
  for (j = 0; j < 4; j++)
    for (k = 1; k < 8; k++) {
      x = x_pos + k * slider_vectors[2 * j];
      y = y_pos + k * slider_vectors[2 * j + 1];
      if ( ! On_board(x, y)) break;
      tmp             = Bit(8 * y + x);
      possible_moves |= tmp;
      if (tmp & moves) break;
    }
  return possible_moves & (~Bit(pos));
}

static void Init_bishop_magics(void)
{
  int i, j;
  U64 magics, allmoves;
  const int bishop_vectors[8] = {1,1,-1,-1,1,-1,-1,1};
  for (i = 0; i < 64; i++) {
    magics = BISHOP_MOVE_MAGICS[i] & (~Bit(i));
    for (j = 0; j < 512; j++) {
      allmoves = Permutate_bb(magics, j);
      BISHOP_MAGIC_MOVES[i][Bishop_magic_index(i, allmoves)] = Make_slider_magic_moves(bishop_vectors, i, allmoves);
    }
  }
}

static void Init_rook_magics(void)
{
  int i, j;
  U64 magics, allmoves;
  const int rook_vectors[8] = {1,0,0,1,0,-1,-1,0};
  for (i = 0; i < 64; i++) {
    magics = ROOK_MOVE_MAGICS[i] & (~Bit(i));
    for (j = 0; j < 4096; j++) {
      allmoves = Permutate_bb(magics, j);
      ROOK_MAGIC_MOVES[i][Rook_magic_index(i, allmoves)] = Make_slider_magic_moves(rook_vectors, i, allmoves);
    }
  }
}

static U64 Make_jump_moves(const int pos, const int len, const int dy, const int *jump_vectors)
{
  int i, x, y;
  U64 moves = 0;
  const int x_pos = X(pos), y_pos = Y(pos);
  for (i = 0; i < len; i++) {
    x = x_pos + jump_vectors[2 * i];
    y = y_pos + dy * jump_vectors[2 * i + 1];
    if (On_board(x, y)) moves |= Bit(8 * y + x);
  }
  return moves;
}

static void Init_jump_moves(void)
{
  int i;
  const int king_vectors[2 * 8]       = {1,0,0,1,0,-1,-1,0,1,1,-1,-1,1,-1,-1,1};
  const int knight_vectors[2 * 8]     = {2,1,-2,1,2,-1,-2,-1,1,2,-1,2,1,-2,-1,-2};
  const int pawn_check_vectors[2 * 2] = {-1,1,1,1};
  const int pawn_1_vectors[1 * 2]     = {0,1};
  for (i = 0; i < 64; i++) {
    KING_MOVES[i]     = Make_jump_moves(i, 8,  1, king_vectors);
    KNIGHT_MOVES[i]   = Make_jump_moves(i, 8,  1, knight_vectors);
    PAWN_CHECKS_W[i]  = Make_jump_moves(i, 2,  1, pawn_check_vectors);
    PAWN_CHECKS_B[i]  = Make_jump_moves(i, 2, -1, pawn_check_vectors);
    PAWN_1_MOVES_W[i] = Make_jump_moves(i, 1,  1, pawn_1_vectors);
    PAWN_1_MOVES_B[i] = Make_jump_moves(i, 1, -1, pawn_1_vectors);
  }
  for (i = 0; i < 8; i++) {
    PAWN_2_MOVES_W[ 8 + i] = Make_jump_moves( 8 + i, 1,  1, pawn_1_vectors) | Make_jump_moves( 8 + i, 1,  2, pawn_1_vectors);
    PAWN_2_MOVES_B[48 + i] = Make_jump_moves(48 + i, 1, -1, pawn_1_vectors) | Make_jump_moves(48 + i, 1, -2, pawn_1_vectors);
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

// Go

static void Init(void)
{
  RANDOM_SEED += ULL(time(NULL));
  Init_zobrist();
  Init_bishop_magics();
  Init_rook_magics();
  Init_jump_moves();
  Hashtable_set_size(256);
  Init_board();
}

static void Go(void)
{
  MYASSERT(((sizeof(int) >= 4) && (sizeof(U64) >= 8) && ((0x1122334455667788ULL >> 32)  == 0x11223344ULL) && ((0xFFFFFFFFFFFFFFFFULL & 0x8142241818244281ULL) == 0x8142241818244281ULL)));
  Init();
  Commands();
}

// "Si vis pacem, para bellum" -- Plato, The Laws of Plato
int main(int argc, char **argv)
{
  atexit(Hashtable_free_memory); // No memory leaks
  Init_tokens(argc, argv);
  Go();
  return EXIT_SUCCESS;
}
