#ifndef DATA_H
#define DATA_H

#define SHORT_LICENSE "GNU General Public License version 3; for details see LICENSE"

///
/// BitBoard moves
///

static const BITBOARD ROOK_MAGIC[64] = {
  0x548001400080106cULL,
  0x900184000110820ULL,
  0x428004200a81080ULL,
  0x140088082000c40ULL,
  0x1480020800011400ULL,
  0x100008804085201ULL,
  0x2a40220001048140ULL,
  0x50000810000482aULL,
  0x250020100020a004ULL,
  0x3101880100900a00ULL,
  0x200a040a00082002ULL,
  0x1004300044032084ULL,
  0x2100408001013ULL,
  0x21f00440122083ULL,
  0xa204280406023040ULL,
  0x2241801020800041ULL,
  0xe10100800208004ULL,
  0x2010401410080ULL,
  0x181482000208805ULL,
  0x4080101000021c00ULL,
  0xa250210012080022ULL,
  0x4210641044000827ULL,
  0x8081a02300d4010ULL,
  0x8008012000410001ULL,
  0x28c0822120108100ULL,
  0x500160020aa005ULL,
  0xc11050088c1000ULL,
  0x48c00101000a288ULL,
  0x494a184408028200ULL,
  0x20880100240006ULL,
  0x10b4010200081ULL,
  0x40a200260000490cULL,
  0x22384003800050ULL,
  0x7102001a008010ULL,
  0x80020c8010900c0ULL,
  0x100204082a001060ULL,
  0x8000118188800428ULL,
  0x58e0020009140244ULL,
  0x100145040040188dULL,
  0x44120220400980ULL,
  0x114001007a00800ULL,
  0x80a0100516304000ULL,
  0x7200301488001000ULL,
  0x1000151040808018ULL,
  0x3000a200010e0020ULL,
  0x1000849180802810ULL,
  0x829100210208080ULL,
  0x1004050021528004ULL,
  0x61482000c41820b0ULL,
  0x241001018a401a4ULL,
  0x45020c009cc04040ULL,
  0x308210c020081200ULL,
  0xa000215040040ULL,
  0x10a6024001928700ULL,
  0x42c204800c804408ULL,
  0x30441a28614200ULL,
  0x40100229080420aULL,
  0x9801084000201103ULL,
  0x8408622090484202ULL,
  0x4022001048a0e2ULL,
  0x280120020049902ULL,
  0x1200412602009402ULL,
  0x914900048020884ULL,
  0x104824281002402ULL
};

static const BITBOARD ROOK_MASK[64] = {
  0x101010101017eULL,
  0x202020202027cULL,
  0x404040404047aULL,
  0x8080808080876ULL,
  0x1010101010106eULL,
  0x2020202020205eULL,
  0x4040404040403eULL,
  0x8080808080807eULL,
  0x1010101017e00ULL,
  0x2020202027c00ULL,
  0x4040404047a00ULL,
  0x8080808087600ULL,
  0x10101010106e00ULL,
  0x20202020205e00ULL,
  0x40404040403e00ULL,
  0x80808080807e00ULL,
  0x10101017e0100ULL,
  0x20202027c0200ULL,
  0x40404047a0400ULL,
  0x8080808760800ULL,
  0x101010106e1000ULL,
  0x202020205e2000ULL,
  0x404040403e4000ULL,
  0x808080807e8000ULL,
  0x101017e010100ULL,
  0x202027c020200ULL,
  0x404047a040400ULL,
  0x8080876080800ULL,
  0x1010106e101000ULL,
  0x2020205e202000ULL,
  0x4040403e404000ULL,
  0x8080807e808000ULL,
  0x1017e01010100ULL,
  0x2027c02020200ULL,
  0x4047a04040400ULL,
  0x8087608080800ULL,
  0x10106e10101000ULL,
  0x20205e20202000ULL,
  0x40403e40404000ULL,
  0x80807e80808000ULL,
  0x17e0101010100ULL,
  0x27c0202020200ULL,
  0x47a0404040400ULL,
  0x8760808080800ULL,
  0x106e1010101000ULL,
  0x205e2020202000ULL,
  0x403e4040404000ULL,
  0x807e8080808000ULL,
  0x7e010101010100ULL,
  0x7c020202020200ULL,
  0x7a040404040400ULL,
  0x76080808080800ULL,
  0x6e101010101000ULL,
  0x5e202020202000ULL,
  0x3e404040404000ULL,
  0x7e808080808000ULL,
  0x7e01010101010100ULL,
  0x7c02020202020200ULL,
  0x7a04040404040400ULL,
  0x7608080808080800ULL,
  0x6e10101010101000ULL,
  0x5e20202020202000ULL,
  0x3e40404040404000ULL,
  0x7e80808080808000ULL
};

