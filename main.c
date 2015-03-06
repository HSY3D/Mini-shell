#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>

#define MAX_LINE 80 /* 80 chars per line, per command, should be enough. */
#define MAX_HISTORY 10
#define MAX_JOBS 10

int historyCounter = -1;
char history[MAX_HISTORY][MAX_LINE];

int jobs[MAX_JOBS];
int jobsCount = 0;

int status;
int pid;

void setup(char inputBuffer[], char *args[], int *background);
void addToHistory (char command[MAX_LINE]);

/**
 * setup() reads in the next command line, separating it into distinct
 * tokens using whitespace (space or tab) as delimiters. setup() sets
 * the args parameter as a null-terminated string.
 */
void setup(char inputBuffer[], char *args[], int *background)
{
    int length, /* # of characters in the command line */
    i,        /* loop index for accessing inputBuffer array */
    start,    /* index where beginning of next command parameter is */
    ct;       /* index of where to place the next parameter into args[] */
    
    ct = 0;
    
    /* read what the user enters on the command line */
    length = read(STDIN_FILENO, inputBuffer, MAX_LINE);
    
    start = -1;
    if (length == 0) {
        /* ctrl-d was entered, quit the shell normally */
        printf("\n");
        exit(0);
    }
    
    if (length < 0) {
        /* somthing wrong; terminate with error code of -1 */
        perror("Reading the command");
        exit(-1);
    }
    
    /* examine every character in the inputBuffer */
    for (i = 0; i < length; i++) {
        
        switch (inputBuffer[i]) {
                
            case ' ':
                
            case '\t':               /* argument separators */
                
                if(start != -1){
                    args[ct] = &inputBuffer[start];    /* set up pointer */
                    ct++;
                }
                
                inputBuffer[i] = '\0'; /* add a null char; make a C string */
                start = -1;
                break;
                
            case '\n':                 /* should be the final char examined */
                
                if (start != -1){
                    args[ct] = &inputBuffer[start];
                    ct++;
                }
                
                inputBuffer[i] = '\0';
                args[ct] = NULL; /* no more arguments to this command */
                break;
                
            default :
                /* some other character */
                if (inputBuffer[i] == '&'){
                    *background  = 1;
                    inputBuffer[i] = '\0';
                } else if (start == -1)
                    start = i;
        }
    }
    args[ct] = NULL; /* just in case the input line was > MAX_LINE */
}

