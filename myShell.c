#include <unistd.h>
#include <wait.h>
#include <signal.h>
#include <stdlib.h>

enum { STDIN, STDOUT, STDERR };
/* c will be used to iterate over these strings in the write() calls */
char *c;
pid_t childPid;

void print ( char * c, char *msg ) {
    /*
     * Prints the content of the character array
     * 
     * Arguments:
     *  c, a character pointer used for iteration through msg
     *  msg, a string to print
     */
    for (c = msg; *c; ++c) write( STDOUT, c, 1);
}

void handler (int signum) {
    print(c, "You cannot escape me... I am the Phoenix King!\n");
    kill(childPid, SIGKILL);
}

int main (int argc, char *argv[], char *argp[]) {
    signal(SIGALRM, handler);
    
    if (argc != 2) {
        print( c, "Correct usage: ./ozai integer\n" );
        kill(getpid(), SIGKILL);
        //write ( STDOUT, "Correct usage: \n", 18);	
    }	
	
    int timeout = atoi(argv[1]);
    timeout = 1;
    int BUFSIZE = 1024;
    int cmd_len;
    int i;
    char buffer[BUFSIZE];
    int status;
    while (1) {
        print( c, "Fire Lord Ozai# " );

        cmd_len = read ( STDIN, buffer, BUFSIZE );
        char cmd[cmd_len-1];
        i = 0;
        // populate char array cmd with contents of buffer
        for (i; i < cmd_len-1; ++i) {
                cmd[i] = buffer[i];
        }
        cmd[cmd_len-1] = '\0';

        childPid = fork();
        /* childPid is 0 if we're in the child */
        if (!childPid){
            alarm(0);
            char * argvx[] = { cmd, NULL };
            signal(SIGALRM, handler);
            int fail = execve(cmd, argvx, argp);
            if (fail) {           
                print( c, "Command not found...\n" );
                kill( getpid(), SIGKILL );
            }
        }
        /* childPid holds the child's pid if we're in the parent */
        else {
            int sig = alarm(timeout);
            int n = waitpid(childPid, &status, 0);
//            print( c, "But, how? What--what did you do to me?!\n" );
            alarm(0);
        }
    }
}
