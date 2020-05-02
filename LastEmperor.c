/*
LastEmperor, a Chess960 movegen tool (Derived from Sapeli 1.67)
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

#include <stdarg.h>
#include <string.h>
#include <sys/time.h>
#include <assert.h>
#include <iostream>

// Constants

#define NAME        "LastEmperor 1.10"
#define STARTPOS    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq"
#define MAX_MOVES   218
#define MAX_TOKENS  32
#define WHITE()     (BRD->white[0] | BRD->white[1] | BRD->white[2] | BRD->white[3] | BRD->white[4] | BRD->white[5])
#define BLACK()     (BRD->black[0] | BRD->black[1] | BRD->black[2] | BRD->black[3] | BRD->black[4] | BRD->black[5])
#define BOTH()      (WHITE() | BLACK())

// Macros

#define X(a)        ((a) & 7)
#define Y(a)        ((a) >> 3)
#define RESET(a)    memset((a), 0, sizeof((a)))

// Namespace

namespace LastEmperor {

// Structs

typedef struct {
  uint64_t white[6]; // White bitboards
  uint64_t black[6]; // Black bitboards
  char board[64];    // Pieces
  char epsq;         // En passant square
  uint8_t castle;    // Castling rights
} BOARD_T;

typedef struct {
  uint64_t hash, nodes;
  int depth;
} HASH_T;

// Prototypes

uint64_t BishopMagicMoves(const int, const uint64_t);
uint64_t RookMagicMoves(const int, const uint64_t);
bool ChecksW();
bool ChecksB();
uint64_t PerftB(const int);
void PrintHelp();

// Magics

const uint64_t ROOK_MAGIC[64] = {
  0x548001400080106cULL,0x900184000110820ULL,0x428004200a81080ULL,0x140088082000c40ULL,0x1480020800011400ULL,0x100008804085201ULL,0x2a40220001048140ULL,0x50000810000482aULL,
  0x250020100020a004ULL,0x3101880100900a00ULL,0x200a040a00082002ULL,0x1004300044032084ULL,0x2100408001013ULL,0x21f00440122083ULL,0xa204280406023040ULL,0x2241801020800041ULL,
  0xe10100800208004ULL,0x2010401410080ULL,0x181482000208805ULL,0x4080101000021c00ULL,0xa250210012080022ULL,0x4210641044000827ULL,0x8081a02300d4010ULL,0x8008012000410001ULL,
  0x28c0822120108100ULL,0x500160020aa005ULL,0xc11050088c1000ULL,0x48c00101000a288ULL,0x494a184408028200ULL,0x20880100240006ULL,0x10b4010200081ULL,0x40a200260000490cULL,
  0x22384003800050ULL,0x7102001a008010ULL,0x80020c8010900c0ULL,0x100204082a001060ULL,0x8000118188800428ULL,0x58e0020009140244ULL,0x100145040040188dULL,0x44120220400980ULL,
  0x114001007a00800ULL,0x80a0100516304000ULL,0x7200301488001000ULL,0x1000151040808018ULL,0x3000a200010e0020ULL,0x1000849180802810ULL,0x829100210208080ULL,0x1004050021528004ULL,
  0x61482000c41820b0ULL,0x241001018a401a4ULL,0x45020c009cc04040ULL,0x308210c020081200ULL,0xa000215040040ULL,0x10a6024001928700ULL,0x42c204800c804408ULL,0x30441a28614200ULL,
  0x40100229080420aULL,0x9801084000201103ULL,0x8408622090484202ULL,0x4022001048a0e2ULL,0x280120020049902ULL,0x1200412602009402ULL,0x914900048020884ULL,0x104824281002402ULL
};
const uint64_t ROOK_MASK[64] = {
  0x101010101017eULL,0x202020202027cULL,0x404040404047aULL,0x8080808080876ULL,0x1010101010106eULL,0x2020202020205eULL,0x4040404040403eULL,0x8080808080807eULL,
  0x1010101017e00ULL,0x2020202027c00ULL,0x4040404047a00ULL,0x8080808087600ULL,0x10101010106e00ULL,0x20202020205e00ULL,0x40404040403e00ULL,0x80808080807e00ULL,
  0x10101017e0100ULL,0x20202027c0200ULL,0x40404047a0400ULL,0x8080808760800ULL,0x101010106e1000ULL,0x202020205e2000ULL,0x404040403e4000ULL,0x808080807e8000ULL,
  0x101017e010100ULL,0x202027c020200ULL,0x404047a040400ULL,0x8080876080800ULL,0x1010106e101000ULL,0x2020205e202000ULL,0x4040403e404000ULL,0x8080807e808000ULL,
  0x1017e01010100ULL,0x2027c02020200ULL,0x4047a04040400ULL,0x8087608080800ULL,0x10106e10101000ULL,0x20205e20202000ULL,0x40403e40404000ULL,0x80807e80808000ULL,
  0x17e0101010100ULL,0x27c0202020200ULL,0x47a0404040400ULL,0x8760808080800ULL,0x106e1010101000ULL,0x205e2020202000ULL,0x403e4040404000ULL,0x807e8080808000ULL,
  0x7e010101010100ULL,0x7c020202020200ULL,0x7a040404040400ULL,0x76080808080800ULL,0x6e101010101000ULL,0x5e202020202000ULL,0x3e404040404000ULL,0x7e808080808000ULL,
  0x7e01010101010100ULL,0x7c02020202020200ULL,0x7a04040404040400ULL,0x7608080808080800ULL,0x6e10101010101000ULL,0x5e20202020202000ULL,0x3e40404040404000ULL,0x7e80808080808000ULL
};
const uint64_t ROOK_MOVE_MAGICS[64] = {
  0x101010101017eULL,0x202020202027cULL,0x404040404047aULL,0x8080808080876ULL,0x1010101010106eULL,0x2020202020205eULL,0x4040404040403eULL,0x8080808080807eULL,
  0x1010101017e00ULL,0x2020202027c00ULL,0x4040404047a00ULL,0x8080808087600ULL,0x10101010106e00ULL,0x20202020205e00ULL,0x40404040403e00ULL,0x80808080807e00ULL,
  0x10101017e0100ULL,0x20202027c0200ULL,0x40404047a0400ULL,0x8080808760800ULL,0x101010106e1000ULL,0x202020205e2000ULL,0x404040403e4000ULL,0x808080807e8000ULL,
  0x101017e010100ULL,0x202027c020200ULL,0x404047a040400ULL,0x8080876080800ULL,0x1010106e101000ULL,0x2020205e202000ULL,0x4040403e404000ULL,0x8080807e808000ULL,
  0x1017e01010100ULL,0x2027c02020200ULL,0x4047a04040400ULL,0x8087608080800ULL,0x10106e10101000ULL,0x20205e20202000ULL,0x40403e40404000ULL,0x80807e80808000ULL,
  0x17e0101010100ULL,0x27c0202020200ULL,0x47a0404040400ULL,0x8760808080800ULL,0x106e1010101000ULL,0x205e2020202000ULL,0x403e4040404000ULL,0x807e8080808000ULL,
  0x7e010101010100ULL,0x7c020202020200ULL,0x7a040404040400ULL,0x76080808080800ULL,0x6e101010101000ULL,0x5e202020202000ULL,0x3e404040404000ULL,0x7e808080808000ULL,
  0x7e01010101010100ULL,0x7c02020202020200ULL,0x7a04040404040400ULL,0x7608080808080800ULL,0x6e10101010101000ULL,0x5e20202020202000ULL,0x3e40404040404000ULL,0x7e80808080808000ULL
};
const uint64_t BISHOP_MAGIC[64] = {
  0x2890208600480830ULL,0x324148050f087ULL,0x1402488a86402004ULL,0xc2210a1100044bULL,0x88450040b021110cULL,0xc0407240011ULL,0xd0246940cc101681ULL,0x1022840c2e410060ULL,
  0x4a1804309028d00bULL,0x821880304a2c0ULL,0x134088090100280ULL,0x8102183814c0208ULL,0x518598604083202ULL,0x67104040408690ULL,0x1010040020d000ULL,0x600001028911902ULL,
  0x8810183800c504c4ULL,0x2628200121054640ULL,0x28003000102006ULL,0x4100c204842244ULL,0x1221c50102421430ULL,0x80109046e0844002ULL,0xc128600019010400ULL,0x812218030404c38ULL,
  0x1224152461091c00ULL,0x1c820008124000aULL,0xa004868015010400ULL,0x34c080004202040ULL,0x200100312100c001ULL,0x4030048118314100ULL,0x410000090018ULL,0x142c010480801ULL,
  0x8080841c1d004262ULL,0x81440f004060406ULL,0x400a090008202ULL,0x2204020084280080ULL,0xb820060400008028ULL,0x110041840112010ULL,0x8002080a1c84400ULL,0x212100111040204aULL,
  0x9412118200481012ULL,0x804105002001444cULL,0x103001280823000ULL,0x40088e028080300ULL,0x51020d8080246601ULL,0x4a0a100e0804502aULL,0x5042028328010ULL,0xe000808180020200ULL,
  0x1002020620608101ULL,0x1108300804090c00ULL,0x180404848840841ULL,0x100180040ac80040ULL,0x20840000c1424001ULL,0x82c00400108800ULL,0x28c0493811082aULL,0x214980910400080cULL,
  0x8d1a0210b0c000ULL,0x164c500ca0410cULL,0xc6040804283004ULL,0x14808001a040400ULL,0x180450800222a011ULL,0x600014600490202ULL,0x21040100d903ULL,0x10404821000420ULL
};
const uint64_t BISHOP_MASK[64] = {
  0x40201008040200ULL,0x402010080400ULL,0x4020100a00ULL,0x40221400ULL,0x2442800ULL,0x204085000ULL,0x20408102000ULL,0x2040810204000ULL,
  0x20100804020000ULL,0x40201008040000ULL,0x4020100a0000ULL,0x4022140000ULL,0x244280000ULL,0x20408500000ULL,0x2040810200000ULL,0x4081020400000ULL,
  0x10080402000200ULL,0x20100804000400ULL,0x4020100a000a00ULL,0x402214001400ULL,0x24428002800ULL,0x2040850005000ULL,0x4081020002000ULL,0x8102040004000ULL,
  0x8040200020400ULL,0x10080400040800ULL,0x20100a000a1000ULL,0x40221400142200ULL,0x2442800284400ULL,0x4085000500800ULL,0x8102000201000ULL,0x10204000402000ULL,
  0x4020002040800ULL,0x8040004081000ULL,0x100a000a102000ULL,0x22140014224000ULL,0x44280028440200ULL,0x8500050080400ULL,0x10200020100800ULL,0x20400040201000ULL,
  0x2000204081000ULL,0x4000408102000ULL,0xa000a10204000ULL,0x14001422400000ULL,0x28002844020000ULL,0x50005008040200ULL,0x20002010080400ULL,0x40004020100800ULL,
  0x20408102000ULL,0x40810204000ULL,0xa1020400000ULL,0x142240000000ULL,0x284402000000ULL,0x500804020000ULL,0x201008040200ULL,0x402010080400ULL,
  0x2040810204000ULL,0x4081020400000ULL,0xa102040000000ULL,0x14224000000000ULL,0x28440200000000ULL,0x50080402000000ULL,0x20100804020000ULL,0x40201008040200ULL
};
const uint64_t BISHOP_MOVE_MAGICS[64] = {
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

const BOARD_T BRD_EMPTY        = {{0},{0},{0},0,0};
BOARD_T BRD_2                  = {{0},{0},{0},0,0};
BOARD_T *BRD                   = &BRD_2;
BOARD_T *BRD_ORIGINAL          = 0;
bool WTM                       = 0;

// Move generator

BOARD_T *MGEN_MOVES            = 0;
int MGEN_MOVES_N               = 0;
int KING_W                     = 0;
int KING_B                     = 0;
int ROOK_W[2]                  = {0};
int ROOK_B[2]                  = {0};
uint64_t MGEN_WHITE            = 0;
uint64_t MGEN_BLACK            = 0;
uint64_t MGEN_BOTH             = 0;
uint64_t MGEN_EMPTY            = 0;
uint64_t MGEN_GOOD             = 0;
uint64_t MGEN_PAWN_SQ          = 0;
uint64_t CASTLE_W[2]           = {0};
uint64_t CASTLE_B[2]           = {0};
uint64_t CASTLE_EMPTY_W[2]     = {0};
uint64_t CASTLE_EMPTY_B[2]     = {0};
uint64_t KNIGHT_MOVES[64]      = {0};
uint64_t KING_MOVES[64]        = {0};
uint64_t PAWN_CHECKS_W[64]     = {0};
uint64_t PAWN_CHECKS_B[64]     = {0};
uint64_t PAWN_1_MOVES_W[64]    = {0};
uint64_t PAWN_1_MOVES_B[64]    = {0};
uint64_t PAWN_2_MOVES_W[64]    = {0};
uint64_t PAWN_2_MOVES_B[64]    = {0};
uint64_t BISHOP_MAGIC_MOVES[64][512] = {{0}};
uint64_t ROOK_MAGIC_MOVES[64][4096]  = {{0}};

// Zobrist

uint64_t ZOBRIST_BOARD[13][64] = {{0}};
uint64_t ZOBRIST_EP[64]        = {0};
uint64_t ZOBRIST_CASTLE[16]    = {0};
uint64_t ZOBRIST_WTM[2]        = {0};

// Hash

HASH_T *HASH                   = 0;
uint64_t HASH_SIZE             = 0;
int HASH_COUNT                 = 0;
int HASH_KEY                   = 0;

// Misc

uint64_t RANDOM_SEED           = 131783;
int TOKENS_N                   = 0;
int TOKENS_I                   = 0;
char POSITION_FEN[128]         = STARTPOS;
char TOKENS[MAX_TOKENS][128]   = {{0}};
char FEN_STR[4][128]           = {{0}};

// static void DebugTokens() {int i; Print("TOKENS ( %i ) :", TOKENS_N); for (i = 0; i < TOKENS_N; i++) Print("%i. %s", i, TOKENS[i]);}
// static void DebugLog(const char *str) {FILE *file = fopen("LastEmperor-log.txt", "a+"); fprintf(file, "%s\n:::\n", str); fclose(file);}

// Utils

uint64_t Nps(const uint64_t nodes, const uint64_t ms) {
  return ms ? (1000 * nodes) / ms : 0;
}

int Lsb(const uint64_t bb) {
  return __builtin_ctzll(bb);
}

int Popcount(const uint64_t bb) {
  return __builtin_popcountll(bb);
}

uint64_t ClearBit(const uint64_t bb) {
  return bb & (bb - 1);
}

uint64_t Bit(const int nbits) {
  return 0x1ULL << nbits;
}

void Print(const char *format, ...) {
  va_list va;
  va_start(va, format);
  vfprintf(stdout, format, va);
  va_end(va);
  fprintf(stdout, "\n");
  fflush(stdout);
}

uint64_t Now() {
  struct timeval tv;
  assert(gettimeofday(&tv, NULL) == 0);
  return (uint64_t) (1000 * tv.tv_sec + tv.tv_usec / 1000);
}

bool IsNumber(const char ch) {
  return ch >= '0' && ch <= '9';
}

uint64_t RandomBb() {
  static uint64_t a = 0x12311227ULL, b = 0x1931311ULL, c = 0x13138141ULL;
  a ^= b + c;
  b ^= b * c + 0x1717711ULL;
  c *= 3;
  c += 1;
#define MIXER(val) (((val) << 7) ^ ((val) >> 5))
  return MIXER(a) ^ MIXER(b) ^ MIXER(c);
}

uint64_t RandomUint64T() {
  int i;
  uint64_t ret = 0;
  for (i = 0; i < 8; i++) ret ^= RandomBb() << (8 * i);
  return ret;
}

bool OnBoard(const int x, const int y) {
  return x >= 0 && y >= 0 && x <= 7 && y <= 7;
}

// Token stuff

void TokenAdd(const char *token) {
  assert(TOKENS_N + 1 <= MAX_TOKENS);
  strcpy(TOKENS[TOKENS_N], token);
  TOKENS_N++;
}

bool TokenOk() {
  return TOKENS_I < TOKENS_N;
}

const char *TokenCurrent() {
  return TokenOk() ? TOKENS[TOKENS_I] : "\0";
}

void TokenPop() {
  TOKENS_I++;
}

bool TokenIs(const char *token) {
  return TokenOk() && ! strcmp(token, TokenCurrent());
}

bool Token(const char *token) {
  if ( ! TokenIs(token)) return 0;
  TokenPop();
  return 1;
}

int TokenInt() {
  int ret = 0;
  if (TokenOk() && TOKENS[TOKENS_I][0] != '-') {
    ret = atoi(TOKENS[TOKENS_I]);
    TokenPop();
  }
  return ret;
}

// Board stuff

void BuildBitboards() {
  RESET(BRD->white);
  RESET(BRD->black);
  for (int i = 0; i < 64; i++)
    if (BRD->board[i] > 0)      BRD->white[ BRD->board[i] - 1] |= Bit(i);
    else if (BRD->board[i] < 0) BRD->black[-BRD->board[i] - 1] |= Bit(i);
}

uint64_t Fill(int from, const int to) {
  uint64_t ret = Bit(from);
  const int diff = from > to ? -1 : 1;
  if (from < 0 || to < 0 || from > 63 || to > 63) return 0;
  if (from == to) return ret;
  while (from != to) {
    from += diff;
    ret  |= Bit(from);
  }
  return ret;
}

void FindCastleRooksAndKings() {
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

int Piece(const char piece) {
  for (int i = 0; i < 6; i++)
    if (     piece == "pnbrqk"[i]) return -i - 1;
    else if (piece == "PNBRQK"[i]) return  i + 1;
  return 0;
}

void FenBoard(const char *fen) {
  int pos = 56;
  while (*fen != '\0' && pos >= 0) {
    if (*fen == '/') {
      pos -= 16;
    } else if (IsNumber(*fen)) {
      pos += *fen - '0';
    } else {
      BRD->board[pos] = (char)Piece(*fen);
      pos++;
    }
    fen++;
  }
}

void FenKQkq(const char *fen) {
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

void FenEp(const char *fen) {
  if (*fen == '-' || *fen == '\0' || *(fen + 1) == '\0') return;
  BRD->epsq = *fen - 'a';
  fen++;
  BRD->epsq += 8 * (*fen - '1');
}

void FenSplit(const char *fen) {
  int len, i, piece = 0;
  for (i = 0; i < 4; i++) FEN_STR[i][0] = '\0';
  while (piece < 4) {
    while (*fen == ' ') fen++;
    if (*fen == '\0') return;
    len = 0;
    while (*fen != ' ' && *fen != '\0') {
      assert((len < 128));
      FEN_STR[piece][len] = *fen;
      len++;
      FEN_STR[piece][len] = '\0';
      fen++;
    }
    piece++;
  }
}

void BuildCastleBitboards() {
  CASTLE_W[0]       = Fill(KING_W, 6);
  CASTLE_W[1]       = Fill(KING_W, 2);
  CASTLE_B[0]       = Fill(KING_B, 56 + 6);
  CASTLE_B[1]       = Fill(KING_B, 56 + 2);
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

void FenCreate(const char *fen) {
  FenSplit(fen);
  if (FEN_STR[0][0] == '\0') return;
  FenBoard(FEN_STR[0]);
  if (FEN_STR[1][0] == '\0') return;
  WTM = FEN_STR[1][0] == 'w';
  if (FEN_STR[2][0] == '\0') return;
  FindCastleRooksAndKings();
  FenKQkq(FEN_STR[2]);
  BuildCastleBitboards();
  if (FEN_STR[3][0] == '\0') return;
  FenEp(FEN_STR[3]);
}

void AssumeLegalPosition() {
  assert((Popcount(BRD->white[5]) == 1 && Popcount(BRD->black[5]) == 1));
  if (WTM) assert(( ! ChecksW())); else assert(( ! ChecksB()));
  assert((BRD->board[Lsb(BRD->white[5])] == 6 && BRD->board[Lsb(BRD->black[5])] == -6));
  for (int i = 0; i < 64; i++) {
    if (BRD->board[i] > 0)
      assert(((BRD->board[i] <=  6) && (Bit(i) & BRD->white[ BRD->board[i] - 1])));
    else if (BRD->board[i] < 0)
      assert(((BRD->board[i] >= -6) && (Bit(i) & BRD->black[-BRD->board[i] - 1])));
    else
      assert(( ! (Bit(i) & BOTH())));
  }
  assert((BRD->epsq >= -1 && BRD->epsq <= 63));
  if (BRD->epsq != -1) {
    if (WTM) assert((Y(BRD->epsq) == 5)); else assert((Y(BRD->epsq) == 2));
  }
  if (BRD->castle & 1) assert((X(ROOK_W[0]) >= X(KING_W) && BRD->board[ROOK_W[0]] ==  4));
  if (BRD->castle & 2) assert((X(ROOK_W[1]) <= X(KING_W) && BRD->board[ROOK_W[1]] ==  4));
  if (BRD->castle & 4) assert((X(ROOK_B[0]) >= X(KING_B) && BRD->board[ROOK_B[0]] == -4));
  if (BRD->castle & 8) assert((X(ROOK_B[1]) <= X(KING_B) && BRD->board[ROOK_B[1]] == -4));
}

void Fen(const char *fen) {
  BRD_2     = BRD_EMPTY;
  BRD       = &BRD_2;
  WTM       = 1;
  BRD->epsq = -1;
  KING_W    = 0;
  KING_B    = 0;
  RESET(ROOK_W);
  RESET(ROOK_B);
  FenCreate(fen);
  BuildBitboards();
  AssumeLegalPosition();
}

// Checks

bool ChecksHereW(const int pos) {
  const uint64_t both = BOTH();
  return ((PAWN_CHECKS_B[pos]            & BRD->white[0])                 |
          (KNIGHT_MOVES[pos]             & BRD->white[1])                 |
          (BishopMagicMoves(pos, both) & (BRD->white[2] | BRD->white[4])) |
          (RookMagicMoves(pos, both)   & (BRD->white[3] | BRD->white[4])) |
          (KING_MOVES[pos]               & BRD->white[5]));
}

bool ChecksHereB(const int pos) {
  const uint64_t both = BOTH();
  return ((PAWN_CHECKS_W[pos]            & BRD->black[0])                 |
          (KNIGHT_MOVES[pos]             & BRD->black[1])                 |
          (BishopMagicMoves(pos, both) & (BRD->black[2] | BRD->black[4])) |
          (RookMagicMoves(pos, both)   & (BRD->black[3] | BRD->black[4])) |
          (KING_MOVES[pos]               & BRD->black[5]));
}

bool ChecksCastleW(uint64_t squares) {
  for (; squares; squares = ClearBit(squares)) if (ChecksHereW(Lsb(squares))) return 1;
  return 0;
}

bool ChecksCastleB(uint64_t squares) {
  for (; squares; squares = ClearBit(squares)) if (ChecksHereB(Lsb(squares))) return 1;
  return 0;
}

bool ChecksW() {
  return ChecksHereW(Lsb(BRD->black[5]));
}

bool ChecksB() {
  return ChecksHereB(Lsb(BRD->white[5]));
}

// Mgen

uint64_t BishopMagicIndex(const int position, const uint64_t mask) {
  return ((mask & BISHOP_MASK[position]) * BISHOP_MAGIC[position]) >> 55;
}

uint64_t RookMagicIndex(const int position, const uint64_t mask) {
  return ((mask & ROOK_MASK[position]) * ROOK_MAGIC[position]) >> 52;
}

uint64_t BishopMagicMoves(const int position, const uint64_t mask) {
  return BISHOP_MAGIC_MOVES[position][BishopMagicIndex(position, mask)];
}

uint64_t RookMagicMoves(const int position, const uint64_t mask) {
  return ROOK_MAGIC_MOVES[position][RookMagicIndex(position, mask)];
}

void AddCastleOOW() {
  if (ChecksCastleB(CASTLE_W[0])) return;
  MGEN_MOVES[MGEN_MOVES_N] = *BRD;
  BRD                    = &MGEN_MOVES[MGEN_MOVES_N];
  BRD->epsq              = -1;
  BRD->castle           &= 4 | 8;
  BRD->board[ROOK_W[0]]  = 0;
  BRD->board[KING_W]     = 0;
  BRD->board[5]          = 4;
  BRD->board[6]          = 6;
  BRD->white[3]         ^= Bit(ROOK_W[0]);
  BRD->white[5]         ^= Bit(KING_W);
  BRD->white[3]         |= Bit(5);
  BRD->white[5]         |= Bit(6);
  if (ChecksB()) return;
  MGEN_MOVES_N++;
}

void AddCastleOOOW() {
  if (ChecksCastleB(CASTLE_W[1])) return;
  MGEN_MOVES[MGEN_MOVES_N] = *BRD;
  BRD                    = &MGEN_MOVES[MGEN_MOVES_N];
  BRD->epsq              = -1;
  BRD->castle           &= 4 | 8;
  BRD->board[ROOK_W[1]]  = 0;
  BRD->board[KING_W]     = 0;
  BRD->board[3]          = 4;
  BRD->board[2]          = 6;
  BRD->white[3]         ^= Bit(ROOK_W[1]);
  BRD->white[5]         ^= Bit(KING_W);
  BRD->white[3]         |= Bit(3);
  BRD->white[5]         |= Bit(2);
  if (ChecksB()) return;
  MGEN_MOVES_N++;
}

void MgenCastleMovesW() {
  if ((BRD->castle & 1) && ! (CASTLE_EMPTY_W[0] & MGEN_BOTH)) {AddCastleOOW();  BRD = BRD_ORIGINAL;}
  if ((BRD->castle & 2) && ! (CASTLE_EMPTY_W[1] & MGEN_BOTH)) {AddCastleOOOW(); BRD = BRD_ORIGINAL;}
}

void AddCastleOOB() {
  if (ChecksCastleW(CASTLE_B[0])) return;
  MGEN_MOVES[MGEN_MOVES_N] = *BRD;
  BRD = &MGEN_MOVES[MGEN_MOVES_N];
  BRD->epsq              = -1;
  BRD->castle           &= 1 | 2;
  BRD->board[ROOK_B[0]]  = 0;
  BRD->board[KING_B]     = 0;
  BRD->board[56 + 5]     = -4;
  BRD->board[56 + 6]     = -6;
  BRD->black[3]         ^= Bit(ROOK_B[0]);
  BRD->black[5]         ^= Bit(KING_B);
  BRD->black[3]         |= Bit(56 + 5);
  BRD->black[5]         |= Bit(56 + 6);
  if (ChecksW()) return;
  MGEN_MOVES_N++;
}

void AddCastleOOOB() {
  if (ChecksCastleW(CASTLE_B[1])) return;
  MGEN_MOVES[MGEN_MOVES_N] = *BRD;
  BRD = &MGEN_MOVES[MGEN_MOVES_N];
  BRD->epsq              = -1;
  BRD->castle           &= 1 | 2;
  BRD->board[ROOK_B[1]]  = 0;
  BRD->board[KING_B]     = 0;
  BRD->board[56 + 3]     = -4;
  BRD->board[56 + 2]     = -6;
  BRD->black[3]         ^= Bit(ROOK_B[1]);
  BRD->black[5]         ^= Bit(KING_B);
  BRD->black[3]         |= Bit(56 + 3);
  BRD->black[5]         |= Bit(56 + 2);
  if (ChecksW()) return;
  MGEN_MOVES_N++;
}

void MgenCastleMovesB() {
  if ((BRD->castle & 4) && ! (CASTLE_EMPTY_B[0] & MGEN_BOTH)) {AddCastleOOB();  BRD = BRD_ORIGINAL;}
  if ((BRD->castle & 8) && ! (CASTLE_EMPTY_B[1] & MGEN_BOTH)) {AddCastleOOOB(); BRD = BRD_ORIGINAL;}
}

void CheckCastleRightsW() {
  if (BRD->board[KING_W]    != 6) {BRD->castle &= 4 | 8; return;}
  if (BRD->board[ROOK_W[0]] != 4)  BRD->castle &= 2 | 4 | 8;
  if (BRD->board[ROOK_W[1]] != 4)  BRD->castle &= 1 | 4 | 8;
}

void CheckCastleRightsB() {
  if (BRD->board[KING_B]    != -6) {BRD->castle &= 1 | 2; return;}
  if (BRD->board[ROOK_B[0]] != -4)  BRD->castle &= 1 | 2 | 8;
  if (BRD->board[ROOK_B[1]] != -4)  BRD->castle &= 1 | 2 | 4;
}

void HandleCastleRights() {
  if ( ! BRD->castle) return;
  CheckCastleRightsW();
  CheckCastleRightsB();
}

void ModifyPawnStuffW(const int from, const int to) {
  if (to == BRD_ORIGINAL->epsq) {
    BRD->board[to - 8]  = 0;
    BRD->black[0]      ^= Bit(to - 8);
  } else if (Y(to) - Y(from) == 2) {
    BRD->epsq = (char)(to - 8);
  }
}

void AddPromotionW(const int from, const int to, const int piece) {
  const int eat = BRD->board[to];
  MGEN_MOVES[MGEN_MOVES_N] = *BRD;
  BRD = &MGEN_MOVES[MGEN_MOVES_N];
  BRD->epsq         = -1;
  BRD->board[to]    = piece;
  BRD->board[from]  = 0;
  BRD->white[0]    ^= Bit(from);
  BRD->white[piece - 1] |= Bit(to);
  if (eat) BRD->black[-eat - 1] ^= Bit(to);
  if (ChecksB()) return;
  HandleCastleRights();
  MGEN_MOVES_N++;
}

void AddNormalStuffW(const int from, const int to) {
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
  if (BRD->board[to] == 1) ModifyPawnStuffW(from, to);
  if (ChecksB()) return;
  HandleCastleRights();
  MGEN_MOVES_N++;
}

void AddPromotionStuffW(const int from, const int to) // =qnrb
{
  BOARD_T *board = BRD;
  for (int piece = 2; piece <= 5; piece++) {AddPromotionW(from, to, piece); BRD = board;}
}

void AddW(const int from, const int to) {
  if (BRD->board[from] == 1 && Y(from) == 6)
    AddPromotionStuffW(from, to);
  else
    AddNormalStuffW(from, to);
}

void ModifyPawnStuffB(const int from, const int to) {
  if (to == BRD_ORIGINAL->epsq) {
    BRD->board[to + 8]  = 0;
    BRD->white[0]      ^= Bit(to + 8);
  } else if (Y(to) - Y(from) == -2) {
    BRD->epsq = to + 8;
  }
}

void AddNormalStuffB(const int from, const int to) {
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
  if (BRD->board[to] == -1) ModifyPawnStuffB(from, to);
  if (ChecksW()) return;
  HandleCastleRights();
  MGEN_MOVES_N++;
}

void AddPromotionB(const int from, const int to, const int piece) {
  const int eat = BRD->board[to];
  MGEN_MOVES[MGEN_MOVES_N] = *BRD;
  BRD = &MGEN_MOVES[MGEN_MOVES_N];
  BRD->epsq         = -1;
  BRD->board[from]  = 0;
  BRD->board[to]    = piece;
  BRD->black[0]    ^= Bit(from);
  BRD->black[-piece - 1] |= Bit(to);
  if (eat) BRD->white[eat - 1] ^= Bit(to);
  if (ChecksW()) return;
  HandleCastleRights();
  MGEN_MOVES_N++;
}

void AddPromotionStuffB(const int from, const int to) {
  BOARD_T *board = BRD;
  for (int piece = 2; piece <= 5; piece++) {AddPromotionB(from, to, -piece); BRD = board;}
}

void AddB(const int from, const int to) {
  if (BRD->board[from] == -1 && Y(from) == 1)
    AddPromotionStuffB(from, to);
  else
    AddNormalStuffB(from, to);
}

void AddMovesW(const int from, uint64_t moves) {
  for (; moves; moves = ClearBit(moves)) {
    AddW(from, Lsb(moves));
    BRD = BRD_ORIGINAL;
  }
}

void AddMovesB(const int from, uint64_t moves) {
  for (; moves; moves = ClearBit(moves)) {
    AddB(from, Lsb(moves));
    BRD = BRD_ORIGINAL;
  }
}

void MgenSetupW() {
  MGEN_WHITE   = WHITE();
  MGEN_BLACK   = BLACK();
  MGEN_BOTH    = MGEN_WHITE | MGEN_BLACK;
  MGEN_EMPTY   = ~MGEN_BOTH;
  MGEN_PAWN_SQ = BRD->epsq > 0 ? (MGEN_BLACK | (Bit(BRD->epsq) & 0x0000FF0000000000ULL)) : MGEN_BLACK;
}

void MgenSetupB() {
  MGEN_WHITE   = WHITE();
  MGEN_BLACK   = BLACK();
  MGEN_BOTH    = MGEN_WHITE | MGEN_BLACK;
  MGEN_EMPTY   = ~MGEN_BOTH;
  MGEN_PAWN_SQ = BRD->epsq > 0 ? (MGEN_WHITE | (Bit(BRD->epsq) & 0x0000000000FF0000ULL)) : MGEN_WHITE;
}

#define FORLOOP(mypieces) for (uint64_t pieces = mypieces; pieces; pieces = ClearBit(pieces))

void MgenPawnsW() {
  FORLOOP(BRD->white[0]) {
    const int pos = Lsb(pieces);
    AddMovesW(pos, PAWN_CHECKS_W[pos] & MGEN_PAWN_SQ);
    if (Y(pos) == 1) {
      if (PAWN_1_MOVES_W[pos] & MGEN_EMPTY) AddMovesW(pos, PAWN_2_MOVES_W[pos] & MGEN_EMPTY);
    } else {
      AddMovesW(pos, PAWN_1_MOVES_W[pos] & MGEN_EMPTY);
    }
  }
}

void MgenPawnsB() {
  FORLOOP(BRD->black[0]) {
    const int pos = Lsb(pieces);
    AddMovesB(pos, PAWN_CHECKS_B[pos] & MGEN_PAWN_SQ);
    if (Y(pos) == 6) {
      if (PAWN_1_MOVES_B[pos] & MGEN_EMPTY) AddMovesB(pos, PAWN_2_MOVES_B[pos] & MGEN_EMPTY);
    } else {
      AddMovesB(pos, PAWN_1_MOVES_B[pos] & MGEN_EMPTY);
    }
  }
}

void MgenKnightsW() {
  FORLOOP(BRD->white[1]) {const int pos = Lsb(pieces); AddMovesW(pos, KNIGHT_MOVES[pos] & MGEN_GOOD);}
}

void MgenKnightsB() {
  FORLOOP(BRD->black[1]) {const int pos = Lsb(pieces); AddMovesB(pos, KNIGHT_MOVES[pos] & MGEN_GOOD);}
}

void MgenBishopsPlusQueensW() {
  FORLOOP(BRD->white[2] | BRD->white[4]) {const int pos = Lsb(pieces); AddMovesW(pos, BishopMagicMoves(pos, MGEN_BOTH) & MGEN_GOOD);}
}

void MgenBishopsPlusQueensB() {
  FORLOOP(BRD->black[2] | BRD->black[4]) {const int pos = Lsb(pieces); AddMovesB(pos, BishopMagicMoves(pos, MGEN_BOTH) & MGEN_GOOD);}
}

void MgenRooksPlusQueensW() {
  FORLOOP(BRD->white[3] | BRD->white[4]) {const int pos = Lsb(pieces); AddMovesW(pos, RookMagicMoves(pos, MGEN_BOTH) & MGEN_GOOD);}
}

void MgenRooksPlusQueensB() {
  FORLOOP(BRD->black[3] | BRD->black[4]) {const int pos = Lsb(pieces); AddMovesB(pos, RookMagicMoves(pos, MGEN_BOTH) & MGEN_GOOD);}
}

void MgenKingW() {
  const int pos = Lsb(BRD->white[5]);
  AddMovesW(pos, KING_MOVES[pos] & MGEN_GOOD);
}

void MgenKingB() {
  const int pos = Lsb(BRD->black[5]);
  AddMovesB(pos, KING_MOVES[pos] & MGEN_GOOD);
}

void MgenAllW() {
  MgenSetupW();
  MGEN_GOOD = ~MGEN_WHITE;
  MgenPawnsW();
  MgenKnightsW();
  MgenBishopsPlusQueensW();
  MgenRooksPlusQueensW();
  MgenKingW();
  MgenCastleMovesW();
}

void MgenAllB() {
  MgenSetupB();
  MGEN_GOOD = ~MGEN_BLACK;
  MgenPawnsB();
  MgenKnightsB();
  MgenBishopsPlusQueensB();
  MgenRooksPlusQueensB();
  MgenKingB();
  MgenCastleMovesB();
}

int MgenW(BOARD_T *moves) {
  MGEN_MOVES_N = 0;
  MGEN_MOVES   = moves;
  BRD_ORIGINAL = BRD;
  MgenAllW();
  return MGEN_MOVES_N;
}

int MgenB(BOARD_T *moves) {
  MGEN_MOVES_N = 0;
  MGEN_MOVES   = moves;
  BRD_ORIGINAL = BRD;
  MgenAllB();
  return MGEN_MOVES_N;
}

// Hash

uint64_t Hash(const int wtm) {
  uint64_t hash = ZOBRIST_EP[BRD->epsq + 1] ^ ZOBRIST_WTM[wtm] ^ ZOBRIST_CASTLE[BRD->castle];
  for (uint64_t both = BOTH(); both; both &= both - 1) {
    const int pos = Lsb(both);
    hash ^= ZOBRIST_BOARD[BRD->board[pos] + 6][pos];
  }
  return hash;
}

void HashtableFreeMemory() {
  if (!HASH_SIZE) return;
  free(HASH);
  HASH      = 0;
  HASH_SIZE = 0;
}

void HashtableSetSize(const int usize) {
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
  assert(HASH != NULL);
}

// Perft

uint64_t GetPerft(const uint64_t hash, const int depth) {
  const HASH_T *entry = &HASH[(uint32_t)(hash & (uint64_t) HASH_KEY)];
  return entry->hash == hash && entry->depth == depth ? entry->nodes : 0;
}

void AddPerft(const uint64_t hash, const uint64_t nodes, const int depth) {
  HASH_T *entry = &HASH[(uint32_t)(hash & (uint64_t) HASH_KEY)];
  if ( ! nodes || (entry->hash == hash && entry->nodes > nodes)) return;
  entry->depth = depth;
  entry->hash  = hash;
  entry->nodes = nodes;
}

// Bulk counting + hashing
uint64_t PerftW(const int depth) {
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

uint64_t PerftB(const int depth) {
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

uint64_t Perft(const int depth) {
  Fen(POSITION_FEN);
  if (depth > 0) return WTM ? PerftW(depth - 1) : PerftB(depth - 1);
  return 1;
}

// 561735852 -> 561,735,852
const char *BigNumber(uint64_t number) {
  char str[256] = "";
  static char ret[256];
  int counter = 0, three = 2, i, len;
  if ( ! number) {ret[0] = '0'; ret[1] = '\0'; return ret;}
  for (i = 0; i < 100; i++) {if ( ! number) break; str[counter] = '0' + (number % 10); counter++; number /= 10; if ( ! three && number) {str[counter] = ','; counter++; three = 3;} three--; str[counter + 1] = '\0';}
  for (counter = 0, len = (int) strlen(str), i = len - 1; i >= 0; i--) {ret[counter] = str[i]; counter++;}
  ret[counter] = '\0';
  return ret;
}

void PerftPrint(const int depth, const uint64_t nodes, const uint64_t ms) {
  const char *big_num = BigNumber(nodes);
  if (depth < 0) printf("Total"); else printf("%5i", depth);
  printf("%20s %11.3f %11.3f\n", big_num, 0.000001 * (double) Nps(nodes, ms), 0.001 * (double) ms);
}

void PerftRun(const int depth) {
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

uint64_t SuiteRun(const int depth) {
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

void Bench(const bool fullsuite) {
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
  assert((nodes == (fullsuite ? 21799671196 : 561735852)));
  exit(EXIT_SUCCESS);
}

// Commands

void SetFen() {
  Fen(STARTPOS);
  POSITION_FEN[0] = '\0';
  strcpy(POSITION_FEN, TokenCurrent());
  TokenPop();
  Fen(POSITION_FEN);
}

void Commands() {
  if ( ! TOKENS_N) {PrintHelp(); return;}
  while (TokenOk()) {
    if (     Token("-help"))    PrintHelp();
    else if (Token("-version")) Print(NAME);
    else if (Token("-fen"))     SetFen();
    else if (Token("-perft"))   PerftRun(std::max(1, TokenInt()));
    else if (Token("-bench"))   Bench(TokenInt());
    else if (Token("-hash"))    HashtableSetSize(TokenInt());
    else TokenPop();
  }
}

void PrintHelp() {
  Print("# %s\nGNU General Public License version 3; for details see 'LICENSE'\n", NAME);
  Print("Usage: lastemperor [COMMAND] [OPTION]? ...");
  Print("> lastemperor -hash 512 -perft 6 # Set 512 MB hash and run perft\n");
  Print("## LastEmperor Commands");
  Print("-help         This help");
  Print("-version      Show Version");
  Print("-hash [N]     Set hash in N MB");
  Print("-fen [FEN]    Set fen");
  Print("-perft [1..]  Run perft position");
  Print("-bench [01]   Benchmark (0 = normal, 1 = full)\n");
  Print("Full source code here: <https://github.com/SamuraiDangyo/LastEmperor/>");
  exit(EXIT_SUCCESS);
}

// Init

uint64_t PermutateBb(const uint64_t moves, const int index) {
  int i, total = 0, good[64] = {0};
  const int popn = Popcount(moves);
  uint64_t permutations = 0;
  for (i = 0; i < 64; i++)
    if (moves & Bit(i)) {
      good[total] = i;
      total++;
    }
  for (i = 0; i < popn; i++) if ((1 << i) & index) permutations |= Bit(good[i]);
  return permutations & moves;
}

uint64_t MakeSliderMagicMoves(const int *slider_vectors, const int pos, const uint64_t moves) {
  int i, j, x, y;
  uint64_t tmp, possible_moves = 0;
  const int x_pos = X(pos), y_pos = Y(pos);
  for (i = 0; i < 4; i++)
    for (j = 1; j < 8; j++) {
      x = x_pos + j * slider_vectors[2 * i];
      y = y_pos + j * slider_vectors[2 * i + 1];
      if ( ! OnBoard(x, y)) break;
      tmp             = Bit(8 * y + x);
      possible_moves |= tmp;
      if (tmp & moves) break;
    }
  return possible_moves & (~Bit(pos));
}

void InitBishopMagics() {
  const int bishop_vectors[8] = {1,1,-1,-1,1,-1,-1,1};
  for (int i = 0; i < 64; i++) {
    const uint64_t magics = BISHOP_MOVE_MAGICS[i] & (~Bit(i));
    for (int j = 0; j < 512; j++) {
      const uint64_t allmoves = PermutateBb(magics, j);
      BISHOP_MAGIC_MOVES[i][BishopMagicIndex(i, allmoves)] = MakeSliderMagicMoves(bishop_vectors, i, allmoves);
    }
  }
}

void InitRookMagics() {
  int i, j;
  const int rook_vectors[8] = {1,0,0,1,0,-1,-1,0};
  for (i = 0; i < 64; i++) {
    const uint64_t magics = ROOK_MOVE_MAGICS[i] & (~Bit(i));
    for (j = 0; j < 4096; j++) {
      const uint64_t allmoves = PermutateBb(magics, j);
      ROOK_MAGIC_MOVES[i][RookMagicIndex(i, allmoves)] = MakeSliderMagicMoves(rook_vectors, i, allmoves);
    }
  }
}

uint64_t MakeJumpMoves(const int pos, const int len, const int dy, const int *jump_vectors) {
  uint64_t moves = 0;
  const int x_pos = X(pos), y_pos = Y(pos);
  for (int i = 0; i < len; i++) {
    int x = x_pos + jump_vectors[2 * i];
    int y = y_pos + dy * jump_vectors[2 * i + 1];
    if (OnBoard(x, y)) moves |= Bit(8 * y + x);
  }
  return moves;
}

void InitJumpMoves() {
  const int king_vectors[2 * 8] = {1,0,0,1,0,-1,-1,0,1,1,-1,-1,1,-1,-1,1}, knight_vectors[2 * 8] = {2,1,-2,1,2,-1,-2,-1,1,2,-1,2,1,-2,-1,-2},
            pawn_check_vectors[2 * 2] = {-1,1,1,1}, pawn_1_vectors[1 * 2] = {0,1};
  for (int i = 0; i < 64; i++) {
    KING_MOVES[i]     = MakeJumpMoves(i, 8,  1, king_vectors);
    KNIGHT_MOVES[i]   = MakeJumpMoves(i, 8,  1, knight_vectors);
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

void InitZobrist() {
  int i, j;
  for (i = 0; i < 13; i++) for (j = 0; j < 64; j++) ZOBRIST_BOARD[i][j] = RandomUint64T();
  for (i = 0; i < 64; i++) ZOBRIST_EP[i]     = RandomUint64T();
  for (i = 0; i < 16; i++) ZOBRIST_CASTLE[i] = RandomUint64T();
  for (i = 0; i <  2; i++) ZOBRIST_WTM[i]    = RandomUint64T();
}

void Init() {
  RANDOM_SEED += (uint64_t) time(NULL);
  InitZobrist();
  InitBishopMagics();
  InitRookMagics();
  InitJumpMoves();
  HashtableSetSize(256);
  Fen(STARTPOS);
}}

// "Si vis pacem, para bellum" -- Plato, The Laws of Plato
int main(int argc, char **argv) {
  for (int i = 1; i < argc; i++) LastEmperor::TokenAdd(argv[i]);
  atexit(LastEmperor::HashtableFreeMemory);
  LastEmperor::Init();
  LastEmperor::Commands();
  return EXIT_SUCCESS;
}