int main (void)
{
    char inputBuffer[MAX_LINE];
    int background;
    char *args[MAX_LINE/+1];
    
    //Init history array to empty strings
    int i;
    for (int i = 0; i < MAX_HISTORY; ++i)
    {
        strcpy(history[i],"");
    }
    historyCounter++;

    for (i = 0; i < MAX_JOBS; ++i)
    {
        jobs[i] = -1;
    }

    int cdCheck;
    int execvpFail;
    int execvpCheck;
    int jobsCheck;
    int fgPID;

    while (1)
    {
        background = 0;
        printf(" COMMAND-> \n");
        setup(inputBuffer, args, &background);

        char* cwd;
        char buff[PATH_MAX + 1];
        char tmpString[MAX_LINE];
        tmpString[0] = 0;

        cdCheck = 1;
        execvpFail = 0;
        execvpCheck = 0;
        jobsCheck = 0;
        fgPID = -1;

        if (strcmp(args[0], "history") != 0)
        {
            if (strcmp(args[0], "cd") == 0)
            {
                cdCheck = chdir(args[1]);
                if (cdCheck == -1)
                {
                    printf("Unsuccessful.\n");
                }
                else 
                {
                    printf("Successful.\n");
                }
            } 
            else if (strcmp(args[0], "pwd") == 0)
            {
                cwd = getcwd(buff, PATH_MAX + 1);
                if (cwd != NULL)
                {
                    printf("%s\n", cwd);
                }
            } 
            else if (strcmp(args[0], "exit") == 0)
            {
                exit(1);
            }   
            else if (strcmp(args[0], "fg") == 0)
            {
                int i;
                if (args[1] == NULL)
                {
                    printf("Invalid arguements.\n");
                }
                else
                {
                    for (i = 0; i < MAX_JOBS; ++i)
                    {
                        if (jobs[i] == atoi(args[1]))
                        {
                            fgPID = jobs[i];

                            int tmpPID = 0;
                            tmpPID = jobs[i];
                            jobs[i] = jobs[jobsCount - 1];
                            jobs[jobsCount - 1] = tmpPID;
                            // jobs[jobsCount - 1] = -1;

                            jobsCount--;
                            int wait;
                            wait = waitpid(fgPID, &status, WUNTRACED);
                            break;
                        }
                    }
                }
            }
            else if (strcmp(args[0], "jobs") == 0)
            {
                jobsCheck = 1;
                int i;
                for (i = 0; i < jobsCount; ++i)
                {
                    printf("%d\n", jobs[i]);
                }
            }
           
            int i;
            for (int i = 0; args[i] != NULL; ++i)
            {
                strcat(tmpString, args[i]);
                strcat(tmpString, " ");
            }
            
            addToHistory(tmpString);
            tmpString[0] = 0;

            if (strcmp(args[0], "fg") != 0)
            {
                pid = fork();
                if (pid < 0)
                {
                    printf("fork pid = %d\n", pid);
                    exit(2);
                } 

                if (pid == 0)
                {
                    if (strcmp(args[0], "jobs") == 0 && jobsCheck == 0)
                    {
                        int i;
                        for (i = 0; i < jobsCount; ++i)
                        {
                            printf("%d\n", jobs[i]);
                        }

                        exit(1);
                    }
                    else if (cdCheck == 1 && jobsCount == 0)
                    {
                        execvp(args[0], args);
                        execvpFail = 1;
                    }
                }
                else if (background == 0)
                {
                    waitpid(pid, &status, WUNTRACED);
                    if (WIFSTOPPED(status) == 0)
                    {
                        kill(pid, SIGCONT);
                    }
                }
                else
                {
                    jobs[jobsCount] = pid;
                    jobsCount++;
                }
            }
        }
        else if (strcmp(args[0], "history") == 0)
        {
            if (strcmp(args[1], "r") == 0)
            {
                int i;
                for (i = 0; i < 10; ++i)
                {
                    printf("%d %s\n", i, history[i]);
                }
                if (args[2] == NULL)
                {
                    if (historyCounter == 0)
                    {
                        printf("This is the first command.\n");
                        addToHistory(tmpString);
                    }
                    else if (historyCounter < 10)
                    {
                        addToHistory(history[historyCounter - 1]);
                        printf("%s\n", history[historyCounter - 1]);
                    }
                    else
                    {
                        addToHistory(history[9]);
                        printf("%s\n", history[9]);
                    }
                }
                else
                {
                    int i;
                    char cmdChar = args[2][0];
                    // printf("%c\n", cmd);
                    if (historyCounter <= 10)
                    {
                        for (i = historyCounter - 1; i >= 0; --i)
                        {
                            if (history[i][0] == cmdChar)
                            {
                                printf("%s\n", history[i]);
                                i = -1; //break
                            }
                        }
                    } 
                    else
                    {
                        int i;
                        for (i = 9; i >= 0; --i)
                        {
                            if (history[i][0] == cmdChar)
                            {
                                printf("%s\n", history[i]);
                                i = -1; //break
                            }
                        }
                    }
                }
            }
        }
    }
}

void addToHistory (char tmpString[MAX_LINE]) 
{
    int i;
    if (historyCounter < 10) {
        strcpy (history[historyCounter], tmpString);
        historyCounter++;
    }
    else
    {
        for (int i = 0; i < 10; ++i)
        {
            strcpy(history[i], history[i+1]);
        }

        strcpy(history[9], tmpString);
        historyCounter++;
    }
}

