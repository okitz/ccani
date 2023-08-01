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

assert 0 'int main(){0;}'
assert 22 "int main(){ 10 + 15 - 3 + 0 ; }"
assert 15 'int main(){5 * (9-6);}'
assert 4 'int main(){(3+5)/2;}'
assert 5 'int main(){22%4+3;}'
assert 30 'int main(){+20-(-10);}'
assert 0 'int main(){(1+4*5) >= 20 == 0;}'

assert 9 'int main(){int a;int b;a = 1;b = a * 3;a = b * 3;}'

assert 14 'int main(){int returner;int b;returner = 3; b = 5 * 6 - 8; return returner + b / 2;}'

assert 2 'int main(){int a;if(a = 10) return 1 + 1;}'
assert 2 'int main(){int i;i = 1;if(i < 50){i = i*2;}return i;}'
assert 64 'int main(){int i;i = 1;while(i < 50){i = i*2;} return i;}'
assert 34 'int main(){int s;int m;s=0;m=1;int i;for(i = 1;i <= 4;i=i+1){s=s+i;m=m*i;} return s+m;}'

assert 11 'int main(){foo(34, 23);}'

assert 5 'int main(){return sub(3)+2;} int sub(x){return x;}'

# assert 18 'int main(){ return gcd(90, 144); } int gcd(a, b){ if(b == 0)return a; else return gcd(b, a%b); }'

assert 3 'int main(){int x;int y;int z;x = 3; y = 5; z = &y + 8;return *z;}' 

echo OK