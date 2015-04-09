#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include "tokenizer.h"
#include "utils.h"

int status1 = 42; //The answer to life, the universe, and EVERYTHING.
int status2;
pid_t childPid1;
pid_t childPid2;
pid_t shellPid;
int shellFG = 0;
int childFG = 1;
struct linked_List *backgroundProcesses;
char ** tokens;


static void handleSigchld() {
	
	if (WIFSTOPPED(status2)) {
		if(!(*tokens[0] == 'b' && *(tokens[0]+1) == 'g') && !(*tokens[0] == 'f' && *(tokens[0]+1) == 'g'))
		{
		addNode(childPid1, childPid1, 1, &backgroundProcesses, tokens, 0);
		}
		printf("Stopped: %s\n", backgroundProcesses->processName);
		fflush(stdout);

	}
	
	if (WIFCONTINUED(status2)) {
		printf("Resumed: %s\n", backgroundProcesses->processName);
		fflush(stdout);

	}

	if( tcsetpgrp(STDERR_FILENO, shellPid ) <0 ) perror("Setting terminal control to parent failed.");
	sigset_t x;
	
	sigemptyset(&x);
	sigaddset(&x, SIGCHLD);
	sigprocmask(SIG_BLOCK, &x, NULL);
    sigprocmask(SIG_UNBLOCK, &x, NULL);
	signal(SIGCHLD, handleSigchld);
	fflush(stdout);
	return;
}

/**
 * Main program execution
 */
