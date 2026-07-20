#include <stdio.h>
#include <unistd.h>

int main() {
	int shared_var = 42;

	if(fork() ==0) {
		shared_var = 100;
		printf("Child: modified shared_var = %d\n", shared_var);
	} else {
		sleep(1);
		printf("Parent: shared_var = %d\n", shared_var);
	}
	return 0;
}