static const BITBOARD BISHOP_MAGIC[64] = {
  0x2890208600480830ULL,
  0x324148050f087ULL,
  0x1402488a86402004ULL,
  0xc2210a1100044bULL,
  0x88450040b021110cULL,
  0xc0407240011ULL,
  0xd0246940cc101681ULL,
  0x1022840c2e410060ULL,
  0x4a1804309028d00bULL,
  0x821880304a2c0ULL,
  0x134088090100280ULL,
  0x8102183814c0208ULL,
  0x518598604083202ULL,
  0x67104040408690ULL,
  0x1010040020d000ULL,
  0x600001028911902ULL,
  0x8810183800c504c4ULL,
  0x2628200121054640ULL,
  0x28003000102006ULL,
  0x4100c204842244ULL,
  0x1221c50102421430ULL,
  0x80109046e0844002ULL,
  0xc128600019010400ULL,
  0x812218030404c38ULL,
  0x1224152461091c00ULL,
  0x1c820008124000aULL,
  0xa004868015010400ULL,
  0x34c080004202040ULL,
  0x200100312100c001ULL,
  0x4030048118314100ULL,
  0x410000090018ULL,
  0x142c010480801ULL,
  0x8080841c1d004262ULL,
  0x81440f004060406ULL,
  0x400a090008202ULL,
  0x2204020084280080ULL,
  0xb820060400008028ULL,
  0x110041840112010ULL,
  0x8002080a1c84400ULL,
  0x212100111040204aULL,
  0x9412118200481012ULL,
  0x804105002001444cULL,
  0x103001280823000ULL,
  0x40088e028080300ULL,
  0x51020d8080246601ULL,
  0x4a0a100e0804502aULL,
  0x5042028328010ULL,
  0xe000808180020200ULL,
  0x1002020620608101ULL,
  0x1108300804090c00ULL,
  0x180404848840841ULL,
  0x100180040ac80040ULL,
  0x20840000c1424001ULL,
  0x82c00400108800ULL,
  0x28c0493811082aULL,
  0x214980910400080cULL,
  0x8d1a0210b0c000ULL,
  0x164c500ca0410cULL,
  0xc6040804283004ULL,
  0x14808001a040400ULL,
  0x180450800222a011ULL,
  0x600014600490202ULL,
  0x21040100d903ULL,
  0x10404821000420ULL
};

static const BITBOARD BISHOP_MASK[64] = {
  0x40201008040200ULL,
  0x402010080400ULL,
  0x4020100a00ULL,
  0x40221400ULL,
  0x2442800ULL,
  0x204085000ULL,
  0x20408102000ULL,
  0x2040810204000ULL,
  0x20100804020000ULL,
  0x40201008040000ULL,
  0x4020100a0000ULL,
  0x4022140000ULL,
  0x244280000ULL,
  0x20408500000ULL,
  0x2040810200000ULL,
  0x4081020400000ULL,
  0x10080402000200ULL,
  0x20100804000400ULL,
  0x4020100a000a00ULL,
  0x402214001400ULL,
  0x24428002800ULL,
  0x2040850005000ULL,
  0x4081020002000ULL,
  0x8102040004000ULL,
  0x8040200020400ULL,
  0x10080400040800ULL,
  0x20100a000a1000ULL,
  0x40221400142200ULL,
  0x2442800284400ULL,
  0x4085000500800ULL,
  0x8102000201000ULL,
  0x10204000402000ULL,
  0x4020002040800ULL,
  0x8040004081000ULL,
  0x100a000a102000ULL,
  0x22140014224000ULL,
  0x44280028440200ULL,
  0x8500050080400ULL,
  0x10200020100800ULL,
  0x20400040201000ULL,
  0x2000204081000ULL,
  0x4000408102000ULL,
  0xa000a10204000ULL,
  0x14001422400000ULL,
  0x28002844020000ULL,
  0x50005008040200ULL,
  0x20002010080400ULL,
  0x40004020100800ULL,
  0x20408102000ULL,
  0x40810204000ULL,
  0xa1020400000ULL,
  0x142240000000ULL,
  0x284402000000ULL,
  0x500804020000ULL,
  0x201008040200ULL,
  0x402010080400ULL,
  0x2040810204000ULL,
  0x4081020400000ULL,
  0xa102040000000ULL,
  0x14224000000000ULL,
  0x28440200000000ULL,
  0x50080402000000ULL,
  0x20100804020000ULL,
  0x40201008040200ULL
};

