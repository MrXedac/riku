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

int main(int argc, char** argv)
{
    printf("Hello world!\n");
    printf("how many fibonacci iterations? ");
    int nbr;
    int ret = scanf("%d", &nbr);
    if(ret > 0) {
    	printf("doing some Fibonacci (n=%d)\n", nbr);
    	fibonacci(nbr);
    } else {
    	printf("invalid number, doing n=10\n");
	    fibonacci(10);
    }
    
    return 5;
}
