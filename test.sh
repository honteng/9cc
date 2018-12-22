#!/bin/bash
try() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  gcc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$expected expected, but got $actual"
    exit 1
  fi
}

try 6 '6;'
try 42 '6*7;'
try 0 '0;'
try 21 '5+20-4;'
try 41 ' 12 + 34 - 5; '
try 47 '5+6*7;'
try 15 "5*(9-6);"
try 4 "(3+5)/2;"
try 1 "a=1; a;"
try 2 "a=1; a+1;"
try 3 "a=1; b=2; a+b;"
try 5 "(10-3)-2;"
try 5 "10-3-2;"
try 9 "10-(3-2);"
try 2 "(3+5)/2/2;"
try 8 "(3+5)/2*2;"
try 1 "1==1;"
try 0 "1<1;"
try 1 "1<2;"
try 1 "1 <= 1;"
try 0 "1>1;"
try 1 "2>1;"
try 1 "1 >= 1;"
try 6 "return 6; 1;"
try 26 'return 6+10*2; 1;'
try 1 "1!=3;"
try 0 "1!=1;"
try 4 "{a=1; b=a+2; return b+1;}"
echo OK
