/*
LastEmperor, a Chess960 program
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

// Headers (TODO: get rid off C crap ...)

#include <iostream>
#include <vector>
#include <iomanip>
#include <string.h>
#include <sys/time.h>
#include <immintrin.h>

// Namespace

namespace lastemperor {

// Struct declarations

struct Board {uint64_t white[6], black[6]; char board[64], epsq; uint8_t from, to, castle; void reset();};
struct MyHash {uint64_t hash, nodes; int depth; MyHash();};

// Struct definitions

void Board::reset() {epsq = from = to = castle = 0; memset(board, 0, sizeof(board)); memset(white, 0, sizeof(white)); memset(black, 0, sizeof(black));}
MyHash::MyHash() {hash = nodes = depth = 0;}

// Constexpr

constexpr char
  k_name[] = "LastEmperor 1.09", k_startpos[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - -";

constexpr int
  k_max_moves = 218, k_rook_vectors[8] = {1,0,0,1,0,-1,-1,0}, k_bishop_vectors[8] = {1,1,-1,-1,1,-1,-1,1}, k_king_vectors[2 * 8] = {1,0,0,1,0,-1,-1,0,1,1,-1,-1,1,-1,-1,1},
  k_knight_vectors[2 * 8] = {2,1,-2,1,2,-1,-2,-1,1,2,-1,2,1,-2,-1,-2};

constexpr uint64_t
  k_rook_mask[64] =
    {0x101010101017eULL,0x202020202027cULL,0x404040404047aULL,0x8080808080876ULL,0x1010101010106eULL,0x2020202020205eULL,0x4040404040403eULL,0x8080808080807eULL,
     0x1010101017e00ULL,0x2020202027c00ULL,0x4040404047a00ULL,0x8080808087600ULL,0x10101010106e00ULL,0x20202020205e00ULL,0x40404040403e00ULL,0x80808080807e00ULL,
     0x10101017e0100ULL,0x20202027c0200ULL,0x40404047a0400ULL,0x8080808760800ULL,0x101010106e1000ULL,0x202020205e2000ULL,0x404040403e4000ULL,0x808080807e8000ULL,
     0x101017e010100ULL,0x202027c020200ULL,0x404047a040400ULL,0x8080876080800ULL,0x1010106e101000ULL,0x2020205e202000ULL,0x4040403e404000ULL,0x8080807e808000ULL,
     0x1017e01010100ULL,0x2027c02020200ULL,0x4047a04040400ULL,0x8087608080800ULL,0x10106e10101000ULL,0x20205e20202000ULL,0x40403e40404000ULL,0x80807e80808000ULL,
     0x17e0101010100ULL,0x27c0202020200ULL,0x47a0404040400ULL,0x8760808080800ULL,0x106e1010101000ULL,0x205e2020202000ULL,0x403e4040404000ULL,0x807e8080808000ULL,
     0x7e010101010100ULL,0x7c020202020200ULL,0x7a040404040400ULL,0x76080808080800ULL,0x6e101010101000ULL,0x5e202020202000ULL,0x3e404040404000ULL,0x7e808080808000ULL,
     0x7e01010101010100ULL,0x7c02020202020200ULL,0x7a04040404040400ULL,0x7608080808080800ULL,0x6e10101010101000ULL,0x5e20202020202000ULL,0x3e40404040404000ULL,0x7e80808080808000ULL},
  k_rook_move_magics[64] =
    {0x101010101017eULL,0x202020202027cULL,0x404040404047aULL,0x8080808080876ULL,0x1010101010106eULL,0x2020202020205eULL,0x4040404040403eULL,0x8080808080807eULL,
     0x1010101017e00ULL,0x2020202027c00ULL,0x4040404047a00ULL,0x8080808087600ULL,0x10101010106e00ULL,0x20202020205e00ULL,0x40404040403e00ULL,0x80808080807e00ULL,
     0x10101017e0100ULL,0x20202027c0200ULL,0x40404047a0400ULL,0x8080808760800ULL,0x101010106e1000ULL,0x202020205e2000ULL,0x404040403e4000ULL,0x808080807e8000ULL,
     0x101017e010100ULL,0x202027c020200ULL,0x404047a040400ULL,0x8080876080800ULL,0x1010106e101000ULL,0x2020205e202000ULL,0x4040403e404000ULL,0x8080807e808000ULL,
     0x1017e01010100ULL,0x2027c02020200ULL,0x4047a04040400ULL,0x8087608080800ULL,0x10106e10101000ULL,0x20205e20202000ULL,0x40403e40404000ULL,0x80807e80808000ULL,
     0x17e0101010100ULL,0x27c0202020200ULL,0x47a0404040400ULL,0x8760808080800ULL,0x106e1010101000ULL,0x205e2020202000ULL,0x403e4040404000ULL,0x807e8080808000ULL,
     0x7e010101010100ULL,0x7c020202020200ULL,0x7a040404040400ULL,0x76080808080800ULL,0x6e101010101000ULL,0x5e202020202000ULL,0x3e404040404000ULL,0x7e808080808000ULL,
     0x7e01010101010100ULL,0x7c02020202020200ULL,0x7a04040404040400ULL,0x7608080808080800ULL,0x6e10101010101000ULL,0x5e20202020202000ULL,0x3e40404040404000ULL,0x7e80808080808000ULL},
  k_bishop_mask[64] =
    {0x40201008040200ULL,0x402010080400ULL,0x4020100a00ULL,0x40221400ULL,0x2442800ULL,0x204085000ULL,0x20408102000ULL,0x2040810204000ULL,
     0x20100804020000ULL,0x40201008040000ULL,0x4020100a0000ULL,0x4022140000ULL,0x244280000ULL,0x20408500000ULL,0x2040810200000ULL,0x4081020400000ULL,
     0x10080402000200ULL,0x20100804000400ULL,0x4020100a000a00ULL,0x402214001400ULL,0x24428002800ULL,0x2040850005000ULL,0x4081020002000ULL,0x8102040004000ULL,
     0x8040200020400ULL,0x10080400040800ULL,0x20100a000a1000ULL,0x40221400142200ULL,0x2442800284400ULL,0x4085000500800ULL,0x8102000201000ULL,0x10204000402000ULL,
     0x4020002040800ULL,0x8040004081000ULL,0x100a000a102000ULL,0x22140014224000ULL,0x44280028440200ULL,0x8500050080400ULL,0x10200020100800ULL,0x20400040201000ULL,
     0x2000204081000ULL,0x4000408102000ULL,0xa000a10204000ULL,0x14001422400000ULL,0x28002844020000ULL,0x50005008040200ULL,0x20002010080400ULL,0x40004020100800ULL,
     0x20408102000ULL,0x40810204000ULL,0xa1020400000ULL,0x142240000000ULL,0x284402000000ULL,0x500804020000ULL,0x201008040200ULL,0x402010080400ULL,
     0x2040810204000ULL,0x4081020400000ULL,0xa102040000000ULL,0x14224000000000ULL,0x28440200000000ULL,0x50080402000000ULL,0x20100804020000ULL,0x40201008040200ULL},
  k_bishop_move_magics[64] =
    {0x40201008040200ULL,0x402010080400ULL,0x4020100a00ULL,0x40221400ULL,0x2442800ULL,0x204085000ULL,0x20408102000ULL,0x2040810204000ULL,
     0x20100804020000ULL,0x40201008040000ULL,0x4020100a0000ULL,0x4022140000ULL,0x244280000ULL,0x20408500000ULL,0x2040810200000ULL,0x4081020400000ULL,
     0x10080402000200ULL,0x20100804000400ULL,0x4020100a000a00ULL,0x402214001400ULL,0x24428002800ULL,0x2040850005000ULL,0x4081020002000ULL,0x8102040004000ULL,
     0x8040200020400ULL,0x10080400040800ULL,0x20100a000a1000ULL,0x40221400142200ULL,0x2442800284400ULL,0x4085000500800ULL,0x8102000201000ULL,0x10204000402000ULL,
     0x4020002040800ULL,0x8040004081000ULL,0x100a000a102000ULL,0x22140014224000ULL,0x44280028440200ULL,0x8500050080400ULL,0x10200020100800ULL,0x20400040201000ULL,
     0x2000204081000ULL,0x4000408102000ULL,0xa000a10204000ULL,0x14001422400000ULL,0x28002844020000ULL,0x50005008040200ULL,0x20002010080400ULL,0x40004020100800ULL,
     0x20408102000ULL,0x40810204000ULL,0xa1020400000ULL,0x142240000000ULL,0x284402000000ULL,0x500804020000ULL,0x201008040200ULL,0x402010080400ULL,
     0x2040810204000ULL,0x4081020400000ULL,0xa102040000000ULL,0x14224000000000ULL,0x28440200000000ULL,0x50080402000000ULL,0x20100804020000ULL,0x40201008040200ULL};

// Variables

uint64_t
  m_black = 0, m_both = 0, m_empty = 0, m_good = 0, m_pawn_sq = 0, m_white = 0, m_pawn_1_moves_w[64] = {0}, m_pawn_1_moves_b[64] = {0}, m_pawn_2_moves_w[64] = {0}, m_pawn_2_moves_b[64] = {0},
  z_ep[64]= {0}, z_castle[16] = {0}, z_wtm[2] = {0}, z_board[13][64] = {{0}}, m_castle_w[2] = {0}, m_castle_b[2] = {0}, m_castle_empty_w[2] = {0}, m_castle_empty_b[2] = {0}, m_bishop_moves[64] = {0}, m_rook_moves[64] = {0},
  m_queen_moves[64] = {0}, m_knight_moves[64] = {0}, m_king_moves[64] = {0}, m_pawn_checks_w[64] = {0}, m_pawn_checks_b[64] = {0}, m_bishop_magic_moves[64][512] = {{0}}, m_rook_magic_moves[64][4096] = {{0}}, g_seed = 131783,
  h_hash_key = 1, h_hashsize = 0;

int
  m_king_w = 0, m_king_b = 0, m_moves_n = 0, m_rook_w[2] = {0}, m_rook_b[2] = {0};

MyHash
  *h_myhash = 0;

Board
  m_board_tmp, *m_board = &m_board_tmp, *m_moves = 0, *m_board_original = 0;

bool
  m_wtm = 0;

std::string
  g_position = "";

// Prototypes

uint64_t PerftB(const int);
uint64_t RookMagicMoves(const int, const uint64_t);
uint64_t BishopMagicMoves(const int, const uint64_t);

// Utils

template <class Type> Type Between(const Type val_a, const Type val_b, const Type val_c) {return std::max(val_a, std::min(val_b, val_c));}
inline uint8_t Xcoord(const uint64_t bb) {return bb & 7;}
inline uint8_t Ycoord(const uint64_t bb) {return bb >> 3;}
uint64_t Nps(const uint64_t nodes, const uint64_t ms) {return (1000 * nodes) / (ms + 1);}
inline uint64_t ClearBit(const uint64_t bb) {return bb & (bb - 1);}
inline uint64_t Bit(const int nbits) {return 0x1ULL << nbits;}
bool OnBoard(const int x, const int y) {return x >= 0 && x <= 7 && y >= 0 && y <= 7;}
uint64_t White() {return m_board->white[0] | m_board->white[1] | m_board->white[2] | m_board->white[3] | m_board->white[4] | m_board->white[5];}
uint64_t Black() {return m_board->black[0] | m_board->black[1] | m_board->black[2] | m_board->black[3] | m_board->black[4] | m_board->black[5];}
uint64_t Both() {return White() | Black();}

inline int Lsb(const uint64_t bb) {
#if defined MODERN
  return __builtin_ctzll(bb);
#else
  for (int i = 0; i < 64; i++) {if (bb & Bit(i)) return i;} return 0;
#endif
}

inline int PopCount(const uint64_t bb) {
#if defined MODERN
  return __builtin_popcountll(bb);
#else
  int popn = 0; for (uint64_t b64 = bb; b64; b64 = ClearBit(b64)) {popn++;} return popn;
#endif
}

template <class Type> void Splitter(const std::string& str, Type& container, const std::string& delims = " ") {
  std::size_t current, previous = 0;
  current = str.find_first_of(delims);
  while (current != std::string::npos) {
    container.push_back(str.substr(previous, current - previous));
    previous = current + 1;
    current  = str.find_first_of(delims, previous);
  }
  container.push_back(str.substr(previous, current - previous));
}

void Assert(const bool test, const std::string msg) {
  if (test) return;
  std::cerr << msg << std::endl;
  exit(EXIT_FAILURE);
}

uint64_t Now() {
  struct timeval tv;
  if (gettimeofday(&tv, NULL)) return 0;
  return (uint64_t) (1000 * tv.tv_sec + tv.tv_usec / 1000);
}

uint64_t RandomBB() { // Deterministic
  static uint64_t va = 0X12311227ULL, vb = 0X1931311ULL, vc = 0X13138141ULL;
  va ^= vb + vc;
  vb ^= vb * vc + 0x1717711ULL;
  vc  = (3 * vc) + 1;
  const auto mixer = [](const uint64_t val) {return ((val) << 7) ^ ((val) >> 5);};
  return mixer(va) ^ mixer(vb) ^ mixer(vc);
}

uint64_t Random8x64() {
  uint64_t val = 0;
  for (int i = 0; i < 8; i++) val ^= RandomBB() << (8 * i);
  return val;
}

// Hash

inline uint64_t Hash(const int wtm) {
  uint64_t hash = z_ep[m_board->epsq + 1] ^ z_wtm[wtm] ^ z_castle[m_board->castle], both = Both();
  for (; both; both = ClearBit(both)) {const int sq = Lsb(both); hash ^= z_board[m_board->board[sq] + 6][sq];}
  return hash;
}

void HashtableFreeMemory() {
  if (!h_myhash) return;
  delete[] h_myhash;
  h_myhash = 0;
}

void HashtableSetSize(const int usize) {
  HashtableFreeMemory();
  uint64_t hashsize = (1 << 20) * Between<uint64_t>(1, (uint64_t) usize, 1024 * 1024);
  uint32_t hash_count = 1;
  for (const uint32_t max_count = (uint32_t) (hashsize / sizeof(MyHash)); hash_count < max_count; hash_count <<= 1);
  hash_count >>= 1;
  h_hash_key = hash_count - 1;
  h_myhash = new MyHash[hash_count];
}

uint64_t GetPerft(const uint64_t hash, const uint8_t depth) {
  const MyHash *entry = &h_myhash[(uint32_t) (hash & h_hash_key)];
  return (entry->hash == hash && entry->depth == depth) ? entry->nodes : 0;
}

void AddPerft(const uint64_t hash, const uint64_t nodes, const uint8_t depth) {
  MyHash *entry = &h_myhash[(uint32_t) (hash & h_hash_key)];
  if (!nodes || (entry->hash == hash && entry->nodes > nodes)) return;
  entry->hash  = hash;
  entry->depth = depth;
  entry->nodes = nodes;
}

// Board

void BuildBitboards() {
  memset(m_board->white, 0, sizeof(m_board->white));
  memset(m_board->black, 0, sizeof(m_board->black));
  for (auto i = 0; i < 64; i++)
    if (     m_board->board[i] > 0) m_board->white[ m_board->board[i] - 1] |= Bit(i);
    else if (m_board->board[i] < 0) m_board->black[-m_board->board[i] - 1] |= Bit(i);
}

uint64_t Fill(int from, const int to) {
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

void FindKings() {
  for (auto i = 0; i < 64; i++)
    if (     m_board->board[i] ==  6) m_king_w = i;
    else if (m_board->board[i] == -6) m_king_b = i;
}

void FindRank1Rank8Rooks() {
  for (auto i = m_king_w + 1; i <  8; i++) if (m_board->board[i] == 4) m_rook_w[0] = i;
  for (auto i = m_king_w - 1; i > -1; i--) if (m_board->board[i] == 4) m_rook_w[1] = i;
  for (auto i = m_king_b + 1; i < 64;         i++) if (m_board->board[i] == -4) m_rook_b[0] = i;
  for (auto i = m_king_b - 1; i > 64 - 8 - 1; i--) if (m_board->board[i] == -4) m_rook_b[1] = i;
}

void FindCastlingRooksAndKings() {
  m_king_w = m_king_b = 0;
  FindKings();
  memset(m_rook_w, 0, sizeof(m_rook_w));
  memset(m_rook_b, 0, sizeof(m_rook_b));
  FindRank1Rank8Rooks();
}

void BuildCastlingBitboards() {
  m_castle_w[0]       = Fill(m_king_w, 6);
  m_castle_w[1]       = Fill(m_king_w, 2);
  m_castle_b[0]       = Fill(m_king_b, 56 + 6);
  m_castle_b[1]       = Fill(m_king_b, 56 + 2);
  m_castle_empty_w[0] = (m_castle_w[0] | Fill(m_rook_w[0], 5     )) ^ (Bit(m_king_w) | Bit(m_rook_w[0]));
  m_castle_empty_b[0] = (m_castle_b[0] | Fill(m_rook_b[0], 56 + 5)) ^ (Bit(m_king_b) | Bit(m_rook_b[0]));
  m_castle_empty_w[1] = (m_castle_w[1] | Fill(m_rook_w[1], 3     )) ^ (Bit(m_king_w) | Bit(m_rook_w[1]));
  m_castle_empty_b[1] = (m_castle_b[1] | Fill(m_rook_b[1], 56 + 3)) ^ (Bit(m_king_b) | Bit(m_rook_b[1]));
  for (auto i = 0; i < 2; i++) {
    m_castle_empty_w[i] &= 0xFFULL;
    m_castle_w[i]       &= 0xFFULL;
    m_castle_empty_b[i] &= 0xFF00000000000000ULL;
    m_castle_b[i]       &= 0xFF00000000000000ULL;
  }
}

int Piece(const char piece) {
  for (auto i = 0; i < 6; i++)
    if (     piece == "pnbrqk"[i]) return -i - 1;
    else if (piece == "PNBRQK"[i]) return  i + 1;
  return 0;
}

void FenBoard(const std::string fen) {
  int pos = 56;
  for (unsigned int i = 0; i < fen.length() && pos >= 0; i++) if (fen[i] == '/') pos -= 16; else if (isdigit(fen[i])) pos += fen[i] - '0'; else m_board->board[pos++] = Piece(fen[i]);
}

void FenKQkq(const std::string fen) {
  for (size_t i = 0; i < fen.length(); i++)
    if (fen[i] == 'K') {m_board->castle |= 1;}
    else if (fen[i] == 'Q') {m_board->castle |= 2;}
    else if (fen[i] == 'k') {m_board->castle |= 4;}
    else if (fen[i] == 'q') {m_board->castle |= 8;}
    else if (fen[i] >= 'A' && fen[i] <= 'H') {
      const int tmp = fen[i] - 'A';
      if (tmp > m_king_w) {m_rook_w[0] = tmp; m_board->castle |= 1;} else if (tmp < m_king_w) {m_rook_w[1] = tmp; m_board->castle |= 2;}
    } else if (fen[i] >= 'a' && fen[i] <= 'h') {
      const int tmp = fen[i] - 'a';
      if (tmp > Xcoord(m_king_b)) {m_rook_b[0] = 56 + tmp; m_board->castle |= 4;} else if (tmp < Xcoord(m_king_b)) {m_rook_b[1] = 56 + tmp; m_board->castle |= 8;}
    }
}

void FenEp(const std::string fen) {
  if (fen.length() != 2) return;
  m_board->epsq = (fen[0] - 'a') + 8 * (fen[1] - '1');
}

void FenGen(const std::string fen) {
  std::vector<std::string> tokens = {};
  Splitter<std::vector<std::string>>(fen, tokens, " ");
  Assert(tokens.size() >= 4, "Error #1: Bad fen");
  FenBoard(tokens[0]);
  m_wtm = tokens[1][0] == 'w';
  FindCastlingRooksAndKings();
  FenKQkq(tokens[2]);
  BuildCastlingBitboards();
  FenEp(tokens[3]);
}

void FenReset() {
  m_board_tmp.reset();
  m_board     = &m_board_tmp;
  m_wtm       = 1;
  m_board->epsq = -1;
  m_king_w = m_king_b = 0;
  memset(m_rook_w, 0, sizeof(m_rook_w));
  memset(m_rook_b, 0, sizeof(m_rook_b));
}

void Fen(const std::string fen) {
  g_position = fen;
  FenReset();
  FenGen(fen);
  BuildBitboards();
  Assert(PopCount(m_board->white[5]) == 1 && PopCount(m_board->black[5]) == 1, "Error #2: Bad board");
}

// Checks

bool ChecksHereW(const int square) {
  const uint64_t both = Both();
  return ((m_pawn_checks_b[square] & m_board->white[0]) | (m_knight_moves[square] & m_board->white[1]) | (BishopMagicMoves(square, both) & (m_board->white[2] | m_board->white[4])) |
          (RookMagicMoves(square, both) & (m_board->white[3] | m_board->white[4])) | (m_king_moves[square] & m_board->white[5]));
}

bool ChecksHereB(const int square) {
  const uint64_t both = Both();
  return ((m_pawn_checks_w[square] & m_board->black[0]) | (m_knight_moves[square] & m_board->black[1]) | (BishopMagicMoves(square, both) & (m_board->black[2] | m_board->black[4])) |
          (RookMagicMoves(square, both) & (m_board->black[3] | m_board->black[4])) | (m_king_moves[square] & m_board->black[5]));
}

bool ChecksCastleW(uint64_t squares) {for (; squares; squares = ClearBit(squares)) {if (ChecksHereW(Lsb(squares))) return 1;} return 0;}
bool ChecksCastleB(uint64_t squares) {for (; squares; squares = ClearBit(squares)) {if (ChecksHereB(Lsb(squares))) return 1;} return 0;}
inline bool ChecksW() {return ChecksHereW(Lsb(m_board->black[5]));}
inline bool ChecksB() {return ChecksHereB(Lsb(m_board->white[5]));}

// Move generator

#if defined PEXT
uint32_t Pext(const uint64_t occupied, const uint64_t mask) {return _pext_u64(occupied, mask);}
uint64_t BishopMagicMoves(const int square, const uint64_t occupied) {return m_bishop_magic_moves[square][Pext(occupied, k_bishop_mask[square])];}
uint64_t RookMagicMoves(const int square, const uint64_t occupied) {return m_rook_magic_moves[  square][Pext(occupied, k_rook_mask[square])];}
#else
const uint64_t
  k_rook_magic[64] = {
    0x548001400080106cULL,0x900184000110820ULL,0x428004200a81080ULL,0x140088082000c40ULL,0x1480020800011400ULL,0x100008804085201ULL,0x2a40220001048140ULL,0x50000810000482aULL,
    0x250020100020a004ULL,0x3101880100900a00ULL,0x200a040a00082002ULL,0x1004300044032084ULL,0x2100408001013ULL,0x21f00440122083ULL,0xa204280406023040ULL,0x2241801020800041ULL,
    0xe10100800208004ULL,0x2010401410080ULL,0x181482000208805ULL,0x4080101000021c00ULL,0xa250210012080022ULL,0x4210641044000827ULL,0x8081a02300d4010ULL,0x8008012000410001ULL,
    0x28c0822120108100ULL,0x500160020aa005ULL,0xc11050088c1000ULL,0x48c00101000a288ULL,0x494a184408028200ULL,0x20880100240006ULL,0x10b4010200081ULL,0x40a200260000490cULL,
    0x22384003800050ULL,0x7102001a008010ULL,0x80020c8010900c0ULL,0x100204082a001060ULL,0x8000118188800428ULL,0x58e0020009140244ULL,0x100145040040188dULL,0x44120220400980ULL,
    0x114001007a00800ULL,0x80a0100516304000ULL,0x7200301488001000ULL,0x1000151040808018ULL,0x3000a200010e0020ULL,0x1000849180802810ULL,0x829100210208080ULL,0x1004050021528004ULL,
    0x61482000c41820b0ULL,0x241001018a401a4ULL,0x45020c009cc04040ULL,0x308210c020081200ULL,0xa000215040040ULL,0x10a6024001928700ULL,0x42c204800c804408ULL,0x30441a28614200ULL,
    0x40100229080420aULL,0x9801084000201103ULL,0x8408622090484202ULL,0x4022001048a0e2ULL,0x280120020049902ULL,0x1200412602009402ULL,0x914900048020884ULL,0x104824281002402ULL},
  k_bishop_magic[64] = {
    0x2890208600480830ULL,0x324148050f087ULL,0x1402488a86402004ULL,0xc2210a1100044bULL,0x88450040b021110cULL,0xc0407240011ULL,0xd0246940cc101681ULL,0x1022840c2e410060ULL,
    0x4a1804309028d00bULL,0x821880304a2c0ULL,0x134088090100280ULL,0x8102183814c0208ULL,0x518598604083202ULL,0x67104040408690ULL,0x1010040020d000ULL,0x600001028911902ULL,
    0x8810183800c504c4ULL,0x2628200121054640ULL,0x28003000102006ULL,0x4100c204842244ULL,0x1221c50102421430ULL,0x80109046e0844002ULL,0xc128600019010400ULL,0x812218030404c38ULL,
    0x1224152461091c00ULL,0x1c820008124000aULL,0xa004868015010400ULL,0x34c080004202040ULL,0x200100312100c001ULL,0x4030048118314100ULL,0x410000090018ULL,0x142c010480801ULL,
    0x8080841c1d004262ULL,0x81440f004060406ULL,0x400a090008202ULL,0x2204020084280080ULL,0xb820060400008028ULL,0x110041840112010ULL,0x8002080a1c84400ULL,0x212100111040204aULL,
    0x9412118200481012ULL,0x804105002001444cULL,0x103001280823000ULL,0x40088e028080300ULL,0x51020d8080246601ULL,0x4a0a100e0804502aULL,0x5042028328010ULL,0xe000808180020200ULL,
    0x1002020620608101ULL,0x1108300804090c00ULL,0x180404848840841ULL,0x100180040ac80040ULL,0x20840000c1424001ULL,0x82c00400108800ULL,0x28c0493811082aULL,0x214980910400080cULL,
    0x8d1a0210b0c000ULL,0x164c500ca0410cULL,0xc6040804283004ULL,0x14808001a040400ULL,0x180450800222a011ULL,0x600014600490202ULL,0x21040100d903ULL,0x10404821000420ULL};
uint64_t BishopMagicIndex(const int square, const uint64_t mask) {return ((mask & k_bishop_mask[square]) * k_bishop_magic[square]) >> 55;}
uint64_t RookMagicIndex(const int square, const uint64_t mask) {return ((mask & k_rook_mask[square]) * k_rook_magic[square]) >> 52;}
uint64_t BishopMagicMoves(const int square, const uint64_t mask) {return m_bishop_magic_moves[square][BishopMagicIndex(square, mask)];}
uint64_t RookMagicMoves(const int square, const uint64_t mask) {return m_rook_magic_moves[square][RookMagicIndex(square, mask)];}
#endif

void HandleCastlingW(const int from, const int to) {
  m_moves[m_moves_n] = *m_board;
  m_board = &m_moves[m_moves_n];
  m_board->epsq = -1;
  m_board->from = from;
  m_board->to = to;
  m_board->castle &= 4 | 8;
}

void AddCastleOOW() {
  if (ChecksCastleB(m_castle_w[0])) return;
  HandleCastlingW(m_king_w, 6);
  m_board->board[m_rook_w[0]] = 0;
  m_board->board[m_king_w] = 0;
  m_board->board[5] = 4;
  m_board->board[6] = 6;
  m_board->white[3] = (m_board->white[3] ^ Bit(m_rook_w[0])) | Bit(5);
  m_board->white[5] = (m_board->white[5] ^ Bit(m_king_w))    | Bit(6);
  if (ChecksB()) return;
  m_moves_n++;
}

void AddCastleOOOW() {
  if (ChecksCastleB(m_castle_w[1])) return;
  HandleCastlingW(m_king_w, 2);
  m_board->board[m_rook_w[1]] = 0;
  m_board->board[m_king_w] = 0;
  m_board->board[3] = 4;
  m_board->board[2] = 6;
  m_board->white[3] = (m_board->white[3] ^ Bit(m_rook_w[1])) | Bit(3);
  m_board->white[5] = (m_board->white[5] ^ Bit(m_king_w))    | Bit(2);
  if (ChecksB()) return;
  m_moves_n++;
}

void MgenCastlingMovesW() {
  if ((m_board->castle & 1) && !(m_castle_empty_w[0] & m_both)) {AddCastleOOW();  m_board = m_board_original;}
  if ((m_board->castle & 2) && !(m_castle_empty_w[1] & m_both)) {AddCastleOOOW(); m_board = m_board_original;}
}

void HandleCastlingB(const int from, const int to) {
  m_moves[m_moves_n] = *m_board;
  m_board = &m_moves[m_moves_n];
  m_board->epsq = -1;
  m_board->from = from;
  m_board->to = to;
  m_board->castle &= 1 | 2;
}

void AddCastleOOB() {
  if (ChecksCastleW(m_castle_b[0])) return;
  HandleCastlingB(m_king_b, 56 + 6);
  m_board->board[m_rook_b[0]] = 0;
  m_board->board[m_king_b] = 0;
  m_board->board[56 + 5] = -4;
  m_board->board[56 + 6] = -6;
  m_board->black[3] = (m_board->black[3] ^ Bit(m_rook_b[0])) | Bit(56 + 5);
  m_board->black[5] = (m_board->black[5] ^ Bit(m_king_b))    | Bit(56 + 6);
  if (ChecksW()) return;
  m_moves_n++;
}

void AddCastleOOOB() {
  if (ChecksCastleW(m_castle_b[1])) return;
  HandleCastlingB(m_king_b, 56 + 2);
  m_board->board[m_rook_b[1]] = 0;
  m_board->board[m_king_b] = 0;
  m_board->board[56 + 3] = -4;
  m_board->board[56 + 2] = -6;
  m_board->black[3] = (m_board->black[3] ^ Bit(m_rook_b[1])) | Bit(56 + 3);
  m_board->black[5] = (m_board->black[5] ^ Bit(m_king_b))    | Bit(56 + 2);
  if (ChecksW()) return;
  m_moves_n++;
}

void MgenCastlingMovesB() {
  if ((m_board->castle & 4) && !(m_castle_empty_b[0] & m_both)) {AddCastleOOB();  m_board = m_board_original;}
  if ((m_board->castle & 8) && !(m_castle_empty_b[1] & m_both)) {AddCastleOOOB(); m_board = m_board_original;}
}

void CheckCastlingRightsW() {
  if (m_board->board[m_king_w]    != 6) {m_board->castle &= 4 | 8; return;}
  if (m_board->board[m_rook_w[0]] != 4)  m_board->castle &= 2 | 4 | 8;
  if (m_board->board[m_rook_w[1]] != 4)  m_board->castle &= 1 | 4 | 8;
}

void CheckCastlingRightsB() {
  if (m_board->board[m_king_b]    != -6) {m_board->castle &= 1 | 2; return;}
  if (m_board->board[m_rook_b[0]] != -4)  m_board->castle &= 1 | 2 | 8;
  if (m_board->board[m_rook_b[1]] != -4)  m_board->castle &= 1 | 2 | 4;
}

void HandleCastlingRights() {
  if (!m_board->castle) return;
  CheckCastlingRightsW();
  CheckCastlingRightsB();
}

void ModifyPawnStuffW(const int from, const int to) {
  if (to == m_board_original->epsq) {
    m_board->board[to - 8] = 0;
    m_board->black[0]     ^= Bit(to - 8);
  } else if (Ycoord(to) - Ycoord(from) == 2) {
    m_board->epsq = to - 8;
  }
}

void AddPromotionW(const int from, const int to, const int piece) {
  const int eat = m_board->board[to];
  m_moves[m_moves_n] = *m_board;
  m_board = &m_moves[m_moves_n];
  m_board->from        = from;
  m_board->to          = to;
  m_board->epsq        = -1;
  m_board->board[to]   = piece;
  m_board->board[from] = 0;
  m_board->white[0]   ^= Bit(from);
  m_board->white[piece - 1] |= Bit(to);
  if (eat <= -1) m_board->black[-eat - 1] ^= Bit(to);
  if (ChecksB()) return;
  HandleCastlingRights();
  m_moves_n++;
}

void AddPromotionStuffW(const int from, const int to) {
  Board *tmp = m_board;
  for (int piece = 2; piece <= 5; piece++) {
    AddPromotionW(from, to, piece);
    m_board = tmp;
  }
}

void AddNormalStuffW(const int from, const int to) {
  const int me = m_board->board[from], eat = m_board->board[to];
  if (me <= 0) return;
  m_moves[m_moves_n] = *m_board;
  m_board = &m_moves[m_moves_n];
  m_board->from          = from;
  m_board->to            = to;
  m_board->epsq          = -1;
  m_board->board[from]   = 0;
  m_board->board[to]     = me;
  m_board->white[me - 1] = (m_board->white[me - 1] ^ Bit(from)) | Bit(to);
  if (eat <= -1) m_board->black[-eat - 1] ^= Bit(to);
  if (m_board->board[to] == 1) ModifyPawnStuffW(from, to);
  if (ChecksB()) return;
  HandleCastlingRights();
  m_moves_n++;
}

void AddW(const int from, const int to) {
  if (m_board->board[from] == 1 && Ycoord(from) == 6)
    AddPromotionStuffW(from, to);
  else
    AddNormalStuffW(from, to);
}

void ModifyPawnStuffB(const int from, const int to) {
  if (to == m_board_original->epsq) {
    m_board->board[to + 8] = 0;
    m_board->white[0]     ^= Bit(to + 8);
  } else if (Ycoord(to) - Ycoord(from) == -2) {
    m_board->epsq = to + 8;
  }
}

void AddNormalStuffB(const int from, const int to) {
  const int me = m_board->board[from], eat = m_board->board[to];
  if (me >= 0) return;
  m_moves[m_moves_n] = *m_board;
  m_board                 = &m_moves[m_moves_n];
  m_board->from           = from;
  m_board->to             = to;
  m_board->epsq           = -1;
  m_board->board[to]      = me;
  m_board->board[from]    = 0;
  m_board->black[-me - 1] = (m_board->black[-me - 1] ^ Bit(from)) | Bit(to);
  if (eat >= 1) m_board->white[eat - 1] ^= Bit(to);
  if (m_board->board[to] == -1) ModifyPawnStuffB(from, to);
  if (ChecksW()) return;
  HandleCastlingRights();
  m_moves_n++;
}

void AddPromotionB(const int from, const int to, const int piece) {
  const int eat = m_board->board[to];
  m_moves[m_moves_n] = *m_board;
  m_board = &m_moves[m_moves_n];
  m_board->from        = from;
  m_board->to          = to;
  m_board->epsq        = -1;
  m_board->board[from] = 0;
  m_board->board[to]   = piece;
  m_board->black[0]   ^= Bit(from);
  m_board->black[-piece - 1] |= Bit(to);
  if (eat >= 1) m_board->white[eat - 1] ^= Bit(to);
  if (ChecksW()) return;
  HandleCastlingRights();
  m_moves_n++;
}

void AddPromotionStuffB(const int from, const int to) {
  Board *tmp = m_board;
  for (int piece = 2; piece <= 5; piece++) {
    AddPromotionB(from, to, -piece);
    m_board = tmp;
  }
}

void AddB(const int from, const int to) {
  if (m_board->board[from] == -1 && Ycoord(from) == 1)
    AddPromotionStuffB(from, to);
  else
    AddNormalStuffB(from, to);
}

void AddMovesW(const int from, uint64_t moves) {
  for (; moves; moves = ClearBit(moves)) {
    AddW(from, Lsb(moves));
    m_board = m_board_original;
  }
}

void AddMovesB(const int from, uint64_t moves) {
  for (; moves; moves = ClearBit(moves)) {
    AddB(from, Lsb(moves));
    m_board = m_board_original;
  }
}

void MgenSetupW() {
  m_white   = White();
  m_black   = Black();
  m_both    = m_white | m_black;
  m_empty   = ~m_both;
  m_pawn_sq = m_board->epsq > 0 ? m_black | (Bit(m_board->epsq) & 0x0000FF0000000000ULL) : m_black;
}

void MgenSetupB() {
  m_white   = White();
  m_black   = Black();
  m_both    = m_white | m_black;
  m_empty   = ~m_both;
  m_pawn_sq = m_board->epsq > 0 ? m_white | (Bit(m_board->epsq) & 0x0000000000FF0000ULL) : m_white;
}

void MgenPawnsW() {
  for (uint64_t pieces = m_board->white[0]; pieces; pieces = ClearBit(pieces)) {
    const int sq = Lsb(pieces);
    AddMovesW(sq, m_pawn_checks_w[sq] & m_pawn_sq);
    if (Ycoord(sq) == 1) {
      if (m_pawn_1_moves_w[sq] & m_empty) AddMovesW(sq, m_pawn_2_moves_w[sq] & m_empty);
    } else {
      AddMovesW(sq, m_pawn_1_moves_w[sq] & m_empty);
    }
  }
}

void MgenPawnsB() {
  for (uint64_t pieces = m_board->black[0]; pieces; pieces = ClearBit(pieces)) {
    const int sq = Lsb(pieces);
    AddMovesB(sq, m_pawn_checks_b[sq] & m_pawn_sq);
    if (Ycoord(sq) == 6) {
      if (m_pawn_1_moves_b[sq] & m_empty) AddMovesB(sq, m_pawn_2_moves_b[sq] & m_empty);
    } else {
      AddMovesB(sq, m_pawn_1_moves_b[sq] & m_empty);
    }
  }
}

void MgenKnightsW() {for (uint64_t pieces = m_board->white[1]; pieces; pieces = ClearBit(pieces)) {const int sq = Lsb(pieces); AddMovesW(sq, m_knight_moves[sq] & m_good);}}
void MgenKnightsB() {for (uint64_t pieces = m_board->black[1]; pieces; pieces = ClearBit(pieces)) {const int sq = Lsb(pieces); AddMovesB(sq, m_knight_moves[sq] & m_good);}}
void MgenBishopsPlusQueensW() {for (uint64_t pieces = m_board->white[2] | m_board->white[4]; pieces; pieces = ClearBit(pieces)) {const int sq = Lsb(pieces); AddMovesW(sq, BishopMagicMoves(sq, m_both) & m_good);}}
void MgenBishopsPlusQueensB() {for (uint64_t pieces = m_board->black[2] | m_board->black[4]; pieces; pieces = ClearBit(pieces)) {const int sq = Lsb(pieces); AddMovesB(sq, BishopMagicMoves(sq, m_both) & m_good);}}
void MgenRooksPlusQueensW() {for (uint64_t pieces = m_board->white[3] | m_board->white[4]; pieces; pieces = ClearBit(pieces)) {const int sq = Lsb(pieces); AddMovesW(sq, RookMagicMoves(sq, m_both) & m_good);}}
void MgenRooksPlusQueensB() {for (uint64_t pieces = m_board->black[3] | m_board->black[4]; pieces; pieces = ClearBit(pieces)) {const int sq = Lsb(pieces); AddMovesB(sq, RookMagicMoves(sq, m_both) & m_good);}}
void MgenKingW() {const int sq = Lsb(m_board->white[5]); AddMovesW(sq, m_king_moves[sq] & m_good);}
void MgenKingB() {const int sq = Lsb(m_board->black[5]); AddMovesB(sq, m_king_moves[sq] & m_good);}

void MgenAllW() {
  MgenSetupW();
  m_good = ~m_white;
  MgenPawnsW();
  MgenKnightsW();
  MgenBishopsPlusQueensW();
  MgenRooksPlusQueensW();
  MgenKingW();
  MgenCastlingMovesW();
}

void MgenAllB() {
  MgenSetupB();
  m_good = ~m_black;
  MgenPawnsB();
  MgenKnightsB();
  MgenBishopsPlusQueensB();
  MgenRooksPlusQueensB();
  MgenKingB();
  MgenCastlingMovesB();
}

int MgenW(Board *moves) {
  m_moves_n = 0;
  m_moves = moves;
  m_board_original = m_board;
  MgenAllW();
  return m_moves_n;
}

int MgenB(Board *moves) {
  m_moves_n = 0;
  m_moves = moves;
  m_board_original = m_board;
  MgenAllB();
  return m_moves_n;
}

// Perft

uint64_t PerftW(const int depth) {
  Board moves[k_max_moves];
  const uint64_t hash = Hash(1);
  uint64_t nodes = GetPerft(hash, depth);
  if (nodes) return nodes;
  const int len = MgenW(moves);
  if (depth <= 0) return (uint64_t) len;
  for (int i = 0; i < len; i++) {m_board = moves + i; nodes += PerftB(depth - 1);}
  AddPerft(hash, nodes, depth);
  return nodes;
}

uint64_t PerftB(const int depth) {
  Board moves[k_max_moves];
  const uint64_t hash = Hash(0);
  uint64_t nodes = GetPerft(hash, depth);
  if (nodes) return nodes;
  const int len = MgenB(moves);
  if (depth <= 0) return (uint64_t) len;
  for (int i = 0; i < len; i++) {m_board = moves + i; nodes += PerftW(depth - 1);}
  AddPerft(hash, nodes, depth);
  return nodes;
}

uint64_t Perft(const int depth) {
  if (depth <= 0) return 1;
  return m_wtm ? PerftW(depth - 1) : PerftB(depth - 1);
}

const std::string BigNumber(const uint64_t number) { // 561735852 -> 561,735,852
  std::string str = std::to_string(number), ret = "";
  const size_t len = str.length();
  for (size_t i = 0; i < len; i++) {
    if (i && ((len - i) % 3 == 0)) ret += ",";
    ret += str[i];
  }
  return ret;
}

double GetNps(const uint64_t nodes, const uint64_t ms) {const double ret = 0.000001 * ((double) Nps(nodes, ms)); return ret < 0.1 ? 0 : ret;}

void PerftPrint(const int depth, const uint64_t nodes, const uint64_t ms) {
  std::cout << std::setfill(' ') << std::setprecision(6) << depth << std::setw(18) << BigNumber(nodes) << std::setw(14) << GetNps(nodes, ms) << std::setw(12) << (0.001 * ((double) ms)) << std::endl;
}

void PerftPrintTotal(const uint64_t nodes, const uint64_t ms) {
  std::cout << std::setfill(' ') << std::setprecision(6) << "Total" << std::setw(14) << BigNumber(nodes) << std::setw(14) << GetNps(nodes, ms) << std::setw(12) << (0.001 * ((double) ms)) << std::endl;
}

void PerftRun(const int depth) {
  uint64_t nodes, start_time, diff_time, totaltime = 0, allnodes = 0;
  std::cout << "[ " << g_position << " ]" << std::endl;
  std::cout << "Depth           Nodes        Mnps        Time" << std::endl;
  for (int i = 0; i < depth + 1; i++) {
    Fen(g_position);
    start_time = Now();
    nodes      = Perft(i);
    diff_time  = Now() - start_time;
    totaltime  += diff_time;
    allnodes   += nodes;
    PerftPrint(i, nodes, diff_time);
  }
  std::cout << std::setfill('=') << std::setw(46) << ' ' << std::endl;
  std::cout << std::setfill(' ') << "               Nodes        Mnps        Time" << std::endl;
  PerftPrintTotal(allnodes, totaltime);
}

uint64_t SuiteRun(const int depth) {
  uint64_t start, nodes = 0, allnodes = 0;
  std::cout << "Depth            Nodes        Mnps        Time" << std::endl;
  for (int i = 0; i <= depth; i++) {
    Fen(g_position);
    start = Now();
    nodes = Perft(i);
    allnodes += nodes;
    PerftPrint(i, nodes, Now() - start);
  }
  return allnodes;
}

void Bench(const bool fullsuite) {
  uint64_t nodes = 0, start = Now();
  int nth = 0;
  const std::vector<std::string> suite = {
    // Normal : https://www.chessprogramming.org/Perft_Results
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - -",
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - -",
    "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - -",
    "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - -",
    "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - -",
    "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - -",
    // Chess960 : https://www.chessprogramming.org/Chess960_Perft_Results
    "bqnb1rkr/pp3ppp/3ppn2/2p5/5P2/P2P4/NPP1P1PP/BQ1BNRKR w HFhf - -",
    "bnqbnr1r/p1p1ppkp/3p4/1p4p1/P7/3NP2P/1PPP1PP1/BNQB1RKR w HF - -",
    "nrbq2kr/ppppppb1/5n1p/5Pp1/8/P5P1/1PPPP2P/NRBQNBKR w HBhb - -",
    "1r1bkqbr/pppp1ppp/2nnp3/8/2P5/N4P2/PP1PP1PP/1RNBKQBR w Hh - -",
    "rkqnbbnr/ppppppp1/8/7p/3N4/6PP/PPPPPP2/RKQNBB1R w HAa - -",
    "rbqkr1bn/pp1ppp2/2p1n2p/6p1/8/4BPNP/PPPPP1P1/RBQKRN2 w EAea - -"
  };
  for (auto fen : suite) {
    if (nth++) std::cout << std::endl;
    g_position = fen;
    std::cout << "[ #" << nth << ": " << fen << " ]" << std::endl;
    nodes += SuiteRun(fullsuite ? 6 : 5);
  }
  std::cout << std::setfill('=') << std::setw(46) << '\n' << std::endl;
  PerftPrintTotal(nodes, Now() - start);
  Assert(nodes == (fullsuite ? 21799671196 : 561735852), "Error #3: Broken move generator");
}

// Init

uint64_t PermutateBb(const uint64_t moves, const int index) {
  int total = 0, good[64] = {0};
  uint64_t permutations = 0;
  for (int i = 0; i < 64; i++) if (moves & Bit(i)) good[total++] = i;
  const int popn = PopCount(moves);
  for (int i = 0; i < popn; i++) if ((1 << i) & index) permutations |= Bit(good[i]);
  return permutations & moves;
}

uint64_t MakeSliderMagicMoves(const int *slider_vectors, const int square, const uint64_t moves) {
  uint64_t possible_moves = 0;
  const int x_square = Xcoord(square), y_square = Ycoord(square);
  for (int i = 0; i < 4; i++)
    for (int j = 1; j < 8; j++) {
      const int x = x_square + j * slider_vectors[2 * i], y = y_square + j * slider_vectors[2 * i + 1];
      if (!OnBoard(x, y)) break;
      const uint64_t tmp = Bit(8 * y + x);
      possible_moves    |= tmp;
      if (tmp & moves) break;
    }
  return possible_moves & (~Bit(square));
}

void InitBishopMagics() {
  for (int i = 0; i < 64; i++) {
    const uint64_t magics = k_bishop_move_magics[i] & (~Bit(i));
    for (int j = 0; j < 512; j++) {
      const uint64_t allmoves = PermutateBb(magics, j);
#if defined PEXT
      m_bishop_magic_moves[i][Pext(allmoves, k_bishop_mask[i])] = MakeSliderMagicMoves(k_bishop_vectors, i, allmoves);
#else
      m_bishop_magic_moves[i][BishopMagicIndex(i, allmoves)] = MakeSliderMagicMoves(k_bishop_vectors, i, allmoves);
#endif
    }
  }
}

void InitRookMagics() {
  for (int i = 0; i < 64; i++) {
    const uint64_t magics = k_rook_move_magics[i] & (~Bit(i));
    for (int j = 0; j < 4096; j++) {
      const uint64_t allmoves = PermutateBb(magics, j);
#if defined PEXT
      m_rook_magic_moves[i][Pext(allmoves, k_rook_mask[i])] = MakeSliderMagicMoves(k_rook_vectors, i, allmoves);
#else
      m_rook_magic_moves[i][RookMagicIndex(i, allmoves)] = MakeSliderMagicMoves(k_rook_vectors, i, allmoves);
#endif
    }
  }
}

uint64_t MakeSliderMoves(const int square, const int *slider_vectors) {
  uint64_t moves = 0;
  const int x_square = Xcoord(square), y_square = Ycoord(square);
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

void InitSliderMoves() {
  for (int i = 0; i < 64; i++) {
    m_rook_moves[i]   = MakeSliderMoves(i, k_rook_vectors);
    m_bishop_moves[i] = MakeSliderMoves(i, k_bishop_vectors);
    m_queen_moves[i]  = m_rook_moves[i] | m_bishop_moves[i];
  }
}

uint64_t MakeJumpMoves(const int square, const int len, const int dy, const int *jump_vectors) {
  uint64_t moves = 0;
  const int x_square = Xcoord(square), y_square = Ycoord(square);
  for (int i = 0; i < len; i++) {
    const int x = x_square + jump_vectors[2 * i], y = y_square + dy * jump_vectors[2 * i + 1];
    if (OnBoard(x, y)) moves |= Bit(8 * y + x);
  }
  return moves;
}

void InitJumpMoves() {
  const int pawn_check_vectors[2 * 2] = {-1,1,1,1}, pawn_1_vectors[1 * 2] = {0,1};
  for (int i = 0; i < 64; i++) {
    m_king_moves[i]     = MakeJumpMoves(i, 8,  1, k_king_vectors);
    m_knight_moves[i]   = MakeJumpMoves(i, 8,  1, k_knight_vectors);
    m_pawn_checks_w[i]  = MakeJumpMoves(i, 2,  1, pawn_check_vectors);
    m_pawn_checks_b[i]  = MakeJumpMoves(i, 2, -1, pawn_check_vectors);
    m_pawn_1_moves_w[i] = MakeJumpMoves(i, 1,  1, pawn_1_vectors);
    m_pawn_1_moves_b[i] = MakeJumpMoves(i, 1, -1, pawn_1_vectors);
  }
  for (int i = 0; i < 8; i++) {
    m_pawn_2_moves_w[ 8 + i] = MakeJumpMoves( 8 + i, 1,  1, pawn_1_vectors) | MakeJumpMoves( 8 + i, 1,  2, pawn_1_vectors);
    m_pawn_2_moves_b[48 + i] = MakeJumpMoves(48 + i, 1, -1, pawn_1_vectors) | MakeJumpMoves(48 + i, 1, -2, pawn_1_vectors);
  }
}

void InitZobrist() {
  for (int i = 0; i < 13; i++) for (int j = 0; j < 64; j++) z_board[i][j] = Random8x64();
  for (int i = 0; i < 64; i++) z_ep[i]     = Random8x64();
  for (int i = 0; i < 16; i++) z_castle[i] = Random8x64();
  for (int i = 0; i <  2; i++) z_wtm[i]    = Random8x64();
}

// Execute

void Init() {
  g_seed += (uint64_t) time(NULL);
  m_board_tmp.reset();
  InitBishopMagics();
  InitRookMagics();
  InitZobrist();
  InitSliderMoves();
  InitJumpMoves();
  HashtableSetSize(128);
  Fen(k_startpos);
  std::atexit(HashtableFreeMemory);
}

// Credit: https://gist.github.com/plasticbox/3708a6cdfbece8cd224487f9ca9794cd
const std::string getCmdOption(int argc, char *argv[], const std::string& option) {
  for (int i = 1; i < argc; i++) {const std::string arg = argv[i]; if (arg.find(option) == 0) return arg.substr(option.length());}
  return "";
}

bool findCmdOption(int argc, char* argv[], const std::string& option) {for (int i = 0; i < argc; i++) {if (option == std::string(argv[i])) return 1;} return 0;}

void PrintHelp() {
  std::cout << ":: Help ::" << std::endl;
  std::cout << "> lastemperor -hash=512 -bench=1 # Set 512 MB hash and run benchmarks" << std::endl;
  std::cout << "--help        This help" << std::endl;
  std::cout << "--version     Show Version" << std::endl;
  std::cout << "-hash=[N]     Set hash in N MB" << std::endl;
  std::cout << "-fen=[FEN]    Set fen" << std::endl;
  std::cout << "-perft=[1..]  Perft" << std::endl;
  std::cout << "-bench=[0,1]  Benchmarks [0 = normal, 1 = full]\n" << std::endl;
  std::cout << "Full source: <https://github.com/SamuraiDangyo/LastEmperor/>" << std::endl;
}

void Args(int argc, char **argv) {
  std::string tmp;
  if ((tmp = getCmdOption(argc, argv, "-fen=")) != "") {Fen(tmp);}
  if ((tmp = getCmdOption(argc, argv, "-hash=")) != "") {HashtableSetSize(Between<int>(1, std::stoi(tmp), 1024 * 1024));}
  if ((tmp = getCmdOption(argc, argv, "-perft=")) != "") {PerftRun(std::max(1, std::stoi(tmp))); return;}
  if ((tmp = getCmdOption(argc, argv, "-bench=")) != "") {Bench(Between<int>(0, std::stoi(tmp), 1)); return;}
  if (findCmdOption(argc, argv, "--version")) {std::cout << k_name << std::endl; return;}
  if (argc >= 2 && !findCmdOption(argc, argv, "--help")) {std::cout << "Invalid command: '" << std::string(argv[1]) << "'" << std::endl; return;}
  PrintHelp();
}}

// "War demands sacrifice of the people. It gives only suffering in return." -- Frederic Clemson Howe
int main(int argc, char **argv) {
  lastemperor::Init();
  lastemperor::Args(argc, argv);
  return EXIT_SUCCESS;
}