static const BITBOARD PAWN_CHECKS_W[64] = {
  0x200ULL,
  0x500ULL,
  0xa00ULL,
  0x1400ULL,
  0x2800ULL,
  0x5000ULL,
  0xa000ULL,
  0x4000ULL,
  0x20000ULL,
  0x50000ULL,
  0xa0000ULL,
  0x140000ULL,
  0x280000ULL,
  0x500000ULL,
  0xa00000ULL,
  0x400000ULL,
  0x2000000ULL,
  0x5000000ULL,
  0xa000000ULL,
  0x14000000ULL,
  0x28000000ULL,
  0x50000000ULL,
  0xa0000000ULL,
  0x40000000ULL,
  0x200000000ULL,
  0x500000000ULL,
  0xa00000000ULL,
  0x1400000000ULL,
  0x2800000000ULL,
  0x5000000000ULL,
  0xa000000000ULL,
  0x4000000000ULL,
  0x20000000000ULL,
  0x50000000000ULL,
  0xa0000000000ULL,
  0x140000000000ULL,
  0x280000000000ULL,
  0x500000000000ULL,
  0xa00000000000ULL,
  0x400000000000ULL,
  0x2000000000000ULL,
  0x5000000000000ULL,
  0xa000000000000ULL,
  0x14000000000000ULL,
  0x28000000000000ULL,
  0x50000000000000ULL,
  0xa0000000000000ULL,
  0x40000000000000ULL,
  0x200000000000000ULL,
  0x500000000000000ULL,
  0xa00000000000000ULL,
  0x1400000000000000ULL,
  0x2800000000000000ULL,
  0x5000000000000000ULL,
  0xa000000000000000ULL,
  0x4000000000000000ULL,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0
};

static const BITBOARD PAWN_CHECKS_B[64] = {
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0x2ULL,
  0x5ULL,
  0xaULL,
  0x14ULL,
  0x28ULL,
  0x50ULL,
  0xa0ULL,
  0x40ULL,
  0x200ULL,
  0x500ULL,
  0xa00ULL,
  0x1400ULL,
  0x2800ULL,
  0x5000ULL,
  0xa000ULL,
  0x4000ULL,
  0x20000ULL,
  0x50000ULL,
  0xa0000ULL,
  0x140000ULL,
  0x280000ULL,
  0x500000ULL,
  0xa00000ULL,
  0x400000ULL,
  0x2000000ULL,
  0x5000000ULL,
  0xa000000ULL,
  0x14000000ULL,
  0x28000000ULL,
  0x50000000ULL,
  0xa0000000ULL,
  0x40000000ULL,
  0x200000000ULL,
  0x500000000ULL,
  0xa00000000ULL,
  0x1400000000ULL,
  0x2800000000ULL,
  0x5000000000ULL,
  0xa000000000ULL,
  0x4000000000ULL,
  0x20000000000ULL,
  0x50000000000ULL,
  0xa0000000000ULL,
  0x140000000000ULL,
  0x280000000000ULL,
  0x500000000000ULL,
  0xa00000000000ULL,
  0x400000000000ULL,
  0x2000000000000ULL,
  0x5000000000000ULL,
  0xa000000000000ULL,
  0x14000000000000ULL,
  0x28000000000000ULL,
  0x50000000000000ULL,
  0xa0000000000000ULL,
  0x40000000000000ULL
};

