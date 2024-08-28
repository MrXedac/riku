#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void fibonacci(int n)
{
  int i;
  int t1 = 0, t2 = 1;
  int nextTerm = t1 + t2;

  printf("fib(%d)=%d, %d, ", n, t1, t2);

  // print 3rd to nth terms
  for (i = 3; i <= n; ++i) {
    printf("%d, ", nextTerm);
    t1 = t2;
    t2 = nextTerm;
    nextTerm = t1 + t2;
  }

  printf("\n");
}

void main()
{
    printf("Hello world!\n");
    printf("Doing some Fibonacci (n=10)\n");
    fibonacci(10);
    exit();
}
