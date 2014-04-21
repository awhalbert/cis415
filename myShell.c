#include <unistd.h>
#include <wait.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

enum { STDIN, STDOUT, STDERR };

/* Here we define all our messages to make the write() calls simpler */
char usageMsg[] = "Correct usage: ./ozai integer\n";
char killMsg[] = "You cannot escape me... I am the Phoenix King!\n";
char noKillMsg[] = "But, how? What--what did you do to me?!\n";
/* c will be used to iterate over these strings in the write() calls */
char *c;

void handler (int signum) {
    for (c = killMsg; *c; ++c) write( STDOUT, c, 1); 
    kill(getpid(), SIGKILL);
}

int main (int argc, char *argv[], char *argp[]) {
    signal(SIGALRM, handler);
    
    if (argc != 2) {
        for (c = usageMsg; *c; ++c) {
            write( STDOUT, c, 1);
        }
        kill(getpid(), SIGKILL);
        //write ( STDOUT, "Correct usage: \n", 18);	
    }	
	
    int timeout = atoi(argv[1]);
    printf("Timeout: %d\n", timeout);
    int BUFSIZE = 1024;
    int cmd_len;
    int i;
    char buffer[BUFSIZE];
    int status;
    while (1) {
        write ( STDOUT, "Fire Lord Ozai# ", 16 );

        cmd_len = read ( STDIN, buffer, BUFSIZE );
        char cmd[cmd_len-1];
        i = 0;
        // populate char array cmd with contents of buffer
        for (i; i < cmd_len-1; ++i) {
                cmd[i] = buffer[i];
        }
        cmd[cmd_len-1] = '\0';

        pid_t pid;
        pid = fork();
        if (!pid){
            alarm(0);
            char * argvx[] = { cmd, NULL };
            signal(SIGALRM, handler);
            execve(cmd, argvx, argp);
        }
        else {
            alarm(timeout);
            int n = waitpid(pid, &status, 0);
            alarm(0);
            for (c = noKillMsg; *c; ++c) write( STDOUT, c, 1);
        }
    }
}
