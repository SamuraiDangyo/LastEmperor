#!/usr/bin/python
# -*- coding: utf-8 -*-

import re, time, sys

fin = open("perft.txt", "rt")
array = []

reg = re.compile(r"^(.*?),") # | re.DOTALL)

#reg = re.compile(r"^(.*?),") # | re.DOTALL)

for line in fin:
  #print(line)
  lst = re.findall(reg, line)
  lst = "{\"%s\", { 1,%s}}, " %(lst[0], line[len(lst[0]) + 1 : len(line) - 1])
  print(lst)
  array.append(line)
	
#print(array)
fin.close()