int main( int argc, char *argv[] ) {
	if(tcsetpgrp( STDERR_FILENO, getpid()) < 0) perror("Terminal control restoration in fg failed");
	backgroundProcesses= (struct linked_List*)malloc(sizeof(struct linked_List));

	shellPid = getpid();
	TOKENIZER *tokenizer;
	char string[256] = "";
	char *tok;
	int br;
	int i;
	int numToks;
	char * const * args1;
	char * const * args2;
	int pipeFD[2];
	status1 = 42;

	string[255] = '\0';	   /* ensure that string is always null-terminated */
	printf( "\nGive me a string to parse or quit by pressing ctrl+d or entering q: ");
	fflush(stdout);

	//set signal handling for the parent
	if (signal(SIGINT, SIG_IGN) == SIG_ERR) perror("SIGINT handler failed in parent");
	if (signal(SIGTERM, SIG_IGN) == SIG_ERR) perror("SIGTERM handler failed in parent");
	if (signal(SIGTTOU, SIG_IGN) == SIG_ERR) perror("SIGTTOU handler failed in parent");
	if (signal(SIGTTIN, SIG_IGN) == SIG_ERR) perror("SIGTTIN handler failed in parent");
	if (signal(SIGCHLD, handleSigchld) == SIG_ERR) perror("SIGCHLD handler failed in parent");
	if (signal(SIGTSTP, SIG_IGN) == SIG_ERR) perror("SIGTSTP handler failed in parent");

	
	while ((br = read( STDIN_FILENO, string, 255 )) > 0) {
		int pipeLocation = 0;
		if (br <= 1) continue;
		string[br-1] = '\0';   /* remove trailing \n */
		/* tokenize string */
		tokenizer = init_tokenizer( string );
		i = 0;
		tokens = (char **)malloc(sizeof(char)*1024);
		while( (tok = get_next_token( tokenizer )) != NULL ) {
			tokens[i] = tok;
			i++;
		}
		numToks = i;
			
		/* Quit command handling */
		if ((*tokens[0] == 'q' || *tokens[0] == 'Q') && *(tokens[0]+1) == '\0'){
			printf("Thank you, come again!\n\n");
			break;
		}
		
		/* run input error checking util--if error, skip to next iteration of while loop, do nothing otherwise */
		if(checkInput(tokens, numToks) < 0){
			printf( "\nGive me a string to parse or quit by pressing ctrl+d or entering q: ");
			fflush(stdout);
			continue; // go to next iteration of while loop
		}
		
		/* if cmd is bg */
		if (*tokens[0] == 'b' && *(tokens[0]+1) == 'g') {
			bg(&backgroundProcesses);
			printf("\nhalvolny shell v1.0 # ");
			fflush(stdout);
			continue;
		}
		
		if (*tokens[0] == 'f' && *(tokens[0]+1) == 'g') {
			fg(&backgroundProcesses, &status2);
			printf("\nhalvolny shell v1.0 # ");
			fflush(stdout);
			continue;			
		}
		
		/* Look for a pipe symbol and create pipe accordingly */
		for(i = 0; i < numToks; i++) {
			if (*tokens[i] == '|') {
				pipeLocation = i;
				pipe(pipeFD);
			}
		}
		
		/* Look for an ampersand and set shellFG accordingly */
		if (*tokens[numToks-1] == '&'){
			 shellFG = 1;
			 childFG = 0;
		 }

		childPid1 = fork();
				
		if (childPid1 && childFG) //I am a foreground
		{
			if(tcsetpgrp(STDERR_FILENO,childPid1) <0 ) perror("Setting terminal control to child 1 failed.");
			shellFG = 0;
		}
		else if(childPid1 && !childFG)
		{
			addNode(childPid1, childPid1, 1, &backgroundProcesses, tokens, 0);
			printf("Running: %s", backgroundProcesses->processName);
			shellFG = 1;
		}

		/* childPid is 0 if we're in the child */
		if (!childPid1){
			
			if (setpgid(0,0)<0) perror("Child1 setpgid failed");
			childPid1 = getpid();
			
			if (signal(SIGINT, SIG_DFL) == SIG_ERR) perror("SIGINT handler failed child 1");			
			if (signal(SIGTERM, SIG_DFL)== SIG_ERR) perror("SIGTERM handler failed child 1");
			if (signal(SIGTSTP, SIG_DFL) == SIG_ERR) perror("SIGTSTP handler failed child 1");
			
			//Redirect standard output into pipe intake 
			if (pipeLocation != 0) handlePipeWrite(pipeFD);

			//Handle redirection symbols
			handleRedirection(tokens, 0, numToks);
			
			//Get arguments to command under execution		
			args1 = makeArgsList(tokens, 0, numToks);

			//Replace child process space with specified command
			if( execvp(args1[0], args1 ) < 0 ) {
				perror("exec failed");
				exit(EXIT_FAILURE);
			}
		}
		
		if(pipeLocation != 0){
			/* This sleep is hacky, but it helps prevent race conditions
			 * caused by the first cmd in the pipeline finishing, sending
			 * a SIGCHLD signal, and the waitpid below setting &status1
			 * at the same time.
			 */
			sleep(1);
			waitpid(childPid1, &status1, WUNTRACED);
			childPid2 = fork();
			
			/* if we're in the parent and the child is to be in the foreground,
			 * give the child terminal control */
			if (childPid2 && childFG) {
				if(tcsetpgrp(STDERR_FILENO,childPid2) <0 ) perror("Setting terminal control to child 2 failed.");
				shellFG = 0;
			}

			else if(childPid2 && !childFG){
				addNode(childPid2, childPid2, 1, &backgroundProcesses, tokens, pipeLocation+1);
				printf("Running: %s", backgroundProcesses->processName);
				shellFG = 1;
			}

			if (!childPid2) {  // in child2	
				setpgid(0,0);
				childPid2 = getpid();
				
				if (signal(SIGINT, SIG_DFL) == SIG_ERR) perror("SIGINT handler failed child 2");			
				if (signal(SIGTERM, SIG_DFL)== SIG_ERR) perror("SIGTERM handler failed child 2");
				if (signal(SIGTSTP, SIG_DFL) == SIG_ERR) perror("SIGTSTP handler failed child 2");
			
				
				//Get argument to command after pipe
				args2 = makeArgsList(tokens, pipeLocation+1, numToks);
		
				//Redirect pipe output into standard input
				handlePipeRead(pipeFD);
				
				//Deal with redirection symbols
				handleRedirection(tokens, pipeLocation, numToks);
				
				//Replace the child 2 process space with the command following the pipe
				if( execvp(args2[0], args2 ) < 0 ) {
					perror("exec failed");
					exit(EXIT_FAILURE);
				}
			}
			
			if(close(pipeFD[0])< 0) perror("closing pipe read end in parent failed");
			if(close(pipeFD[1])< 0) perror("closing pipe write end in parent failed");
		}

		/* If shellFG is true, we don't wait to wait for the child process */
		if (!shellFG) {
			if (waitpid(-1, &status2, WUNTRACED) < 0) perror("Wait not executing in parent");
			if( tcsetpgrp(STDERR_FILENO,getpid() ) <0 ) perror("Setting terminal control to parent failed.");			
			
		}
				
		shellFG = 0;
		childFG = 1;
		sleep(1);
		printf( "\nhalvolny shell v1.0 # ");
		fflush(stdout);
	} //end of shell while loop

	
	for(i=0; i<numToks; i++){
		free(tokens[i]);
	}
	free(tokens);
	free_tokenizer(tokenizer);	
	
	struct linked_List *iterator;
	
	while(backgroundProcesses != NULL) {
		iterator = backgroundProcesses;
		backgroundProcesses = backgroundProcesses->next;
		free(iterator);
	}
	
	return 0;			/* all's well that ends well */
}
