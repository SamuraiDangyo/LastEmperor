#!/usr/bin/python
# -*- coding: utf-8 -*-

import re, time, sys

START_STR = """/**
* LastEmperor, a Chess960 move generator (Derived from Sapeli 1.67)
* Copyright (C) 2019 Toni Helminen
**/

/**
* LastEmperor.c function declarations
* Generated by makefdec.py
**/

#ifndef FDEC_H
#define FDEC_H

"""
END_STR = """

#endif /* #ifndef FDEC_H */
"""

def go():
  f = open("LastEmperor.c", "r")
  data = f.read()

  reg = re.compile(r"^[\w]+.*?\)", re.MULTILINE) # | re.DOTALL)
  lst = re.findall(reg, data)

  dec = START_STR

  # Not main()
  lst = lst[:-1]

  # Remove dublicates
  lst = list(dict.fromkeys(lst))
  dec += ";\n".join(s for s in lst)
  dec += ";"

  dec += END_STR

  f = open("fdec.h", "w")
  f.write(dec)
  f.close()

def main():
  print "~+~+~ makefdec.py ~+~+~"
  print "> Generating function declarations from LastEmperor.c ..." # w/o Herculean effort ...
  start = time.time()
  go()
  print ("= Done! ( %.3fs )" % (time.time() - start))

if __name__ == "__main__":
    main()
