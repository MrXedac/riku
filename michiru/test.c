#include <stdio.h>

int main(int argc, char** argv)
{
	char buffer[64];
	printf("enter something>");
	scanf("%s", buffer);
	printf("you typed: %s\n", buffer);
	return 0;
}