static const BITBOARD KNIGHT_MOVES[64] = {
  0x20400ULL,
  0x50800ULL,
  0xa1100ULL,
  0x142200ULL,
  0x284400ULL,
  0x508800ULL,
  0xa01000ULL,
  0x402000ULL,
  0x2040004ULL,
  0x5080008ULL,
  0xa110011ULL,
  0x14220022ULL,
  0x28440044ULL,
  0x50880088ULL,
  0xa0100010ULL,
  0x40200020ULL,
  0x204000402ULL,
  0x508000805ULL,
  0xa1100110aULL,
  0x1422002214ULL,
  0x2844004428ULL,
  0x5088008850ULL,
  0xa0100010a0ULL,
  0x4020002040ULL,
  0x20400040200ULL,
  0x50800080500ULL,
  0xa1100110a00ULL,
  0x142200221400ULL,
  0x284400442800ULL,
  0x508800885000ULL,
  0xa0100010a000ULL,
  0x402000204000ULL,
  0x2040004020000ULL,
  0x5080008050000ULL,
  0xa1100110a0000ULL,
  0x14220022140000ULL,
  0x28440044280000ULL,
  0x50880088500000ULL,
  0xa0100010a00000ULL,
  0x40200020400000ULL,
  0x204000402000000ULL,
  0x508000805000000ULL,
  0xa1100110a000000ULL,
  0x1422002214000000ULL,
  0x2844004428000000ULL,
  0x5088008850000000ULL,
  0xa0100010a0000000ULL,
  0x4020002040000000ULL,
  0x400040200000000ULL,
  0x800080500000000ULL,
  0x1100110a00000000ULL,
  0x2200221400000000ULL,
  0x4400442800000000ULL,
  0x8800885000000000ULL,
  0x100010a000000000ULL,
  0x2000204000000000ULL,
  0x4020000000000ULL,
  0x8050000000000ULL,
  0x110a0000000000ULL,
  0x22140000000000ULL,
  0x44280000000000ULL,
  0x88500000000000ULL,
  0x10a00000000000ULL,
  0x20400000000000ULL
};

static const BITBOARD KING_MOVES[64] = {
  0x302ULL,
  0x705ULL,
  0xe0aULL,
  0x1c14ULL,
  0x3828ULL,
  0x7050ULL,
  0xe0a0ULL,
  0xc040ULL,
  0x30203ULL,
  0x70507ULL,
  0xe0a0eULL,
  0x1c141cULL,
  0x382838ULL,
  0x705070ULL,
  0xe0a0e0ULL,
  0xc040c0ULL,
  0x3020300ULL,
  0x7050700ULL,
  0xe0a0e00ULL,
  0x1c141c00ULL,
  0x38283800ULL,
  0x70507000ULL,
  0xe0a0e000ULL,
  0xc040c000ULL,
  0x302030000ULL,
  0x705070000ULL,
  0xe0a0e0000ULL,
  0x1c141c0000ULL,
  0x3828380000ULL,
  0x7050700000ULL,
  0xe0a0e00000ULL,
  0xc040c00000ULL,
  0x30203000000ULL,
  0x70507000000ULL,
  0xe0a0e000000ULL,
  0x1c141c000000ULL,
  0x382838000000ULL,
  0x705070000000ULL,
  0xe0a0e0000000ULL,
  0xc040c0000000ULL,
  0x3020300000000ULL,
  0x7050700000000ULL,
  0xe0a0e00000000ULL,
  0x1c141c00000000ULL,
  0x38283800000000ULL,
  0x70507000000000ULL,
  0xe0a0e000000000ULL,
  0xc040c000000000ULL,
  0x302030000000000ULL,
  0x705070000000000ULL,
  0xe0a0e0000000000ULL,
  0x1c141c0000000000ULL,
  0x3828380000000000ULL,
  0x7050700000000000ULL,
  0xe0a0e00000000000ULL,
  0xc040c00000000000ULL,
  0x203000000000000ULL,
  0x507000000000000ULL,
  0xa0e000000000000ULL,
  0x141c000000000000ULL,
  0x2838000000000000ULL,
  0x5070000000000000ULL,
  0xa0e0000000000000ULL,
  0x40c0000000000000ULL
};

