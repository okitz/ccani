#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./ccani "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 '0;'
assert 42 '42;'
assert 13 "10+5-2+0;"
assert 22 " 10 + 15 - 3 + 0 ;"
assert 47 '5+6*7;'
assert 15 '5 * (9-6);'
assert 4 '(3+5)/2;'
assert 10 '-10+20;'
assert 30 '+20-(-10);'

assert 1 '1 < 2;'
assert 0 '(1+4*5) >= 20 == 0;'
assert 3 '(1 == 1) * 3;'

assert 9 'a = 1;b = a * 3;a = b * 3;'

assert 5 'a_343_d = 5;a_343_d;'
assert 3 '___a___ = 2;___a___ + 1;'

assert 14 'a = 3; b = 5 * 6 - 8; return a + b / 2;'
assert 22 'returner = 3; return b = 5 * 6 - 8;  a + b / 2;'

assert 2 'if(a = 10) return 1 + 1;'
assert 4 'if(1 > 4) return 1; else return 2*2;'
assert 64 'i = 1;while(i < 50){i = i*2;} return i;'
assert 34 's=0;m=1;for(i = 1;i <= 4;i=i+1){s=s+i;m=m*i;} return s+m;'

echo OK