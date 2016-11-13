#include <stdio.h>
#include <unistd.h>
#include "rntest.h"

int main(int argc, char **argv)
{
	while (1) {
		fn1();
		sleep(2);

		fn2();
		sleep(2);
	}
  
	return 0;
}