static const BITBOARD BISHOP_MOVE_MAGICS[64] = {
  0x40201008040200ULL,
  0x402010080400ULL,
  0x4020100a00ULL,
  0x40221400ULL,
  0x2442800ULL,
  0x204085000ULL,
  0x20408102000ULL,
  0x2040810204000ULL,
  0x20100804020000ULL,
  0x40201008040000ULL,
  0x4020100a0000ULL,
  0x4022140000ULL,
  0x244280000ULL,
  0x20408500000ULL,
  0x2040810200000ULL,
  0x4081020400000ULL,
  0x10080402000200ULL,
  0x20100804000400ULL,
  0x4020100a000a00ULL,
  0x402214001400ULL,
  0x24428002800ULL,
  0x2040850005000ULL,
  0x4081020002000ULL,
  0x8102040004000ULL,
  0x8040200020400ULL,
  0x10080400040800ULL,
  0x20100a000a1000ULL,
  0x40221400142200ULL,
  0x2442800284400ULL,
  0x4085000500800ULL,
  0x8102000201000ULL,
  0x10204000402000ULL,
  0x4020002040800ULL,
  0x8040004081000ULL,
  0x100a000a102000ULL,
  0x22140014224000ULL,
  0x44280028440200ULL,
  0x8500050080400ULL,
  0x10200020100800ULL,
  0x20400040201000ULL,
  0x2000204081000ULL,
  0x4000408102000ULL,
  0xa000a10204000ULL,
  0x14001422400000ULL,
  0x28002844020000ULL,
  0x50005008040200ULL,
  0x20002010080400ULL,
  0x40004020100800ULL,
  0x20408102000ULL,
  0x40810204000ULL,
  0xa1020400000ULL,
  0x142240000000ULL,
  0x284402000000ULL,
  0x500804020000ULL,
  0x201008040200ULL,
  0x402010080400ULL,
  0x2040810204000ULL,
  0x4081020400000ULL,
  0xa102040000000ULL,
  0x14224000000000ULL,
  0x28440200000000ULL,
  0x50080402000000ULL,
  0x20100804020000ULL,
  0x40201008040200ULL
};

static const BITBOARD ROOK_MOVE_MAGICS[64] = {
  0x101010101017eULL,
  0x202020202027cULL,
  0x404040404047aULL,
  0x8080808080876ULL,
  0x1010101010106eULL,
  0x2020202020205eULL,
  0x4040404040403eULL,
  0x8080808080807eULL,
  0x1010101017e00ULL,
  0x2020202027c00ULL,
  0x4040404047a00ULL,
  0x8080808087600ULL,
  0x10101010106e00ULL,
  0x20202020205e00ULL,
  0x40404040403e00ULL,
  0x80808080807e00ULL,
  0x10101017e0100ULL,
  0x20202027c0200ULL,
  0x40404047a0400ULL,
  0x8080808760800ULL,
  0x101010106e1000ULL,
  0x202020205e2000ULL,
  0x404040403e4000ULL,
  0x808080807e8000ULL,
  0x101017e010100ULL,
  0x202027c020200ULL,
  0x404047a040400ULL,
  0x8080876080800ULL,
  0x1010106e101000ULL,
  0x2020205e202000ULL,
  0x4040403e404000ULL,
  0x8080807e808000ULL,
  0x1017e01010100ULL,
  0x2027c02020200ULL,
  0x4047a04040400ULL,
  0x8087608080800ULL,
  0x10106e10101000ULL,
  0x20205e20202000ULL,
  0x40403e40404000ULL,
  0x80807e80808000ULL,
  0x17e0101010100ULL,
  0x27c0202020200ULL,
  0x47a0404040400ULL,
  0x8760808080800ULL,
  0x106e1010101000ULL,
  0x205e2020202000ULL,
  0x403e4040404000ULL,
  0x807e8080808000ULL,
  0x7e010101010100ULL,
  0x7c020202020200ULL,
  0x7a040404040400ULL,
  0x76080808080800ULL,
  0x6e101010101000ULL,
  0x5e202020202000ULL,
  0x3e404040404000ULL,
  0x7e808080808000ULL,
  0x7e01010101010100ULL,
  0x7c02020202020200ULL,
  0x7a04040404040400ULL,
  0x7608080808080800ULL,
  0x6e10101010101000ULL,
  0x5e20202020202000ULL,
  0x3e40404040404000ULL,
  0x7e80808080808000ULL
};

