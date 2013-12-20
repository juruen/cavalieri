#!/bin/sh

yacc -o parser.cpp --defines=parser.h parser.yy
flex -o scanner.cpp scanner.ll


