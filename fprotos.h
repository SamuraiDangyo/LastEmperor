/*
LastEmperor, a Chess960 move generator (Derived from Sapeli 1.67)
Copyright (C) 2019 Toni Helminen
*/

//
// LastEmperor.c function prototypes
//

#ifndef FPROTOS_H_GUARD
#define FPROTOS_H_GUARD

static inline BITBOARD Bishop_magic_index(const int, const BITBOARD);
static inline BITBOARD Rook_magic_index(const int, const BITBOARD);
static inline BITBOARD Bishop_magic_moves(const int, const BITBOARD);
static inline BITBOARD Rook_magic_moves(const int, const BITBOARD);
static void Assert(const bool, const int);
static void Init_bishop_magics(void);
static void Build_bitboards(void);
static void Fen_KQkq(const char *);
static void Perft_print(const int, const BITBOARD, const BITBOARD);
static void Perft_run(const int);
static bool Add_pawn_b(const int, const int);
static void Fen_ep(const char *);
static void Assume_legal_position(void);
static void Add_moves_b(const int, BITBOARD);
static void Handle_castle_rights(void);
static BITBOARD Hash(const int);
static void Token_add(const char *);
static bool Checks_w(void);
static BITBOARD Perft_b(const int);
static void Add_castle_O_O_w(void);
static void Debug_tokens(void);
static void Add_castle_O_O_O_b(void);
static void Fen(const char *);
static void Add_underpromotion_b(const BOARD_T *, const int, const int);
static void Init(void);
static BITBOARD Nps(const BITBOARD, const BITBOARD);
static bool Add_pawn_w(const int, const int);
static int Mgen_w(BOARD_T *);
static void Mgen_all_b(void);
static BITBOARD Random_bb(void);
static bool Token_next(const char *);
static BITBOARD Suite_run(const int, const int);
static void Add_perft(const BITBOARD, const BITBOARD, const int);
static bool Equal_strings(const char *, const char *);
static bool Is_number(const char);
static void Handle_castling_b(void);
static bool Checks_b(void);
static void Init_board(void);
static void Check_castle_rights_b(void);
static void Add_b(const int, const int);
static void Add_moves_w(const int, BITBOARD);
static BITBOARD Mgen_bishop_real(const int, const BITBOARD);
static void String_join(char *, const char *);
static void Fen_board(const char *);
static void Castling_moves_w(const BITBOARD);
static void Perft_final_print(const BITBOARD, const BITBOARD);
static void Hashtable_free_memory(void);
static void Fen_create(const char *);
static BITBOARD Perft(const int);
static bool Checks_castle_w(BITBOARD);
static void Command_bench(void);
static int Token_next_int(void);
static void Hashtable_set_size(const int);
static void Command_setfen(void);
static void Add_underpromotion_w(const BOARD_T *, const int, const int);
static bool Token_is(const char *);
static void Add_castle_O_O_b(void);
static void Token_pop(void);
static void Init_tokens(int, char **);
static void Position_fen(void);
static void Token_expect(const char *);
static void Mgen_all_w(void);
static void Happy(void);
static BITBOARD Random_u64(void);
static void Find_castling_rooks_and_kings(void);
static bool Token_ok(void);
static void Check_castle_rights_w(void);
static void Init_zobrist(void);
static void Init_rook_magics(void);
static inline void Swap(BOARD_T *, BOARD_T *);
static void Add_w(const int, const int);
static void Ok(void);
static void Print_help(void);
static inline int Min(const int, const int);
static BITBOARD Mgen_slider_real(const int *, const int, const BITBOARD);
static BITBOARD Fill(int, const int);
static void Build_castling_bitboards(void);
static void Castling_moves_b(const BITBOARD);
static BITBOARD Mgen_rook_real(const int, const BITBOARD);
static BITBOARD Perft_w(const int);
static const char *Token_current(void);
static BITBOARD Permutate_bb(const BITBOARD, const int);
static void Print(const char *, ...);
static const char *Get_time_string(void);
static void Padding(const char *, const int);
static bool On_board(const int, const int);
static BITBOARD Get_perft(const BITBOARD, const int);
static void Commands(void);
static void Go(void);
static bool Checks_castle_b(BITBOARD);
static void Add_castle_O_O_O_w(void);
static void Token_reset(void);
static const char *Int_to_string(const int);
static BITBOARD Now(void);
static void Suite(const int);
static inline int Max(const int, const int);
static int Piece(const char);
static void Handle_castling_w(void);
static void Debug_log(const char *);
static int Mgen_b(BOARD_T *);

#endif /* #ifndef FPROTOS_H_GUARD */