static const BITBOARD PAWN_MOVES_1_W[64] = {
  0x100ULL,
  0x200ULL,
  0x400ULL,
  0x800ULL,
  0x1000ULL,
  0x2000ULL,
  0x4000ULL,
  0x8000ULL,
  0x10000ULL,
  0x20000ULL,
  0x40000ULL,
  0x80000ULL,
  0x100000ULL,
  0x200000ULL,
  0x400000ULL,
  0x800000ULL,
  0x1000000ULL,
  0x2000000ULL,
  0x4000000ULL,
  0x8000000ULL,
  0x10000000ULL,
  0x20000000ULL,
  0x40000000ULL,
  0x80000000ULL,
  0x100000000ULL,
  0x200000000ULL,
  0x400000000ULL,
  0x800000000ULL,
  0x1000000000ULL,
  0x2000000000ULL,
  0x4000000000ULL,
  0x8000000000ULL,
  0x10000000000ULL,
  0x20000000000ULL,
  0x40000000000ULL,
  0x80000000000ULL,
  0x100000000000ULL,
  0x200000000000ULL,
  0x400000000000ULL,
  0x800000000000ULL,
  0x1000000000000ULL,
  0x2000000000000ULL,
  0x4000000000000ULL,
  0x8000000000000ULL,
  0x10000000000000ULL,
  0x20000000000000ULL,
  0x40000000000000ULL,
  0x80000000000000ULL,
  0x100000000000000ULL,
  0x200000000000000ULL,
  0x400000000000000ULL,
  0x800000000000000ULL,
  0x1000000000000000ULL,
  0x2000000000000000ULL,
  0x4000000000000000ULL,
  0x8000000000000000ULL,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0
};

static const BITBOARD PAWN_MOVES_1_B[64] = {
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0x1ULL,
  0x2ULL,
  0x4ULL,
  0x8ULL,
  0x10ULL,
  0x20ULL,
  0x40ULL,
  0x80ULL,
  0x100ULL,
  0x200ULL,
  0x400ULL,
  0x800ULL,
  0x1000ULL,
  0x2000ULL,
  0x4000ULL,
  0x8000ULL,
  0x10000ULL,
  0x20000ULL,
  0x40000ULL,
  0x80000ULL,
  0x100000ULL,
  0x200000ULL,
  0x400000ULL,
  0x800000ULL,
  0x1000000ULL,
  0x2000000ULL,
  0x4000000ULL,
  0x8000000ULL,
  0x10000000ULL,
  0x20000000ULL,
  0x40000000ULL,
  0x80000000ULL,
  0x100000000ULL,
  0x200000000ULL,
  0x400000000ULL,
  0x800000000ULL,
  0x1000000000ULL,
  0x2000000000ULL,
  0x4000000000ULL,
  0x8000000000ULL,
  0x10000000000ULL,
  0x20000000000ULL,
  0x40000000000ULL,
  0x80000000000ULL,
  0x100000000000ULL,
  0x200000000000ULL,
  0x400000000000ULL,
  0x800000000000ULL,
  0x1000000000000ULL,
  0x2000000000000ULL,
  0x4000000000000ULL,
  0x8000000000000ULL,
  0x10000000000000ULL,
  0x20000000000000ULL,
  0x40000000000000ULL,
  0x80000000000000ULL
};

