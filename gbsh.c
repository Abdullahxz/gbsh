#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <signal.h>
#include <pwd.h>

void printEnv ();   // Print environment variables

int changeCurrentDirectory (char* dir[]);   // To change current directory

int listCurrentDirectory(char* dir);    // To print list of files in current directory

char** parseCommand (char input[],int* tokenCount);    // To parse input

char*** getCommands(char* arr[],int n);    // To parse Commands

void childSignalHandler(int signum);    // Child signal handler

int checkBackground(char* arr);    // Check if user asked to run program in packground

int checkPipe(char* arr);   // Check whether the user used pipe

int checkRedirection(char *str);    // Check whether the user asked for redirection

int checkPipeRedirection(char **str);   // Check if a piped command has redirection

int redirHandler(char* input[], char* inputFile, char* outputFile, int state);    //Generic function to handle redirection

int getIndex (char* arr[],char* x);    // Utility function to get index of any command

int pipeImplementer (char **input, int *noPipes, int i);    // A recursive function to implement pipes

int checkLs(char* arr); // Utility for ls command 

int main(int argc, char *argv[]) {
    
    int saveStdOut;
    
    //Setting up shell environment variable
    char* p=getenv("PWD");
    char* spath = malloc(strlen(p) + 6);
    strcat(spath,p);
    strcat(spath,"/gbsh");    
    setenv("SHELL",spath,1);
    
    //Setting signals
    signal(SIGCHLD, childSignalHandler);
    signal(SIGINT, SIG_IGN);
    
    char input[200];
    char host[50];
    char cpath[150];
    
    do {
        //Printing prompt
        char *userName=getenv("USER");
        gethostname(host,sizeof(host));
        getcwd(cpath,sizeof(cpath));
        fprintf(stdout, "%s@%s:%s$ ",userName,host,cpath);
        
        //Getting input
        fgets(input, sizeof(input), stdin);
        while (strcmp(input,"\n") == 0) {
            fprintf(stdout, "%s@%s:%s$ ",userName,host,cpath);
            fgets(input, sizeof(input), stdin);
        }
        char inCheck[sizeof(input) + 1];
        strcpy(inCheck,input);
        
        //Tokenizing
        int inputLen = strlen(input);
        if (input[inputLen-1] == '\n')
            input[inputLen-1]='\0';
        int tokenNumber=0;
        char** input_2D;
        input_2D=parseCommand(input,&tokenNumber);
        
        //Exit implementation
        if (strcmp(input_2D[0],"exit") == 0 && input_2D[1] == NULL) {
            exit(0);
        }
        
        //pwd implementation
        else if (strcmp(input_2D[0],"pwd") == 0 && input_2D[1] == NULL) {
            char temppath[150];
            getcwd(temppath,sizeof(temppath));
            fprintf(stdout, "%s\n",temppath);
        }
        
        //pwd implementation with output redirection
        else if (strcmp(input_2D[0],"pwd") == 0 && input_2D[1] != NULL) {
            if (strcmp(input_2D[1], ">") == 0 && input_2D[2] != NULL) {
                mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
                int fd = open(input_2D[2], O_WRONLY | O_CREAT | O_TRUNC,mode);
                saveStdOut = dup(1);
                dup2(fd,1);
                close(fd);
                
                char temppath[150];
                getcwd(temppath,sizeof(temppath));
                fprintf(stdout, "%s\n",temppath);
            
                dup2(saveStdOut,1); //Restoring stdout for further use
            }
            else
                fprintf(stdout,"Kindly check your command.\n");
        }
        
        //clear implementation
        else if(strcmp(input_2D[0],"clear") == 0 && input_2D[1] == NULL) {
            fprintf(stdout,"\033[H\033[J");
        }
        
        //cd implementation
        else if (strcmp(input_2D[0],"cd") == 0) {
            int ret = changeCurrentDirectory(input_2D);
            if (ret == 0)
                fprintf(stdout, "Directory successfully changed.\n");
            else if (ret == -1)
                fprintf(stdout, "The Directory entered is not found.\n");
            else
                fprintf(stdout, "Directory successfully changed to HOME.\n");
        }
        
        //ls implementation
        else if (strcmp(input_2D[0],"ls") == 0 && input_2D[1] == NULL) {
            int ret = listCurrentDirectory(cpath);
            if (ret == -1)
                fprintf(stdout,"Can not locate files.\n");
            
        }

        else if (strcmp(input_2D[0],"ls") == 0 && input_2D[1] != NULL && strcmp(input_2D[2], ">") != 0 &&
                 strcmp(input_2D[1], ">") != 0 && checkLs(input_2D[1]) != 1) {
            int ret = listCurrentDirectory(input_2D[1]);
            if (ret == -1)
                fprintf(stdout,"Can not locate files.\n");
            
        }
        
        //ls implementation with output redirection
        else if (strcmp(input_2D[0],"ls") == 0 && (strcmp(input_2D[1], ">") == 0 || strcmp(input_2D[2], ">") == 0)) {
            
            mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
            int fd = open(input_2D[2], O_WRONLY | O_CREAT | O_TRUNC,mode);
            saveStdOut = dup(1);
            dup2(fd,1);
            close(fd);
            if (strcmp(input_2D[1], ">") == 0) {
                int ret = listCurrentDirectory(cpath);
                if (ret == -1)
                    fprintf(stdout,"Can not locate files.\n");
            }
            else {
                int ret = listCurrentDirectory(input_2D[3]);
                if (ret == -1)
                    fprintf(stdout,"Can not locate files.\n");
            }
            dup2(saveStdOut,1); //Restoring stdout for further use

            fprintf(stdout,"Kindly check your command.\n");
        }
        
        //setenv implementation
        else if (strcmp(input_2D[0],"setenv") == 0) {
            int ret;
            
            //checking if a variable already exists
            if (getenv(input_2D[1]) == NULL)
                fprintf(stdout, "A new environment variable created.\n");
            else
                fprintf(stdout, "Updating an already existing environment variable.\n");
                
            if (*input_2D[2] == '\0')
                ret = setenv(input_2D[1],"",1);
            else
                ret = setenv(input_2D[1],input_2D[2],1);

            if (ret == 0) 
                 fprintf(stdout, "Environment variable successfully updated.\n");
            else
                 fprintf(stdout, "There was an error changing environment variables.\n");
        }
        
        //unsetenv implementation
        else if (strcmp(input_2D[0],"unsetenv") == 0) {
            if (getenv(input_2D[1]) == NULL)
                fprintf(stdout, "Environment variable does not exist.\n");
            else {
                int ret = unsetenv(input_2D[1]);
                if (ret == 0) 
                     fprintf(stdout, "Environment variable successfully undefined.\n");
                else
                     fprintf(stdout, "There was an error changing environment variables.\n");
            }
        }
        
        //environ implementation
        else if (strcmp(input_2D[0],"environ") == 0) {
            if (strcmp(input_2D[1], ">") == 0 && input_2D[2] != NULL) {
                mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
                int fd = open(input_2D[2], O_WRONLY | O_CREAT | O_TRUNC,mode);
                saveStdOut = dup(1);
                dup2(fd,1);
                close(fd);
                
                printEnv();
            
                dup2(saveStdOut,1); //Restoring stdout for further use
            }
            else
                printEnv();
        }
        
        //Everything else
        else {
            int numPipes = checkPipe(inCheck);
            int redir = checkRedirection(inCheck);
            
            pid_t pid;
            pid = fork();
            if (pid == -1)
                fprintf(stdout,"Child process could not be created.\n");
            
            if (pid == 0) {     // Child
            
                char temppath[150];
                getcwd(temppath,sizeof(temppath));
                setenv("parent",temppath,1);
    
                // If a command only has redirection and no pipes
                if (numPipes == 0 && redir > 0) {
                    char arr[]=">";
                    int ind = getIndex (input_2D,arr);
                    char arr2[]="<";
                    int ind2 = getIndex (input_2D,arr2);
                    int smaller = (ind < ind2) ? ind : ind2;
                 
                    char** temp= (char**)malloc(smaller * sizeof(char*));
                    for (int i = 0; i < smaller; i++) {
                        temp[i] = (char*)malloc(10 * sizeof(char));
                        memset(temp[i],'\0',10);
                    }
                    
                    
                    if (redir == 2 && input_2D[ind + 1] != NULL) {
                        for (int i = 0; i < ind; i++) {
                            temp[i] = input_2D[i];
                        }
                        temp[smaller] = NULL;
                        redirHandler(temp, NULL, input_2D[ind + 1], 2);
                        exit(0);
                    }
                    
                    else if (redir == 1 && input_2D[ind2 + 1] != NULL) {
                        for (int i = 0; i < ind2; i++) {
                            temp[i] = input_2D[i];
                        }
                        temp[smaller] = NULL;
                        redirHandler(temp, input_2D[ind2 + 1], NULL, 1);
                        exit(0);
                    }
                    
                    else if (redir == 3 && input_2D[ind + 1] != NULL) {
                        for (int i = 0; i < smaller; i++) {
                            temp[i] = input_2D[i];
                        }
                        temp[smaller] = NULL;
                        redirHandler(temp, input_2D[ind2 + 1], input_2D[ind + 1], 3);
                        exit(0);
                    }
                    else
                        fprintf(stdout,"Something is missing.\n");
                    exit(0);
                }
        
                // If a command has pipes as well as redirection
                if (numPipes > 0) {
                    int tempp = numPipes+1;
                    pipeImplementer(input_2D,&tempp,0);
                    exit(0);
                }
                
                // For the rest of commands
                int check = execvp(input_2D[0],input_2D);
                if (check == -1) {
                    fprintf(stdout,"Unidentified input.\n");
                    
                    // Part 1 task
                    for (int i=0;input_2D[i]!=NULL;i++) {
                        printf("%s\n",input_2D[i]);
                    }
                    exit(0);
                }
            }
            
            //Avoiding zombie processes by using wait system call as the entry of child
            //in process table is deleted when parent reads the status of child
            if (checkBackground(inCheck) == 0)
                wait(NULL);
                
            else
                fprintf(stdout,"\t%ld\n",(long)pid); //For background process
        }
        
    }
    while (1);
	exit(0); // exit normally	
}
void printEnv () {
    extern char **environ;
    char *track = *environ;
    for (int i = 1; track != NULL; i++) {
        fprintf(stdout,"%s\n", track);
        track = *(environ+i);
    }
}

