#include <unistd.h>
#include <wait.h>
#include <signal.h>

enum { STDIN, STDOUT, STDERR };

void handler (int signum) {
	write(1, "I am the Phoenix King!\n", 24);
	kill(getpid(), SIGKILL);
}

int main (int argc, char *argv[], char *argp[]) {
	if (argc != 2) {
		write ( STDOUT, "Incorrect usage...\n", 18);
		kill(getpid(), SIGKILL);
	}
	
	
	short BUFSIZE = 1024;
	short cmd_len;
	short c;
	char buffer[BUFSIZE];
	while (1) {
		write ( STDOUT, "Fire Lord Ozai# ", 16 );
		
		cmd_len = read ( STDIN, buffer, BUFSIZE );
		char cmd[cmd_len-1];
		c = 0;
		// populate char array cmd with contents of buffer
		for (c; c < cmd_len-1; ++c) {
			cmd[c] = buffer[c];
		}
		pid_t pid;
		pid = fork();
		signal(SIGALRM, handler);

		if (!pid) {
			char * argvx[] = { cmd, NULL };
			alarm(1);
			execve (cmd, argvx, argp);
			// cancel alarm if exec fails
			alarm(0);
		}
		else {
			int n = wait(NULL);
		}
	}
}