static const BITBOARD PAWN_MOVES_2_W[64] = {
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0x1010000ULL,
  0x2020000ULL,
  0x4040000ULL,
  0x8080000ULL,
  0x10100000ULL,
  0x20200000ULL,
  0x40400000ULL,
  0x80800000ULL,
  0x1000000ULL,
  0x2000000ULL,
  0x4000000ULL,
  0x8000000ULL,
  0x10000000ULL,
  0x20000000ULL,
  0x40000000ULL,
  0x80000000ULL,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0
};

static const BITBOARD PAWN_MOVES_2_B[64] = {
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0x100000000ULL,
  0x200000000ULL,
  0x400000000ULL,
  0x800000000ULL,
  0x1000000000ULL,
  0x2000000000ULL,
  0x4000000000ULL,
  0x8000000000ULL,
  0x10100000000ULL,
  0x20200000000ULL,
  0x40400000000ULL,
  0x80800000000ULL,
  0x101000000000ULL,
  0x202000000000ULL,
  0x404000000000ULL,
  0x808000000000ULL,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0
};

static const BITBOARD BISHOP_MOVES[64] = {
  0x8040201008040201ULL,
  0x80402010080502ULL,
  0x804020110a04ULL,
  0x8041221408ULL,
  0x182442810ULL,
  0x10204885020ULL,
  0x102040810a040ULL,
  0x102040810204080ULL,
  0x4020100804020102ULL,
  0x8040201008050205ULL,
  0x804020110a040aULL,
  0x804122140814ULL,
  0x18244281028ULL,
  0x1020488502050ULL,
  0x102040810a040a0ULL,
  0x204081020408040ULL,
  0x2010080402010204ULL,
  0x4020100805020508ULL,
  0x804020110a040a11ULL,
  0x80412214081422ULL,
  0x1824428102844ULL,
  0x102048850205088ULL,
  0x2040810a040a010ULL,
  0x408102040804020ULL,
  0x1008040201020408ULL,
  0x2010080502050810ULL,
  0x4020110a040a1120ULL,
  0x8041221408142241ULL,
  0x182442810284482ULL,
  0x204885020508804ULL,
  0x40810a040a01008ULL,
  0x810204080402010ULL,
  0x804020102040810ULL,
  0x1008050205081020ULL,
  0x20110a040a112040ULL,
  0x4122140814224180ULL,
  0x8244281028448201ULL,
  0x488502050880402ULL,
  0x810a040a0100804ULL,
  0x1020408040201008ULL,
  0x402010204081020ULL,
  0x805020508102040ULL,
  0x110a040a11204080ULL,
  0x2214081422418000ULL,
  0x4428102844820100ULL,
  0x8850205088040201ULL,
  0x10a040a010080402ULL,
  0x2040804020100804ULL,
  0x201020408102040ULL,
  0x502050810204080ULL,
  0xa040a1120408000ULL,
  0x1408142241800000ULL,
  0x2810284482010000ULL,
  0x5020508804020100ULL,
  0xa040a01008040201ULL,
  0x4080402010080402ULL,
  0x102040810204080ULL,
  0x205081020408000ULL,
  0x40a112040800000ULL,
  0x814224180000000ULL,
  0x1028448201000000ULL,
  0x2050880402010000ULL,
  0x40a0100804020100ULL,
  0x8040201008040201ULL
};

