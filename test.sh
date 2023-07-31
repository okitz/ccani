#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./ccani "$input" > tmp.s
  cc -o tmp tmp.s funcall_test.c
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 'main(){0;}'
assert 22 "main(){ 10 + 15 - 3 + 0 ; }"
assert 15 'main(){5 * (9-6);}'
assert 4 'main(){(3+5)/2;}'
assert 5 'main(){22%4+3;}'
assert 30 'main(){+20-(-10);}'
assert 0 'main(){(1+4*5) >= 20 == 0;}'

assert 9 'main(){a = 1;b = a * 3;a = b * 3;}'
assert 5 'main(){a_343_d = 5;a_343_d;}'
assert 3 'main(){___a___ = 2;___a___ + 1;}'

assert 14 'main(){returner = 3; b = 5 * 6 - 8; return returner + b / 2;}'

assert 2 'main(){if(a = 10) return 1 + 1;}'
assert 64 'main(){i = 1;while(i < 50){i = i*2;} return i;}'
assert 34 'main(){s=0;m=1;for(i = 1;i <= 4;i=i+1){s=s+i;m=m*i;} return s+m;}'

assert 11 'main(){foo(34, 23);}'

assert 5 'main(){return sub()+2;} sub(){return 3;}'
assert 5 'main(){return sub(3)+2;} sub(x){return x;}'

echo OK