int changeCurrentDirectory (char* dir[]) {
    //code for directory with space in name
    //it works but gives an escape code warning so ill comment this
/*
    if (dir[3] == NULL){
        int len = strlen(dir[1]);
        char *temp = malloc(strlen(dir[1]) + strlen(dir[2]) + 3);
        dir[1][len-1]= '\0';
        strcpy(temp,dir[1]);
        strcat(temp,"\ ");
        strcat(temp,dir[2]);
        if (chdir(temp) < 0) {
            return -1;
        }
        return 0;
    }
    */
    if (dir[1] == NULL) {
        const char* homeDir = getenv("HOME");
        chdir(homeDir);
        return 1;
    }
    else if (chdir(dir[1]) < 0) {
        return -1;
    }
    return 0;
}

int listCurrentDirectory(char* dir)    
{
    struct dirent *temp;
    DIR* currDirectory; 
    currDirectory = opendir(dir);  

    if (currDirectory == NULL) {
        return -1;
    }
    else {   
        while ((temp = readdir(currDirectory)) != NULL) {
            if(temp->d_name[0]!='.')
            {
                fprintf(stdout,"%s\n",temp->d_name);
            }
        }
        
        closedir(currDirectory);
        return 0;
    }
}

char** parseCommand (char input[],int* tokenCount) {

    char sep[] = " ";
    for (int i = 0; input[i] != '\0' ; i++) {
        if (input[i] == '&') {
            input[i]='\0';
        }
    }
	char *tokens = strtok(input, sep);
    char** arr= (char**)malloc(15 * sizeof(char*));
    for (int i=0;i<15;i++) {
        arr[i] = (char*)malloc(40 * sizeof(char));
        memset(arr[i],'\0',40);
    }
    
    while(tokens != NULL)
    {
        arr[(*tokenCount)] = tokens;
        (*tokenCount)++;
        tokens = strtok(NULL, sep);
    }
    arr[(*tokenCount)]=NULL;
    return arr;

}

