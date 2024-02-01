#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <libgen.h>
#include <ctype.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>

void user(char *array[], int i, char *envp[], int p, int x);
void batch(char *temp1[], char *temp2, int k, int b);
void execute(char *array[], int ex, int bas, int us, int i, int k, int b);
void cd(char *arguments);
void clr();
void dirr(char *arguments);
void envir(char *envp[]);
void echo(char *array[], int c);
void helpp();
int pause();
void quit();
void pwd();
void redin(char *filename);
void redout(char *text[], char *filename, int k);
void redappend(char *text[], char *filename, int k);

int main(int argc, char* argv[], char *envp[]){
    char *usercmds[9] = {"cd", "clr", "dir", "environ", "echo",
    "help", "pause", "quit", "pwd"};
    char *batcmds[3] = {"<", ">", ">>"};
    char excmds[2] = {'/', '.'};
    char path[200] = "$MYshell:~";
    char bpath[200]; //path of directories
    while(1){
        int us = 0;  //tracks user commands
        int bas = 0;  //tracks batch commands
        int ex = 0;  //tracks exec commands

        char *args = NULL;
        size_t bufsize = 32;
        size_t chars;
        getcwd(bpath, 200);
        //changes the color and font of the command line
        args = (char *)malloc(bufsize * sizeof(char));
        printf("\033[0;32m");
        printf("\e[1m%s\e[m", path);
        printf("\033[0;34m");
        printf("\e[1m%s\e[m", bpath);
        printf("\033[0;37m");
        printf("$ ");
        //gets and puts the command line together
        chars = getline(&args, &bufsize, stdin);
        args[strcspn(args, "\n")] = 0;
        //printf("You typed: '%s' \n",args);

        //variables for argument tokenization
        char *t = strtok(args, " ");
        char *array[50];
        int i = 0; //index of argument array

        //tokenize string of arguments
        while(t != NULL){
            array[i] = t;
            t = strtok(NULL, " ");
            i++;
        }

        int x; //the array index for the usr cmd being called
        int p = 0;

        //checks if the cmd inputed is a usr cmd or not
        for(int j = 0; j<9; j++){
            if((strcmp(array[0], usercmds[j])) == 0){
            //printf("match\n");
            x = j;
            us++;
            p++;
            }
        }  

        int up = 0;
        int down = 0; //index of the 1st bat cmd, so i can start looping from the 1st cmd
        int down2 = 0;
        int b = 0; //array index for 1st bat cmd
        int b2 = 0; //array index for 2nd bat cmd
        int k = 0;

        //checks if the cmd inputed is a batch cmd or not
        for(int j = 0; j<3; j++){
            //printf("array[%d]: %s\n", j, cmds[j]);
            for(k = 0; k<i; k++){
            if((strcmp(array[k], batcmds[j])) == 0){

                    b = j;
                    down = k;
                    up = i - down - 1;
                    bas++;
                    } 
            }
            if(bas > 0){
                for(k = (down +1); k<i; k++){
                    if((strcmp(array[k], batcmds[j])) == 0){
                    b2 = j;
                    down2 = k;
                    bas++;
                    }
                }
            }
        }
        
        for(int e = 0; e<2; e++){
            char ch[20];
            strcpy(ch, array[0]);
            if(ch[0] == excmds[e]){
                ex++;
            }
        }

        char *temp1[down+1];
        char *temp2;
        char *ttemp2 = array[k+1];
        

        for(int t1 = 0; t1<(down); t1++){
            temp1[t1] = array[t1];
        }  
        temp1[down] = NULL;
       
        for(int t2 = 0; t2<up; t2++){
            temp2 = array[i-t2 - 1];
        }

        if(us > 0){
            user(array, i, envp, p, x);
        }  
        if((bas > 0) && (ex < 1)){
            batch(temp1, temp2, k, b);
        }
        if(bas == 2){
            char *argg[2] = {temp2, NULL};
            if(b == 1){
                redout(argg, ttemp2, k);
            }
            if(b == 2){
                redappend(argg, ttemp2, k);
            }
            
        }
        if((ex > 0) || ((bas == 0) && (us == 0))){
            array[i+1] = NULL;
            execute(array, ex, bas, us, i, k, b);
            remove("temp3.txt");
        }
        
    }

    return 0;
}

void execute(char *array[], int ex, int bas, int us, int i, int k, int b){

    if((ex != 0) && (bas ==0)){
        int pid;
            if((pid = fork()) == -1 ){
            perror("can't fork");
            exit(1);
        } else if(pid == 0){

            execv(array[0], array);  
            exit(0);
        }
        else{
            //printf("finish\n");
            //exit(0);
        }
    } else if(bas != 0){
       char *temp = array[i-1];
        int fds[2];
        int pid;
        int stat;
        int file = open("temp3.txt", O_CREAT | O_WRONLY | O_TRUNC,  0777);
        if(pipe(fds) != -1 ){
            pid = fork();

            if(pid ==0){
                close(1); 
                dup2(file, STDOUT_FILENO); 
                close(file);
                //close(fds[0]); 
                for(int j = 1; j<i; j++){
                    array[j] = NULL;
                }
                execv(array[0], array);
                printf("\n");
                //if((stat = execv))
            } else{
                wait(NULL);
                char *tarr[2] = {"temp3.txt", NULL};
                batch(tarr, temp, k, b);
                
            }
        }
    } else{
         int pid;
        if((pid = fork()) == -1 ){
        perror("can't fork");
        exit(1);
        } else if(pid == 0){

        execvp(array[0], array);  
        exit(0);
        } else{
        //printf("finish\n");
        exit(0);
        }
    }

}

