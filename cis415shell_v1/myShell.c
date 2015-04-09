/*
 * Author: Aaron W. Halbert
 * Prof. Kevin Butler
 * CIS 415
 * 24 April 2014
 */

#include <unistd.h>
#include <wait.h>
#include <signal.h>
#include <ctype.h>

enum { STDIN, STDOUT, STDERR };
/* c will be used to iterate over these strings in the write() calls */
char *c;

pid_t childPid;
// chars are 4 bytes on most system, so this allows for 1024 bytes
int BUFSIZE = 256;

int atoi (char * string) {
	/*
	 * Returns an integer interpreted from a string.
	 * Used http://www.geeksforgeeks.org/write-your-own-atoi/
	 * to inform my method.
	 */
	int integer = 0;
	int i = 0;
	for (i; string[i] != '\0'; ++i) {
		integer = 10 * integer + (string[i] - '0');
	}
	return integer;
}

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
    /*
     * Handles the signal sent by the parent process if the child times out
     * by killing the child process after printing a spiteful message
     */
    print(c, "You cannot escape me... I am the Phoenix King!\n");
    kill(childPid, SIGKILL);
}

int main (int argc, char *argv[], char *argp[]) {
    signal(SIGALRM, handler);

    if (argc != 2 || (argc == 2 && isalpha(*argv[1]))) {
        print( c, "Correct usage: ./ozai integer\n" );
        kill(getpid(), SIGKILL);
    }
    int timeout = atoi(argv[1]);
    int len_input;
    int i; // used for iteration
    char buffer[BUFSIZE];
    int status; // used to hold result of alarm in parent after fork
    signal(SIGALRM, handler); // connect alarm to handler

    while (1) {
        print( c, "Fire Lord Ozai# " );
		
        len_input = read ( STDIN, buffer, BUFSIZE );
        if (len_input == BUFSIZE) {
			print( c, "Command too long!\n" );
			continue;
        }
        
        /* kill the shell if the user enters 'q' by itself */
        if (len_input == 2 && buffer[0] == 'q') {
            print ( c, "What... what did you do to me?!\n" );
            kill(getpid(), SIGKILL);
        }
        

        char cmd[] = "";
        char * argv = "";	
        i = 0;
        // populate char array cmd with contents of buffer
        for (i; i < len_input-1; ++i) cmd[i] = buffer[i];
        cmd[len_input-1] = '\0';
        
        childPid = fork();
        /* childPid is 0 if we're in the child */
        if (!childPid){
            alarm(0);
            char * argv[] = {cmd, NULL};
            int fail = execve(cmd, argv, argp);
            if (fail) {           
                print( c, "Command not found...\n" );
                kill( getpid(), SIGKILL );
            }
        }
        /* childPid holds the child's pid if we're in the parent */
        else {
            status = alarm(timeout);
            waitpid(childPid, &status, 0);
            if (WIFEXITED(status)) {
				print( c,"But, how did you escape? ");
				print( c, "What--what did you do to me?!\n" );
			}
            alarm(0);
        }
    }
}
