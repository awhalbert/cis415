#ifndef UTILS_H
#define UTILS_H

struct linked_List
{
    pid_t ID;
    int jobGroup;
    int status;
    char * processName;
    struct linked_List *next;
};

void makeLinkedList(pid_t ID, int jobGroup, int isBG, struct linked_List **top, char ** tokens);
void addNode(pid_t ID, int jobGroup, int isBG, struct linked_List **top, char ** tokens, int cmdLoc);
void deleteTopNode(struct linked_List **top);
void printTopID (struct linked_List *top);


int checkInput(char **tokens, int numToks);

char * const * makeArgsList(char **tokens, int startIndex, int numToks);
void handleRedirection(char **tokens, int startIndex, int numToks);
void handlePipeRead(int *pipeFD);
void handlePipeWrite(int *pipeFD);

void bg(struct linked_List **top);
int fg(struct linked_List **top, int * status);


#endif
