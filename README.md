![LastEmperor](https://github.com/SamuraiDangyo/LastEmperor/blob/master/logo.jpg)

# LastEmperor
A Chess960 program (Derived from Sapeli 1.67)
Very lightweight: 834 sloc

## How To Use
`make` should build a fast binary.
If not then remove `-DPEXT` and `-DMODERN` flags until it works.

## Example: Kiwipete to depth 6
```
lastemperor -hash=1024 -fen="r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -" -perft=6
```

## Credits
logo