char*** getCommands(char* arr[],int n) {
    
    char*** temp;   
    temp = (char ***) malloc (n * sizeof(char**));
    for (int h = 0; h < n; h++) {
        temp[h] = (char **) malloc(10 * sizeof(char*));
        for (int r = 0; r < 10; r++) {
            temp[h][r] = (char *) malloc(10 * sizeof(char));
            memset(temp[h][r],'\0',10);
        }
    } 
    
    int sj = 0, x = 0;
    for (int j = 0; j < n; j++) {
        for (int i = 0; arr[i] != NULL; i++) {
            if (strcmp(arr[i],"|") != 0) {
                temp[j][x] = arr[i];
                x++;
            }
            if (strcmp(arr[i],"|") == 0) {
                temp[j][x] = NULL;
                j++;
                x =0;
            }
        }
        sj=j;
    }
    temp[sj][x]=NULL;
    return temp;
}

void childSignalHandler(int signum) 
{
    //Avoiding zombie processes by using wait system call as the entry of child
    //in process table is deleted when parent reads the status of child
    //wait(NULL);
	while (waitpid(-1, NULL, WNOHANG) > 0) {
    }
}

int checkBackground(char* arr) {
    for (int i = 0; arr[i] != '\0' ; i++) {
        if (arr[i] == '&') {
            return 1;
        }
    }
    return 0;
}

