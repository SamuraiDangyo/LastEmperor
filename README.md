# LastEmperor
Chess960 perft program written in C++14.

## Build
`make` should build a fast binary.
If not then remove the `-DPEXT` flag and try again.

## Example: Kiwipete to depth 6 (+ 1024 MB hash)
`lastemperor -perft "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -" 6 1024`

## License
GPLv3
