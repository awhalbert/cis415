#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>


struct linked_List
{
    pid_t ID;
    int jobGroup;
    int isBG;
    char * processName;
    struct linked_List *next;
};

void makeLinkedList(pid_t ID, int jobGroup, int isBG, struct linked_List **top, char ** tokens)
{
    struct linked_List *ptr = (struct linked_List*)malloc(sizeof(struct linked_List));
    if(ptr == NULL)
    {
        printf("\n Linked list creation failed \n");
        return;
    }
    
    ptr->ID = ID;
    ptr->jobGroup = jobGroup;
    ptr->processName = tokens[0];
    ptr->isBG = isBG;
    ptr->next = NULL;
    *top = ptr;
    
}

void addNode(pid_t ID, int jobGroup, int isBG, struct linked_List **top, char **tokens, int cmdLoc)
{
    if(*top == NULL)
    {
        return (makeLinkedList(ID, jobGroup, isBG, top, tokens));
    }

     struct linked_List *ptr = (struct linked_List*)malloc(sizeof(struct linked_List));
    
    
    if(ptr == NULL)
    {
        printf("\n Process node creation failed \n");
        return;
    } 	
    
    ptr->ID = ID;
    ptr->jobGroup = jobGroup;
    ptr->processName = tokens[cmdLoc];
    ptr->next = *top;
    ptr->isBG = isBG;
	*top = ptr;
	fflush(stdout);
}

void deleteTopNode(struct linked_List **top) {
	/* Delete the top node only if in the background */
	*top = (*top)->next;
}

void printTopID(struct linked_List* top) {
	printf("Node top of linked list: %d\n", (int)top->ID);
	printf("Name of process is %s\n", top->processName);
}


int checkInput(char ** tokens, int numToks){
	int irCount=0;	//input redirection count
	int orCount=0;	//output redirection count
	int pipeCount=0;	//pipe symbol count
	int i;
	
	
	for(i = 0; i < numToks; i++) {		
		//Error checking
		if (*tokens[i] == '>') {
			if ( i == 0 || i == numToks-1 ) {
				printf("\">\" cannot be the first or last symbol in a command\n");
				return -1;
			}
			else orCount++;
		}
		
		else if (*tokens[i] == '<') {
			if ( i == 0 || i == numToks-1 ) {
				printf("\"<\" cannot be the first or last symbol in a command\n");
				return -1;
			}
			else irCount++;
		}
		
		else if (*tokens[i] == '|') {
			if ( i == 0 || i == numToks-1 ) {
				printf("Pipe cannot be the first or last symbol in a command\n");
				return -1;
			}
			else pipeCount++;
		}	
		
		//Error handling
		if(orCount > 1){
			printf("Conflicting input redirection commands- only 1 '>' symbol should be used\n");
			return -1;
		}
		else if(irCount > 1){
			printf("Conflicting input redirection commands- only 1 '<' symbol should be used\n");
			return -1;
		}
		else if(pipeCount>1){
			printf("Multiple pipes not permitted- only one '|' symbol should be used.\n");
			return -1;
		}
		else if (*tokens[i] == '&' && i != numToks-1){
			printf("Ampersand character should only be placed at the end of an input commands series.\n");
			return -1;		
		}		
	}
	return 0;
}

char * const * makeArgsList(char **tokens, int startIndex, int numToks){
    int i = startIndex;
    int j = 0;
    char ** args = malloc(sizeof(char)*1024);
    while(i < numToks && *tokens[i] != '<' && *tokens[i] != '>' && *tokens[i] != '|' && *tokens[i] != '&') {
    	args[j] = tokens[i];
    	i++;
    	j++;
    }
    args[i] = NULL;

	return args;
}

void handleRedirection(char ** tokens, int startIndex, int numToks) {
	int i, ofd, ifd;
	for(i = startIndex; i < numToks; i++) {
		if (*tokens[i] == '>') {	
			// output redirection/
			ofd = open((const char *)tokens[i+1], O_WRONLY | O_CREAT, 0644);
			if (ofd < 0) perror("output open failed");
			if ( dup2(ofd, STDOUT_FILENO) < 0 ) perror("output dup failed");
			if ( close(ofd) < 0 ) perror("output close failed");
		}
			
		else if (*tokens[i] == '<') {
			// input redirection/	
			ifd = open((const char *)tokens[i+1], O_RDONLY);
			if (ifd < 0) perror("input open failed");
			if ( dup2(ifd, STDIN_FILENO) < 0 ) perror("input dup failed");
			if ( close(ifd) < 0 ) perror("input close failed");
		}
		
	}
	return;
}

void handlePipeRead(int * pipeFD) {
	if(close(pipeFD[1]) < 0){
		perror("pipe write file descriptor close failed in child 2 ");
		exit(EXIT_FAILURE);
	}
	if (dup2(pipeFD[0], STDIN_FILENO) < 0){
		perror("pipe dup of read file descriptor to STDIN failed ");
		exit(EXIT_FAILURE);
	}
	if(close(pipeFD[0]) < 0){
		perror("pipe read file descriptor close failed in child 2 ");
		exit(EXIT_FAILURE);
	}
	return;
}

void handlePipeWrite(int * pipeFD) {			
	if(close(pipeFD[0]) < 0){
		perror("pipe read file descriptor close failed in child 1");
		exit(EXIT_FAILURE);
	}
	if (dup2(pipeFD[1], STDOUT_FILENO) < 0) {
		perror("pipe dup of write file descriptor to STDOUT failed");
		exit(EXIT_FAILURE);
	}
	if(close(pipeFD[1]) < 0){
		perror("pipe write file descriptor close failed in child 1");
		exit(EXIT_FAILURE);
	}
	return;	
}

void bg(struct linked_List **top){
	
	while(setpgid((*top)->ID, (*top)->ID) <-1 ) deleteTopNode(top);
	
	if ((*top)->ID > 0) {
		printf("Restarting process %s\n",(*top)->processName);
		if (kill((*top)->ID, SIGCONT) < 0) perror("Sending SIGCONT in bg failed");
	}
	else printf("No background process to restart.\n");
	fflush(stdout);
}

int fg(struct linked_List **top, int * status){
	/**
	 * Returns 0 on success, -1 on failure
	 */
	 
	 while(setpgid((*top)->ID, (*top)->ID) <-1 ) deleteTopNode(top);
	 
	if ((*top)->ID > 0) {
		if (kill((*top)->ID, SIGCONT) < 0) perror("Sending SIGCONT in fg failed");
		if(tcsetpgrp( STDERR_FILENO, (*top)->ID) < 0) perror("Terminal control restoration in fg failed");
		sleep(1);
		printf("Restarting process %s\n",(*top)->processName);
		waitpid((*top)->ID, status, WUNTRACED);
		deleteTopNode(top);
		status = 0;
		return 0;
	}
	else {
		printf("No process to bring into fg\n");
		return -1;
	}
}