int checkPipe(char* arr) {
    int count = 0;
    for (int i = 0; arr[i] != '\0' ; i++) {
        if (arr[i] == '|') {
            count++;
        }
    }
    return count;
}

int checkRedirection(char *array) {
    char *greaterThan = strstr(array, ">");
    char *lessThan = strstr(array, "<");

    if((greaterThan != NULL) && (lessThan != NULL)) {
        return 3;
    }
    else if(greaterThan != NULL) {
        return 2;
    }
    else if(lessThan != NULL) {
        return 1;
    }
    else {
        return -1;
    }
}

int checkPipeRedirection(char **str) {
    int l = 0,r = 0;
    for (int i=0; str[i] != NULL; i++) {
        if (strcmp(str[i],">") == 0)
            l=1;
        else if (strcmp(str[i],"<") == 0)
            r=1;
    }
    if (l==1 && r==1)
        return 3;
    if (l == 1)
        return 2;
    if (r ==1)
        return 1;
    return -1;
        
}

int redirHandler(char* input[], char* inputFile, char* outputFile, int state) {
    int fd,fd2;

    if(state == 1) {

        if((fd = open(inputFile, O_RDONLY, 0644)) < 0){
            fprintf(stdout, "Error opening the input file.\n");
            return -1;
        }

        dup2(fd, 0);
        close(fd);

    }
    else if(state == 2) {

        if((fd = open(outputFile, O_CREAT | O_WRONLY | O_TRUNC, 0644)) < 0){
            fprintf(stdout, "Error opening the output file.\n");
            return -1;
        }

        dup2(fd, 1);
        close(fd);
    }
    else if(state == 3) {

        if((fd = open(inputFile, O_RDONLY, 0644)) < 0){
            fprintf(stdout, "Error opening the input file.\n");
            return -1;
        }

        if((fd2 = open(outputFile, O_CREAT | O_WRONLY | O_TRUNC, 0644)) < 0){
            fprintf(stdout, "Error opening the output file.\n");
            return -1;
        }

        dup2(fd, 0);
        close(fd);

        dup2(fd2, 1);
        close(fd2);
    }

    char temppath[150];
    getcwd(temppath,sizeof(temppath));
    setenv("parent",temppath,1);
    
    int check = execvp(input[0],input);
    if (check == -1) {
        fprintf(stdout,"Unidentified input.\n");
        exit(0);
    }
    
    
    return 1;
}

int getIndex (char* arr[],char* x) {
    for (int i = 0; arr[i] != NULL; i++) {
        if (strcmp(arr[i],x) == 0)
            return i;
    }
    return 99;

}



