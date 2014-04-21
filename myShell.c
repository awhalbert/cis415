#include <unistd.h>
#include <wait.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

enum { STDIN, STDOUT, STDERR };

char usgPointer[] = "Correct usage: ./ozai integer\n";

void handler (int signum) {
	write( STDOUT, "Your process burns... I am the Phoenix King!\n", 46);
	kill(getpid(), SIGKILL);
}

int main (int argc, char *argv[]) {
    if (argc != 2) {
        char *c;
        for (c = usgPointer; *c; c++) {
            write( STDOUT, c, 1);
        }
        kill(getpid(), SIGKILL);
        //write ( STDOUT, "Correct usage: \n", 18);	
    }	
	
    int timeout = atoi(argv[1]);
    printf("Timeout: %d\n", timeout);
    int BUFSIZE = 1024;
    int cmd_len;
    int c;
    char buffer[BUFSIZE];
    int status;
    while (1) {
        write ( STDOUT, "Fire Lord Ozai# ", 16 );

        cmd_len = read ( STDIN, buffer, BUFSIZE );
        char cmd[cmd_len-1];
        c = 0;
        // populate char array cmd with contents of buffer
        for (c; c < cmd_len-1; ++c) {
                cmd[c] = buffer[c];
        }
        cmd[cmd_len-1] = '\0';

        pid_t pid;
        pid = fork();
        
        if (!pid) {
                char * argvx[] = { cmd, NULL };
                char * argpx[] = {NULL};
                write(1, cmd, cmd_len);
                write(1, "\n", 1);
                signal(SIGALRM, handler);
                alarm(1);
//                sleep(2);
                execve(cmd, argvx, argpx);
                // cancel alarm if exec fails
                perror("exec failed\n");
                alarm(0);
        }
        else {
            int n = wait(&status);
        }
    }
}
