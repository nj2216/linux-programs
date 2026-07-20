#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main() {
	pid_t pid;

	printf("Before fork (Printed once by Parent)\n");

	pid = fork();

	if (pid < 0) {
		perror("Fork Failed");
		return 1;
	}
	else if (pid == 0) {
		printf("I am the CHILD. My PID is %d, my Parent's PID is %d\n", getpid(),getppid());
	}
	else {
		printf("I am the PARENT. My PID is %d, my Child's PID is %d\n", getpid(), pid);
	}

	printf("After fork (printed once by each process)\n");
	return 0;
}