static const BITBOARD ROOK_MOVES[64] = {
  0x1010101010101ffULL,
  0x2020202020202ffULL,
  0x4040404040404ffULL,
  0x8080808080808ffULL,
  0x10101010101010ffULL,
  0x20202020202020ffULL,
  0x40404040404040ffULL,
  0x80808080808080ffULL,
  0x10101010101ff01ULL,
  0x20202020202ff02ULL,
  0x40404040404ff04ULL,
  0x80808080808ff08ULL,
  0x101010101010ff10ULL,
  0x202020202020ff20ULL,
  0x404040404040ff40ULL,
  0x808080808080ff80ULL,
  0x101010101ff0101ULL,
  0x202020202ff0202ULL,
  0x404040404ff0404ULL,
  0x808080808ff0808ULL,
  0x1010101010ff1010ULL,
  0x2020202020ff2020ULL,
  0x4040404040ff4040ULL,
  0x8080808080ff8080ULL,
  0x1010101ff010101ULL,
  0x2020202ff020202ULL,
  0x4040404ff040404ULL,
  0x8080808ff080808ULL,
  0x10101010ff101010ULL,
  0x20202020ff202020ULL,
  0x40404040ff404040ULL,
  0x80808080ff808080ULL,
  0x10101ff01010101ULL,
  0x20202ff02020202ULL,
  0x40404ff04040404ULL,
  0x80808ff08080808ULL,
  0x101010ff10101010ULL,
  0x202020ff20202020ULL,
  0x404040ff40404040ULL,
  0x808080ff80808080ULL,
  0x101ff0101010101ULL,
  0x202ff0202020202ULL,
  0x404ff0404040404ULL,
  0x808ff0808080808ULL,
  0x1010ff1010101010ULL,
  0x2020ff2020202020ULL,
  0x4040ff4040404040ULL,
  0x8080ff8080808080ULL,
  0x1ff010101010101ULL,
  0x2ff020202020202ULL,
  0x4ff040404040404ULL,
  0x8ff080808080808ULL,
  0x10ff101010101010ULL,
  0x20ff202020202020ULL,
  0x40ff404040404040ULL,
  0x80ff808080808080ULL,
  0xff01010101010101ULL,
  0xff02020202020202ULL,
  0xff04040404040404ULL,
  0xff08080808080808ULL,
  0xff10101010101010ULL,
  0xff20202020202020ULL,
  0xff40404040404040ULL,
  0xff80808080808080ULL
};

static const BITBOARD QUEEN_MOVES[64] = {
  0x81412111090503ffULL,
  0x2824222120a07ffULL,
  0x404844424150effULL,
  0x8080888492a1cffULL,
  0x10101011925438ffULL,
  0x2020212224a870ffULL,
  0x404142444850e0ffULL,
  0x8182848890a0c0ffULL,
  0x412111090503ff03ULL,
  0x824222120a07ff07ULL,
  0x4844424150eff0eULL,
  0x80888492a1cff1cULL,
  0x101011925438ff38ULL,
  0x20212224a870ff70ULL,
  0x4142444850e0ffe0ULL,
  0x82848890a0c0ffc0ULL,
  0x2111090503ff0305ULL,
  0x4222120a07ff070aULL,
  0x844424150eff0e15ULL,
  0x888492a1cff1c2aULL,
  0x1011925438ff3854ULL,
  0x212224a870ff70a8ULL,
  0x42444850e0ffe050ULL,
  0x848890a0c0ffc0a0ULL,
  0x11090503ff030509ULL,
  0x22120a07ff070a12ULL,
  0x4424150eff0e1524ULL,
  0x88492a1cff1c2a49ULL,
  0x11925438ff385492ULL,
  0x2224a870ff70a824ULL,
  0x444850e0ffe05048ULL,
  0x8890a0c0ffc0a090ULL,
  0x90503ff03050911ULL,
  0x120a07ff070a1222ULL,
  0x24150eff0e152444ULL,
  0x492a1cff1c2a4988ULL,
  0x925438ff38549211ULL,
  0x24a870ff70a82422ULL,
  0x4850e0ffe0504844ULL,
  0x90a0c0ffc0a09088ULL,
  0x503ff0305091121ULL,
  0xa07ff070a122242ULL,
  0x150eff0e15244484ULL,
  0x2a1cff1c2a498808ULL,
  0x5438ff3854921110ULL,
  0xa870ff70a8242221ULL,
  0x50e0ffe050484442ULL,
  0xa0c0ffc0a0908884ULL,
  0x3ff030509112141ULL,
  0x7ff070a12224282ULL,
  0xeff0e1524448404ULL,
  0x1cff1c2a49880808ULL,
  0x38ff385492111010ULL,
  0x70ff70a824222120ULL,
  0xe0ffe05048444241ULL,
  0xc0ffc0a090888482ULL,
  0xff03050911214181ULL,
  0xff070a1222428202ULL,
  0xff0e152444840404ULL,
  0xff1c2a4988080808ULL,
  0xff38549211101010ULL,
  0xff70a82422212020ULL,
  0xffe0504844424140ULL,
  0xffc0a09088848281ULL
};

#endif /* #ifndef DATA_H */