int pipeImplementer (char **input, int *noPipes, int i){
    if(i == *noPipes - 1){

        int c = *noPipes;
        char ***args = getCommands(input,c);
        int x = 0;

        if((x = checkPipeRedirection(args[i])) < 0){
            execvp(args[i][0], args[i]);
            fprintf(stdout,"Exec error.\n");
            exit(0);
        }
        
        char arr[]=">";
        int ind = getIndex (args[i],arr);
        char arr2[]="<";
        int ind2 = getIndex (args[i],arr2);
        int smaller = (ind < ind2) ? ind : ind2;
     
        char** temp= (char**)malloc(smaller * sizeof(char*));
        for (int i = 0; i < smaller; i++) {
            temp[i] = (char*)malloc(10 * sizeof(char));
            memset(temp[i],'\0',10);
        }
        
        if (x == 2 && args[i][ind + 1] != NULL) {
            for (int j = 0; j < ind; j++) {
                temp[j] = args[i][j];
            }
            temp[smaller] = NULL;
            redirHandler(temp, NULL, args[i][ind + 1], 2);
        }
        
        else if (x == 1 && args[i][ind2 + 1] != NULL) {
            for (int j = 0; j < ind2; j++) {
                temp[j] = args[i][j];
            }
            temp[smaller] = NULL;
            redirHandler(temp, args[i][ind2 + 1], NULL, 1);
        }
        
        else if (x == 3 && args[i][ind + 1] != NULL) {
            for (int j = 0; j < smaller; j++) {
                temp[j] = args[i][j];
            }
            temp[smaller] = NULL;
            redirHandler(temp, args[i][ind2 + 1], args[i][ind + 1], 3);
        }
        else
            fprintf(stdout,"Something is missing.\n");
        return 0;
    }
    
    if(i < *noPipes) {
        
        pid_t pid;
        int fd[2];
        int x;
        if(pipe(fd) < 0){
            fprintf(stdout,"Exec error.\n");
            exit(0);
        }

        if((pid = fork()) < 0){
            fprintf(stdout,"Fork error.\n");
            exit(0);
        }

        if(pid != 0){
            dup2(fd[1], 1);
            close(fd[0]);
            int c = *noPipes;
            char*** args = getCommands(input,c);
            
            if((x = checkPipeRedirection(args[i])) < 0){
                execvp(args[i][0], args[i]);
                fprintf(stdout,"Exec error.\n");
                exit(0);
            }

            char arr[]=">";
            int ind = getIndex (args[i],arr);
            char arr2[]="<";
            int ind2 = getIndex (args[i],arr2);
            int smaller = (ind < ind2) ? ind : ind2;
         
            char** temp= (char**)malloc(smaller * sizeof(char*));
            for (int i = 0; i < smaller; i++) {
                temp[i] = (char*)malloc(10 * sizeof(char));
                memset(temp[i],'\0',10);
            }
            
            
            if (x == 2 && args[i][ind + 1] != NULL) {
                for (int j = 0; j < ind; j++) {
                    temp[j] = args[i][j];
                }
                temp[smaller] = NULL;
                redirHandler(temp, NULL, args[i][ind + 1], 2);
            }
            
            else if (x == 1 && args[i][ind2 + 1] != NULL) {
                for (int j = 0; j < ind2; j++) {
                    temp[j] = args[i][j];
                }
                temp[smaller] = NULL;
                redirHandler(temp, args[i][ind2 + 1], NULL, 1);
            }
            
            else if (x == 3 && args[i][ind + 1] != NULL) {
                for (int j = 0; j < smaller; j++) {
                    temp[j] = args[i][j];
                }
                temp[smaller] = NULL;
                redirHandler(temp, args[i][ind2 + 1], args[i][ind + 1], 3);
            }
            else
                fprintf(stdout,"Something is missing.\n");

            waitpid(-1, NULL, 0);
        }
        
        else {
            if(i != *noPipes-1) {
                dup2(fd[0], 0);
            }
            close(fd[1]);
            i++;
            pipeImplementer(input, noPipes, i);
        }
    }
    return 1;
}

int checkLs(char* arr) {
    for (int i = 0; i < strlen(arr); i++) {
        if (arr[i] == '-')
            return 1;
    }
    return 0;
}
