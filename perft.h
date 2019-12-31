#ifndef PERFT_H
#define PERFT_H

///
/// Perft
///

static const PERFT_T PERFT_SUITE[2 * 960] = {

  // All 960 FR starting positions
  #include "perft_960_starting_pos.txt"

  ,

  // https://www.chessprogramming.org/Chess960_Perft_Results
  #include "perft_960_perft_results.txt"

};

#endif /* #ifndef PERFT_H */