void batch(char *temp1[], char *temp2, int k, int b){
    switch(b){
        case 0:
            redin(temp2);
            break;
        case 1:
            redout(temp1, temp2, k);
            break;
        case 2:
            redappend(temp1, temp2, k);
            break;
    }
    
    
}

void user(char *array[], int i, char *envp[], int p, int x){
    if(p == 0){
        printf("invalid command, try again\n");
        return;
    }
    switch(x){
        case 0:
            cd(array[1]);
            break;
        case 1:
            clr();
            break;
        case 2:
            if(array[1] == NULL){
                array[1] = ".";
            }
            dirr(array[1]);
            break;
        case 3:
            envir(envp);
            break;
        case 4:
            echo(array, i);
            break;
        case 5:
            helpp();
            break;
        case 6:
            pause();
            break;
        case 7:
            quit();
            break;
        case 8:
            pwd();
            break;
    }

}

void cd(char *arguments){ //works 
    // printing current working directory    
    chdir(arguments);
}

void clr(){ //works
    printf("\e[1;1H\e[2J");
}

void dirr(char *arguments){ //works
    printf("Files in: %s\n", arguments);
    DIR *d;
    struct dirent *dir;
    
     d = opendir(arguments);
     if (d) {
       while ((dir = readdir(d)) != NULL) {
         printf("%s\n", dir->d_name);
    }
    closedir(d);
  }
}

void envir(char *envp[]){ //works
  int i;
  
    for (i = 0; envp[i] != NULL; i++){
        printf("\n%s", envp[i]);
        }
        printf("\n");  
  
}
    
void echo(char *array[], int c){ // working
    for(int i = 1; i<c; i++){
      printf("%s ", array[i]);
    }
    printf("\n");
   
}

void helpp(){ //working
    printf("cd <directory> - changes the current directory\n");
    printf("clr  - clears the screen\n");
    printf("dir <directory> - lists all the contents of the directory\n");
    printf("environ - lists environment variables\n");
    printf("echo <directory> - changes the current directory\n");
    printf("help <text> - reads back the text you input\n");
    printf("pause - pauses the shell until user hits ENTER\n");
    printf("quit - ends the shell program\n");
    printf("pwd - outputs the fullpath name of the current directory\n");
}

int pause(){ //works
    printf("shell paused, hit 'ENTER' to continue\n");
    getchar();
    return 0;
}

void quit(){ // working
    exit(0);
}

void pwd(){
    char bpath[200];
    getcwd(bpath, 200);
    printf("%s\n", bpath);
}

void redin(char *filename){ 
    // '< filename'  opens and reads file
    char tok;
    printf("reading the file %s\n", filename);
    FILE *file = fopen(filename, "r");

    if(file == NULL){
        printf("Cant open mem file\n\n");
    }

    tok = fgetc(file);
    putchar(tok);

    while(tok != EOF){
        tok = fgetc(file);
        putchar(tok);
    }

    printf("\n\n");

    fclose(file);
}

void redout(char *text[], char *filename, int k){ 
    // '"text" > filename' overwrites to a file 
    struct stat st;
    int s = stat (text[0], &st);
    //copy file to file
    if (S_ISREG (st.st_mode)){
        FILE *sourFile = fopen(text[0], "r");;
        FILE *destFile = fopen(filename, "w");;
        char copy = fgetc(sourFile);
        
        //copy to file
        while(copy != EOF){
            fputc(copy, destFile);
            copy = fgetc(sourFile);
        }
        fclose(sourFile);
        fclose(destFile);
    } else{
        //copy text to file
        FILE *file = fopen(filename, "w");

        int i = 0;
        for(;;){
            fprintf(file, "%s ", text[i]);
            i++;
            if(i == (k-2))
            break;
        }
        fclose(file);
    }

    
}


void redappend(char *text[], char *filename, int k){ 
    // '"text" >> filename' adds to file
     struct stat st;
    int s = stat (text[0], &st);
    printf("redout arr is %s\n", text[0]);
    if (S_ISREG (st.st_mode)){
        FILE *sourFile = fopen(text[0], "r");;
        FILE *destFile = fopen(filename, "a");;
        char copy = fgetc(sourFile);

        while(copy != EOF){
            fputc(copy, destFile);
            copy = fgetc(sourFile);
        }
        fclose(sourFile);
        fclose(destFile);
    } else{
        FILE *file = fopen(filename, "a");

        int i = 0;
        for(;;){
            fprintf(file, "%s ", text[i]);
            i++;
            if(i == (k-2))
            break;
        }
        fclose(file);
    }

}